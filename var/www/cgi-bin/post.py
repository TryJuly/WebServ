##!/usr/bin/env python3
#import os
#import sys
#
#method = os.environ.get("REQUEST_METHOD", "")
#content_length = int(os.environ.get("CONTENT_LENGTH", 0))
#
#body = ""
#if method == "POST" and content_length > 0:
#    body = sys.stdin.read(content_length)
#
#print("Content-Type: text/html\r")
#print("\r")
#print("<html><body>")
#print("<h1>CGI POST Test</h1>")
#print("<p>Method: " + method + "</p>")
#print("<p>Body received: " + body + "</p>")
#print("</body></html>")

#!/usr/bin/env python3
import time

print("Content-Type: text/html\r")
print("\r")
print("<html><body>CGI loop</body></html>\r")

while True:
    time.sleep(1)