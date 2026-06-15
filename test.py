import socket
import time

def test_cgi_timeout(host, port, cgi_path):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))
    
    request = f"GET {cgi_path} HTTP/1.1\r\nHost: {host}\r\nConnection: close\r\n\r\n"
    s.send(request.encode())
    
    start = time.time()
    s.settimeout(20)  # on attend max 20s
    
    try:
        response = s.recv(4096).decode()
        elapsed = time.time() - start
        print(f"Réponse reçue en {elapsed:.2f}s")
        print(response[:200])
        if "504" in response:
            print("✅ Timeout CGI OK")
        else:
            print("❌ Pas de 504")
    except socket.timeout:
        print("❌ Pas de réponse après 20s — serveur bloqué")
    finally:
        s.close()

test_cgi_timeout("127.0.0.1", 8080, "/cgi-bin/post.py")