import time
import json
import commands
import threading
import BaseHTTPServer

HOST_NAME = ''
PORT_NUMBER = 8080

def play(msg, volume, locale):
    commands.getoutput("pico2wave -l \"" + locale + "\" -w pico2wave.wav \"" + msg + "\"")
    commands.getoutput("omxplayer --vol -750 chime.wav")
    commands.getoutput("omxplayer --vol " + str(volume) + " pico2wave.wav")


class TtsHandler(BaseHTTPServer.BaseHTTPRequestHandler):

    def do_POST(s):

        body = json.loads(s.rfile.read(int(s.headers.getheader("content-length", 0))))
        msg = body.get("msg")
        locale = body.get("locale", "en-US")
        volume = body.get("volume", -500)

        threading.Thread(target = play, args = (msg, volume, locale)).start()

        s.send_response(200)


if __name__ == '__main__':
    server_class = BaseHTTPServer.HTTPServer
    httpd = server_class((HOST_NAME, PORT_NUMBER), TtsHandler)
    print time.asctime(), "Server Starts - %s:%s" % (HOST_NAME, PORT_NUMBER)
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        pass
    httpd.server_close()
    print time.asctime(), "Server Stops - %s:%s" % (HOST_NAME, PORT_NUMBER)


