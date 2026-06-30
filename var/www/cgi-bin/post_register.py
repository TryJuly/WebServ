#!/usr/bin/env python3
import os
import sys
from urllib.parse import parse_qs

method = os.environ.get("REQUEST_METHOD", "")
cl     = int(os.environ.get("CONTENT_LENGTH", 0) or 0)

body = ""
if method == "POST" and cl > 0:
    try:
        body = sys.stdin.buffer.read(cl).decode("utf-8", errors="replace")
    except Exception:
        body = ""

fields   = parse_qs(body)
username = fields.get("username", [""])[0].strip()
password = fields.get("password", [""])[0]
email    = fields.get("email",    [""])[0].strip()

errors = []
if not username:
    errors.append("Le champ 'username' est vide.")
elif len(username) < 3:
    errors.append("Le username doit faire au moins 3 caracteres.")
if not password:
    errors.append("Le champ 'password' est vide.")
elif len(password) < 8:
    errors.append("Le mot de passe doit faire au moins 8 caracteres.")
if not email or "@" not in email:
    errors.append("L'adresse email est invalide.")

print("Content-Type: text/html\r")
print("\r")
print("<html><head><title>Inscription POST</title></head><body>")
print("<h1>POST - Inscription utilisateur</h1>")
if not body:
    print("<p>Aucun corps recu. Envoie username, password et email en POST.</p>")
elif errors:
    print("<h2 style='color:red'>Erreurs de validation :</h2><ul>")
    for e in errors:
        print("<li>" + e + "</li>")
    print("</ul>")
else:
    print("<p style='color:green'><strong>Inscription reussie !</strong></p>")
    print("<table border='1' cellpadding='6'>")
    print("<tr><td>Username</td><td>" + username + "</td></tr>")
    print("<tr><td>Email</td><td>" + email + "</td></tr>")
    print("<tr><td>Password</td><td>****" + password[-2:] + "</td></tr>")
    print("</table>")
print("<h2>Corps brut :</h2><pre>" + body + "</pre>")
print("</body></html>")
