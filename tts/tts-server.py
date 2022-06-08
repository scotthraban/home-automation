import os
import time
import json
import subprocess
import threading
import http.server

HOST_NAME = ''
PORT_NUMBER = 8080

def play(msg, volume, locale):
    subprocess.run(["pico2wave", "-l", locale, "-w", "pico2wave.wav", msg])
    subprocess.run(["paplay", "--volume=32768", "chime.wav"])
    subprocess.run(["paplay", "--volume=" + str(volume), "pico2wave.wav"])


class TtsHandler(http.server.BaseHTTPRequestHandler):

    def do_POST(s):

        body = json.loads(s.rfile.read(int(s.headers.get("content-length", 0))))
        msg = body.get("msg")
        locale = body.get("locale", "en-US")
        volume = body.get("volume", 50000)

        threading.Thread(target = play, args = (msg, volume, locale)).start()

        s.send_response(200)


if __name__ == '__main__':
    server_class = http.server.HTTPServer
    httpd = server_class((HOST_NAME, PORT_NUMBER), TtsHandler)
    print(time.asctime(), "Server Starts in %s - %s:%s" % (os.getcwd(), HOST_NAME, PORT_NUMBER))
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        pass
    httpd.server_close()
    print(time.asctime(), "Server Stops - %s:%s" % (HOST_NAME, PORT_NUMBER))

