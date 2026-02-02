#!/usr/bin/env python3
import os
import sys
print(" > hello from Child", file=sys.stderr)

print("Content-Type: text/plain\r")
print("Status: 200 OK\r")
print("\r")  # end of headers
sys.stdout.flush()  # <--- flush headers
print(" > Child did something", file=sys.stderr)

print("Hello from CGI 👋")
print()
sys.stdout.flush()  # <--- flush headers

print("Method:", os.environ.get("REQUEST_METHOD"))
print("Query:", os.environ.get("QUERY_STRING"))
print("Content-Length:", os.environ.get("CONTENT_LENGTH"))

# echo POST body if any
body = sys.stdin.read()
if body:
    print("\n--- BODY ---")
    print(body)
sys.stdout.flush()  # <--- flush headers
print(" > Child is done", file=sys.stderr)
sys.stdout.flush()  # <--- flush headers
sys.exit(0)