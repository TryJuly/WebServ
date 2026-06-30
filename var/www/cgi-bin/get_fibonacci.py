#!/usr/bin/env python3
import os

qs = os.environ.get("QUERY_STRING", "")
n  = 10
for pair in qs.split("&"):
    if pair.startswith("n="):
        try:
            n = int(pair[2:])
            if n < 1:
                n = 1
            if n > 50:
                n = 50
        except ValueError:
            n = 10

def fibonacci(count):
    seq = []
    a, b = 0, 1
    for _ in range(count):
        seq.append(a)
        a, b = b, a + b
    return seq

seq = fibonacci(n)

print("Content-Type: text/html\r")
print("\r")
print("<html><head><title>Fibonacci GET</title></head><body>")
print("<h1>GET - Suite de Fibonacci</h1>")
print("<p>Parametre : <code>n=" + str(n) + "</code> (max 50). Essaie <code>?n=20</code></p>")
print("<p>Resultat (" + str(n) + " termes) :</p>")
print("<p><code>" + ", ".join(str(x) for x in seq) + "</code></p>")
print("</body></html>")
