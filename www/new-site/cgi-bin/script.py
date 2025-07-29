##!/usr/bin/env python3
from os import getenv

print("HTTP/1.1 200 OK\r\n")
print("Content-Length: 15")
print("Host: " + getenv("HTTP_HOST"))
print("\r\n")
print("<h1>POSTED</h1>")