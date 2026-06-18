#!/usr/bin/env python3
import os
import sys

method = os.environ.get("REQUEST_METHOD", "")
cl     = int(os.environ.get("CONTENT_LENGTH", 0) or 0)
qs     = os.environ.get("QUERY_STRING", "")
ctype  = os.environ.get("CONTENT_TYPE", "")

body = sys.stdin.read(cl) if cl > 0 else ""

print("Content-Type: text/html\r")
print("\r")
print("<html><body>")
print(f"<p>method={method}</p>")
print(f"<p>query_string={qs}</p>")
print(f"<p>content_type={ctype}</p>")
print(f"<p>content_length={cl}</p>")
print(f"<p>body={body}</p>")
print("</body></html>")