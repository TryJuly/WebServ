#!/usr/bin/env python3
import os

qs   = os.environ.get("QUERY_STRING", "")
name = ""
for pair in qs.split("&"):
    if pair.startswith("name="):
        name = pair[5:].replace("+", " ")
        break

print("Content-Type: text/html\r")
print("\r")
print("<html><head><title>Hello GET</title></head><body>")
print("<h1>GET - Hello World</h1>")
if name:
    print("<p>Bonjour, <strong>" + name + "</strong> !</p>")
else:
    print("<p>Bonjour, visiteur anonyme !</p>")
    print("<p>Essaie : <code>?name=Alice</code></p>")
print("<p>QUERY_STRING : <code>" + qs + "</code></p>")
print("</body></html>")
