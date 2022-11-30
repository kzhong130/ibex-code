from http.server import BaseHTTPRequestHandler, HTTPServer
import logging

class handler(BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.send_header('Content-type','text/html')
        self.end_headers()

        print("Hello, World! Here is a GET response")
        f = open("static/index.html", 'rb')
        html = f.read()
        f.close()
        self.wfile.write(html)

    def do_POST(self):
        content_length = int(self.headers['Content-Length'])
        self.send_response(200)
        self.send_header('Content-type','text/html')
        self.end_headers()
        body = self.rfile.read(content_length)
        print("POST body size:", content_length/1024.0**2, " MB")

        f = open("static/index.html", 'rb')
        html = f.read()
        self.wfile.write(html)
        f.close()

with HTTPServer(('', 8000), handler) as server:
    server.serve_forever()