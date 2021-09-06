import os
import sys
import time
import json
import requests
from http.server import HTTPServer, SimpleHTTPRequestHandler, BaseHTTPRequestHandler

HOST_NAME = ''
PORT_NUMBER = 8080


class SendTelegramHandler(BaseHTTPRequestHandler):

    _token = None
    _channels = None

    def do_POST(self):
        ctx = os.getenv("CONTEXT_PATH")
        {
            ctx + "/message": self.handle_post_message
        }.get(self.path, self.not_found)()

    def not_found(self):
        self.send_response(404, "Not Found")
        self.send_header("Content-Length", 0)
        self.end_headers()

    def handle_post_message(self):

        body = json.loads(self.rfile.read(int(self.headers.get("content-length", 0))).decode())

        response_code, response_msg = self.send_message(body)
        if response_code != 200:
            print("Returning error response {} / {}".format(response_code, response_msg))

        self.send_response(response_code, response_msg)
        self.send_header("Content-Length", 0)
        self.end_headers()

    def get_channels(self):
        if not self._channels:
            f = open("/telegram-bot/etc/channels.json", "r")
            self._channels = json.loads(f.read())
            f.close()
        return self._channels

    def get_token(self):
        if not self._token:
            self._token = os.getenv("BOT_TOKEN")
        return self._token

    def send_message(self, body):

        if not body.get("channel"):
            return 400, "missing channel"

        channel = self.get_channels().get(body.get("channel"))
        if not channel:
            return 400, "invalid channel: {}, {}".format(body.get("channel"), self.get_channels().keys())

        if not body.get("msg"):
            return 400, "missing msg"

        data = {
            "chat_id": channel,
            "text": body.get("msg")
        }

        if bool(body.get("silent", False)):
            data['disable_notification'] = True

        response = requests.post("https://api.telegram.org/bot" + self.get_token() + "/sendMessage", data)
        if response.status_code != 200:
            return 502, "Failed to send message to telegram channel {} '{}'"\
                .format(response.status_code, response.text)

        return 200, "OK"


def run(server_class=HTTPServer, handler_class=BaseHTTPRequestHandler):
    httpd = server_class((HOST_NAME, PORT_NUMBER), handler_class)
    print(time.asctime(), "Server Starts - %s:%s" % (HOST_NAME, PORT_NUMBER))
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        pass
    httpd.server_close()
    print(time.asctime(), "Server Stops - %s:%s" % (HOST_NAME, PORT_NUMBER))


if __name__ == '__main__':
    run(HTTPServer, SendTelegramHandler)
