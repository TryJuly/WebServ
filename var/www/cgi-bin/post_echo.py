#!/usr/bin/env python3
import os
import sys

method = os.environ.get("REQUEST_METHOD", "")
cl     = int(os.environ.get("CONTENT_LENGTH", 0) or 0)
ctype  = os.environ.get("CONTENT_TYPE", "")

body = ""
if method == "POST" and cl > 0:
    try:
        body = sys.stdin.buffer.read(cl).decode("utf-8", errors="replace")
    except Exception:
        body = ""

print("Content-Type: text/html\r")
print("\r")
print("<html><head><title>Echo POST</title></head><body>")
print("<h1>POST - Echo du corps</h1>")
print("<table border='1' cellpadding='6'>")
print("<tr><td>REQUEST_METHOD</td><td><code>" + method + "</code></td></tr>")
print("<tr><td>CONTENT_TYPE</td><td><code>" + ctype + "</code></td></tr>")
print("<tr><td>CONTENT_LENGTH</td><td><code>" + str(cl) + "</code></td></tr>")
print("</table>")
print("<h2>Corps recu :</h2>")
print("<pre>" + body + "</pre>")
print("</body></html>")
