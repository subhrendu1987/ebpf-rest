from http.server import BaseHTTPRequestHandler, HTTPServer
import json

class SimpleHTTPRequestHandler(BaseHTTPRequestHandler):
    def do_POST(self):
        # Default response for unmatched URLs
        response = {}

        # Check the requested URL path
        if self.path == '/v1/data/ebpf/allow':
            # Read the length of the content
            content_length = int(self.headers['Content-Length'])

            # Read the POST data
            post_data = self.rfile.read(content_length)

            # Parse the JSON data
            try:
                data = json.loads(post_data)
                # Check if the POST data matches the expected structure and value
                if data.get("input", {}).get("funcName") == "mptm_decap":
                    response = {"result": False}
                else:
                    response = {"result": True}
            except json.JSONDecodeError:
                response = {"error": "Invalid JSON"}

        # Set the response code to 200 (OK)
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.end_headers()

        # Send the response as JSON
        self.wfile.write(json.dumps(response).encode('utf-8'))

    def do_GET(self):
        # Respond with {} for all GET requests
        response = {}
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.end_headers()
        self.wfile.write(json.dumps(response).encode('utf-8'))

def run(server_class=HTTPServer, handler_class=SimpleHTTPRequestHandler, port=8181):
    server_address = ('', port)
    httpd = server_class(server_address, handler_class)
    print(f'Starting httpd on port {port}...')
    httpd.serve_forever()

if __name__ == "__main__":
    run()
