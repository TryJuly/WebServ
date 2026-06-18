#!/usr/bin/env python3
import os
import sys

method         = os.environ.get("REQUEST_METHOD", "UNKNOWN")
query_string   = os.environ.get("QUERY_STRING", "")
content_length = int(os.environ.get("CONTENT_LENGTH", 0) or 0)
content_type   = os.environ.get("CONTENT_TYPE", "")

body = ""
if method == "POST" and content_length > 0:
    try:
        body = sys.stdin.buffer.read(content_length).decode("utf-8", errors="replace")
    except Exception:
        body = ""

print("Content-Type: text/html\r")
print("\r")
print("<html><body>")
print("<p>METHOD: " + method + "</p>")
print("<p>QUERY_STRING: " + query_string + "</p>")
print("<p>CONTENT_TYPE: " + content_type + "</p>")
print("<p>CONTENT_LENGTH: " + str(content_length) + "</p>")
print("<p>BODY: " + body + "</p>")
print("</body></html>")
