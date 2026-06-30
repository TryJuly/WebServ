#!/usr/bin/env python3
import os
import sys
import json

method = os.environ.get("REQUEST_METHOD", "")
cl     = int(os.environ.get("CONTENT_LENGTH", 0) or 0)

body = ""
if method == "POST" and cl > 0:
    try:
        body = sys.stdin.buffer.read(cl).decode("utf-8", errors="replace")
    except Exception:
        body = ""

data  = None
error = ""
try:
    if body:
        data = json.loads(body)
except json.JSONDecodeError as e:
    error = str(e)

def render(obj, depth=0):
    indent = "&nbsp;" * (depth * 4)
    lines  = []
    if isinstance(obj, dict):
        for k, v in obj.items():
            lines.append(indent + "<strong>" + str(k) + "</strong> : " + render(v, depth + 1))
    elif isinstance(obj, list):
        for i, v in enumerate(obj):
            lines.append(indent + "[" + str(i) + "] " + render(v, depth + 1))
    else:
        return "<em>" + str(obj) + "</em>"
    return "<br>".join(lines)

print("Content-Type: text/html\r")
print("\r")
print("<html><head><title>JSON POST</title></head><body>")
print("<h1>POST - Parseur JSON</h1>")
if error:
    print("<p style='color:red'>Erreur JSON : " + error + "</p>")
    print("<h2>Corps brut :</h2><pre>" + body + "</pre>")
elif data is not None:
    print("<p>JSON valide. Contenu :</p>")
    print("<p>" + render(data) + "</p>")
else:
    print("<p>Aucun corps recu. Envoie un JSON en POST (Content-Type: application/json).</p>")
print("</body></html>")
