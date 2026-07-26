#!/usr/bin/env python3
"""
Local development server for Vircon32 Web Emulator.
Serves files with correct MIME types, especially application/wasm,
which is required for WebAssembly streaming compilation.

Usage:
    python serve.py [port]          # serve output/ directory (default port 8000)
    python serve.py build 8001      # serve build/ directory on port 8001
"""

import sys
import os
import http.server
import mimetypes

# Ensure .wasm gets the correct MIME type for streaming compile
mimetypes.add_type('application/wasm', '.wasm')
mimetypes.add_type('application/octet-stream', '.v32')
mimetypes.add_type('application/octet-stream', '.data')

class CORPHandler(http.server.SimpleHTTPRequestHandler):
    """HTTP handler that adds the headers required for SharedArrayBuffer
    (COOP + COEP) in case they are ever needed, and suppresses the
    noisy favicon 404 log line."""

    def end_headers(self):
        # These headers are needed if SharedArrayBuffer / pthreads are used.
        # They are harmless for the current single-threaded build.
        self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
        self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
        super().end_headers()

    def log_message(self, fmt, *args):
        # Suppress the favicon 404 noise
        if len(args) >= 2 and 'favicon' in str(args[0]):
            return
        super().log_message(fmt, *args)


def main():
    args = sys.argv[1:]
    port = 8000
    directory = None

    # Parse simple positional args: [directory] [port]
    for arg in args:
        if arg.isdigit():
            port = int(arg)
        else:
            directory = arg

    # Default: serve the output directory (next to this script)
    if directory is None:
        script_dir = os.path.dirname(os.path.abspath(__file__))
        directory = os.path.join(script_dir, 'output')

    if not os.path.isdir(directory):
        print(f'ERROR: directory not found: {directory}')
        print('Run build_windows.ps1 first to generate the output/ directory.')
        sys.exit(1)

    os.chdir(directory)
    handler = CORPHandler
    with http.server.HTTPServer(('', port), handler) as httpd:
        print(f'Vircon32 Web Emulator server')
        print(f'Serving: {directory}')
        print(f'URL:     http://localhost:{port}')
        print(f'Press Ctrl+C to stop.')
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print('\nServer stopped.')


if __name__ == '__main__':
    main()
