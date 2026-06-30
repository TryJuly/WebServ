#!/usr/bin/env python3
import os

cgi_vars = [
    "REQUEST_METHOD", "QUERY_STRING", "CONTENT_TYPE", "CONTENT_LENGTH",
    "SERVER_NAME", "SERVER_PORT", "SERVER_PROTOCOL", "SCRIPT_NAME",
    "PATH_INFO", "REMOTE_ADDR", "HTTP_HOST", "HTTP_USER_AGENT",
]

print("Content-Type: text/html\r")
print("\r")
print("<html><head><title>CGI Env</title></head><body>")
print("<h1>GET - Variables d'environnement CGI</h1>")
print("<table border='1' cellpadding='6'>")
print("<tr><th>Variable</th><th>Valeur</th></tr>")
for var in cgi_vars:
    val = os.environ.get(var, "<non defini>")
    print("<tr><td><code>" + var + "</code></td><td>" + val + "</td></tr>")
print("</table>")
print("</body></html>")
