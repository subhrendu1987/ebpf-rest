from http.server import BaseHTTPRequestHandler, HTTPServer
import json

class RequestHandler(BaseHTTPRequestHandler):
    def do_POST(self):
        content_length = int(self.headers['Content-Length'])
        print("content length:",content_length)
        post_data = self.rfile.read(content_length)
        print("POST MSG:",post_data)
        data = json.loads(post_data)
        print("Received POST request")
        print("Headers:", self.headers)
        print("Data:", data)

        # Respond with a JSON message
        response = {'status': 'success', 'message': 'Data received'}
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.end_headers()
        self.wfile.write(json.dumps(response).encode('utf-8'))

def run(server_class=HTTPServer, handler_class=RequestHandler, port=9900):
    server_address = ('', port)
    httpd = server_class(server_address, handler_class)
    print(f'Starting httpd server on port {port}')
    httpd.serve_forever()

if __name__ == '__main__':
    run()
