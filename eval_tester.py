#!/usr/bin/env python3
"""
WebServ Eval Tester — École 42
Couvre les critères d'évaluation standards du projet webserv.

Tests couverts :
  ①  Protocole HTTP/1.1        (raw sockets — status line, CRLF, headers)
  ②  Fichiers statiques         (MIME types, Content-Length, 404)
  ③  Autoindex                  (directory listing ON / OFF)
  ④  Méthodes HTTP              (GET / POST / DELETE + 405)
  ⑤  Pages d'erreur             (400 403 404 405 413 — HTML custom)
  ⑥  Body size limit            (client_max_body_size → 413)
  ⑦  Redirections               (301 / 302 + Location header)
  ⑧  CGI                        (GET query string + POST stdin + timeout 504)
  ⑨  Cycle upload               (POST → fichier sur disque)
  ⑩  Virtual hosting            (Host header + multi-serveur)
  ⑪  Keep-Alive                 (deux requêtes sur une connexion TCP)
  ⑫  Stress test (siege-style)  (concurrence + disponibilité ≥ 99 %)

Usage :
    ./webserv full.cnf &
    python3 eval_tester.py
"""

import os
import sys
import time
import socket
import threading
from datetime import datetime

try:
    import requests
except ImportError:
    print("Module 'requests' manquant.  Installe-le avec :  pip3 install requests")
    sys.exit(1)

# ─── Couleurs ANSI ────────────────────────────────────────────────────────────
RED    = "\033[91m"
GREEN  = "\033[92m"
YELLOW = "\033[93m"
BLUE   = "\033[94m"
CYAN   = "\033[96m"
BOLD   = "\033[1m"
DIM    = "\033[2m"
RESET  = "\033[0m"

# ─── Configuration ────────────────────────────────────────────────────────────
HOST        = "127.0.0.1"
PORT_8080   = 8080
PORT_8081   = 8081
BASE_8080   = f"http://{HOST}:{PORT_8080}"
BASE_8081   = f"http://{HOST}:{PORT_8081}"
TIMEOUT     = 6
WEBSERV_DIR = os.path.dirname(os.path.abspath(__file__))
FILES_DIR   = os.path.join(WEBSERV_DIR, "var", "www", "html", "files")

# ─── Compteurs thread-safe ────────────────────────────────────────────────────
_lock   = threading.Lock()
_passed = 0
_failed = 0
_errors = 0

def _incr(key):
    global _passed, _failed, _errors
    with _lock:
        if   key == "p": _passed += 1
        elif key == "f": _failed += 1
        elif key == "e": _errors += 1

# ─── Affichage de base ────────────────────────────────────────────────────────
def ok(msg):
    _incr("p")
    print(f"  {GREEN}[PASS]{RESET} {msg}")

def fail(msg):
    _incr("f")
    print(f"  {RED}[FAIL]{RESET} {msg}")

def skip(msg):
    print(f"  {YELLOW}[SKIP]{RESET} {msg}")

def err(msg, exc=""):
    _incr("e")
    exc_str = str(exc)
    # Résume les erreurs "Connection refused" pour ne pas spammer
    if "Connection refused" in exc_str or "Max retries" in exc_str:
        exc_str = "Connection refused — serveur inaccessible"
    print(f"  {YELLOW}[ERR ]{RESET} {msg}")
    print(f"    {DIM}↳ {exc_str[:120]}{RESET}")

def info(msg):
    print(f"    {DIM}{msg}{RESET}")

def section(title):
    bar = "═" * 58
    print(f"\n{BOLD}{CYAN}╔{bar}╗{RESET}")
    print(f"{BOLD}{CYAN}║  {title:<56}║{RESET}")
    print(f"{BOLD}{CYAN}╚{bar}╝{RESET}")

def sub(title):
    print(f"\n  {BOLD}{BLUE}▸ {title}{RESET}")

# ─── Helpers "attendu vs reçu" ────────────────────────────────────────────────
def _show_diff_status(expected, got):
    """Affiche une ligne attendu/reçu pour les status codes."""
    print(f"    {DIM}↳ Attendu : {expected}{RESET}")
    print(f"    {RED}↳ Reçu    : {got}{RESET}")

def _show_body(body_str, label="Body"):
    """Affiche un extrait du corps de réponse."""
    snippet = body_str[:200].replace('\n', '↵').replace('\r', '')
    print(f"    {DIM}↳ {label}  : {snippet!r}{RESET}")

def check_r(name, r, expected_status, *, contains=None, not_contains=None):
    """
    Vérifie la réponse HTTP (requests.Response).
    Affiche toujours attendu/reçu + extrait body en cas d'échec.
    """
    ok_status = (r.status_code == expected_status)
    ok_body   = True
    body_fail_msg = ""

    if ok_status and contains is not None:
        ok_body = contains in r.text
        body_fail_msg = f"'{contains}' absent du body"
    if ok_status and not_contains is not None and not_contains in r.text:
        ok_body = False
        body_fail_msg = f"'{not_contains}' présent alors qu'interdit"

    if ok_status and ok_body:
        ok(name)
        return True

    fail(name)
    if not ok_status:
        _show_diff_status(f"{expected_status}", f"{r.status_code}")
    if not ok_body:
        print(f"    {RED}↳ Contenu : {body_fail_msg}{RESET}")
    _show_body(r.text)
    return False

def check_raw_status(name, raw, expected_status):
    """
    Vérifie le status d'une réponse brute (str).
    Affiche attendu/reçu + premières lignes en cas d'échec.
    """
    code = _status_of(raw)
    if code == expected_status:
        ok(name)
        return True
    fail(name)
    _show_diff_status(f"{expected_status}", f"{code}")
    first_line = raw.split("\r\n")[0] if raw else "(vide)"
    print(f"    {DIM}↳ Status line : {first_line!r}{RESET}")
    body_start = raw.split("\r\n\r\n", 1)
    if len(body_start) > 1:
        _show_body(body_start[1], "Body")
    return False

def check(name, cond, *, detail=""):
    """Vérification booléenne simple (pas de réponse HTTP disponible)."""
    if cond:
        ok(name)
        return True
    fail(name)
    if detail:
        print(f"    {DIM}↳ {detail}{RESET}")
    return False

def warn_stale(raw):
    """Détecte et signale si la réponse contient plusieurs HTTP/1.1 (réponse parasitée)."""
    count = raw.count("HTTP/1.1")
    if count > 1:
        print(f"    {YELLOW}⚠  RÉPONSE PARASITÉE : {count} blocs HTTP/1.1 dans la même réponse{RESET}")
        print(f"    {DIM}   (le serveur colle une réponse d'un CGI/client précédent){RESET}")
        return True
    return False

# ─── Helpers HTTP (requests) ──────────────────────────────────────────────────
def GET(path, base=BASE_8080, **kw):
    return requests.get(base + path, timeout=TIMEOUT, allow_redirects=False, **kw)

def POST(path, base=BASE_8080, **kw):
    return requests.post(base + path, timeout=TIMEOUT, allow_redirects=False, **kw)

def DELETE(path, base=BASE_8080, **kw):
    return requests.delete(base + path, timeout=TIMEOUT, allow_redirects=False, **kw)

# ─── Raw socket helper ────────────────────────────────────────────────────────
def raw_request(req_bytes, host=HOST, port=PORT_8080, timeout=TIMEOUT):
    """Envoie des octets bruts et retourne la réponse complète (str)."""
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(timeout)
    try:
        s.connect((host, port))
        s.sendall(req_bytes)
        data = b""
        try:
            while True:
                chunk = s.recv(4096)
                if not chunk:
                    break
                data += chunk
        except socket.timeout:
            pass
        return data.decode("utf-8", errors="replace")
    finally:
        s.close()

def _status_of(raw):
    """Extrait le code HTTP du PREMIER bloc dans une réponse brute."""
    line = raw.split("\r\n")[0] if raw else ""
    parts = line.split(" ", 2)
    try:
        return int(parts[1]) if len(parts) >= 2 else 0
    except ValueError:
        return 0

def _headers_of(raw):
    """Extrait les headers (section avant le double CRLF) en minuscules."""
    return raw.split("\r\n\r\n")[0].lower() if "\r\n\r\n" in raw else ""

# ─── Setup / Teardown ─────────────────────────────────────────────────────────
_SETUP_FILES = []

def setup():
    os.makedirs(FILES_DIR, exist_ok=True)
    for name in ("eval_del1.txt", "eval_del2.txt", "eval_keep.txt"):
        p = os.path.join(FILES_DIR, name)
        with open(p, "w") as f:
            f.write(f"eval test — {name}\n")
        _SETUP_FILES.append(p)
    info(f"Fichiers de test créés dans {FILES_DIR}")

def cleanup():
    for p in _SETUP_FILES:
        if os.path.exists(p):
            os.remove(p)

def server_up(port=PORT_8080):
    try:
        s = socket.create_connection((HOST, port), timeout=2)
        s.close()
        return True
    except Exception:
        return False

def check_server_still_up(port=PORT_8080):
    """Vérifie que le serveur est toujours debout entre deux sections."""
    if not server_up(port):
        print(f"\n  {RED}{BOLD}⚠  SERVEUR MORT (port {port}) — il a crashé pendant les tests précédents{RESET}")
        return False
    return True

# ═════════════════════════════════════════════════════════════════════════════
# ①  PROTOCOLE HTTP/1.1  (raw sockets)
# ═════════════════════════════════════════════════════════════════════════════
def test_http_protocol():
    section("①  PROTOCOLE HTTP/1.1  (raw sockets)")

    sub("Format de la ligne de statut")
    try:
        raw = raw_request(
            b"GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n"
        )
        warn_stale(raw)
        first_line = raw.split("\r\n")[0]
        check("Réponse commence par 'HTTP/1.1'",
              first_line.startswith("HTTP/1.1"),
              detail=f"Status line reçue : {first_line!r}")
        parts = first_line.split()
        check("Ligne de statut = 'HTTP/1.1 <3 chiffres> <raison>'",
              len(parts) >= 3 and len(parts[1]) == 3 and parts[1].isdigit(),
              detail=f"Status line reçue : {first_line!r}")
        check_raw_status("GET /  → 200 OK", raw, 200)
    except Exception as e:
        err("Ligne de statut (raw)", e)

    sub("Séparateurs CRLF obligatoires")
    try:
        raw = raw_request(
            b"GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n"
        )
        check("Réponse utilise \\r\\n comme séparateur de ligne",
              "\r\n" in raw,
              detail="aucun CRLF trouvé dans la réponse")
        check("En-têtes et corps séparés par \\r\\n\\r\\n",
              "\r\n\r\n" in raw,
              detail="pas de double CRLF")
    except Exception as e:
        err("CRLF", e)

    sub("Host header obligatoire en HTTP/1.1")
    try:
        raw = raw_request(b"GET / HTTP/1.1\r\nConnection: close\r\n\r\n")
        warn_stale(raw)
        check_raw_status("HTTP/1.1 sans Host  → 400 Bad Request", raw, 400)
        if _status_of(raw) == 400:
            info("Le RFC 7230 §5.4 impose Host en HTTP/1.1 → correct")
        else:
            info(f"Note RFC : sans Host en HTTP/1.1, le serveur DOIT répondre 400")
    except Exception as e:
        err("HTTP/1.1 sans Host", e)

    sub("HTTP/1.0 accepté (pas de Host requis)")
    try:
        raw = raw_request(b"GET / HTTP/1.0\r\n\r\n")
        code = _status_of(raw)
        check("HTTP/1.0  → 200 ou 400 (pas de crash / réponse vide)",
              code in (200, 400),
              detail=f"Reçu : {code}  (attendu 200 ou 400)")
    except Exception as e:
        err("HTTP/1.0", e)

    sub("Requête malformée → 400 Bad Request")
    cases = [
        (b"INVALIDE\r\n\r\n",    "méthode inconnue"),
        (b"GET\r\n\r\n",         "GET sans URI ni version"),
        (b"\r\n\r\n",            "requête vide"),
    ]
    for raw_req, label in cases:
        try:
            raw = raw_request(raw_req)
            check_raw_status(f"Requête malformée ({label})  → 400", raw, 400)
        except Exception as e:
            err(f"Requête malformée ({label})", e)

    sub("Headers de réponse minimaux obligatoires")
    try:
        raw = raw_request(
            b"GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n"
        )
        hdrs = _headers_of(raw)
        check("Content-Type présent dans la réponse",
              "content-type:" in hdrs,
              detail=f"Headers reçus :\n{raw.split(chr(13)+chr(10)+chr(13)+chr(10))[0]!r}")
        check("Content-Length ou Transfer-Encoding présent",
              "content-length:" in hdrs or "transfer-encoding:" in hdrs,
              detail="Aucun des deux headers présents")
    except Exception as e:
        err("Headers minimaux", e)

    sub("Méthode non supportée → 405 ou 501")
    for method in ("PATCH", "PUT", "OPTIONS"):
        try:
            raw = raw_request(
                f"{method} / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n"
                .encode()
            )
            code = _status_of(raw)
            check(f"{method} /  → 405 ou 501",
                  code in (405, 501),
                  detail=f"Attendu : 405 ou 501 | Reçu : {code}")
        except Exception as e:
            err(f"{method} /", e)

# ═════════════════════════════════════════════════════════════════════════════
# ②  FICHIERS STATIQUES & MIME TYPES
# ═════════════════════════════════════════════════════════════════════════════
def test_static_files():
    section("②  FICHIERS STATIQUES & MIME TYPES")
    if not check_server_still_up(): return

    sub("Serving d'une page HTML")
    try:
        r = GET("/")
        check_r("GET /  → 200", r, 200)
        ct = r.headers.get("Content-Type", "")
        check("Content-Type contient 'text/html'",
              "text/html" in ct,
              detail=f"Reçu Content-Type : {ct!r}")
        check("Corps non vide", len(r.content) > 0,
              detail=f"Corps vide (0 octets)")
    except Exception as e:
        err("GET / statique", e)

    sub("Exactitude du Content-Length")
    try:
        r = GET("/index.html")
        if r.status_code == 200:
            cl       = int(r.headers.get("Content-Length", -1))
            body_len = len(r.content)
            check(f"Content-Length ({cl}) correspond au corps ({body_len})",
                  cl == body_len or cl == -1,
                  detail=f"Attendu CL={body_len} | Reçu CL={cl}")
        else:
            info(f"GET /index.html → {r.status_code} (test Content-Length ignoré)")
    except Exception as e:
        err("Content-Length", e)

    sub("Fichier inexistant → 404 avec page HTML")
    try:
        r = GET("/this_file_does_not_exist_eval.html")
        check_r("GET fichier absent  → 404", r, 404)
        check("Page 404 en HTML",
              "<html" in r.text.lower(),
              detail=f"Body reçu : {r.text[:150]!r}")
    except Exception as e:
        err("GET 404", e)

    sub("Fichier dans un sous-dossier")
    try:
        r = GET("/files/eval_keep.txt")
        check_r("GET /files/eval_keep.txt  → 200", r, 200)
    except Exception as e:
        err("GET /files/eval_keep.txt", e)

# ═════════════════════════════════════════════════════════════════════════════
# ③  AUTOINDEX (directory listing)
# ═════════════════════════════════════════════════════════════════════════════
def test_autoindex():
    section("③  AUTOINDEX  (directory listing)")
    if not check_server_still_up(): return

    sub("Location /files/ : autoindex ON → listing HTML")
    try:
        r = GET("/files/")
        check_r("GET /files/  → 200", r, 200)
        body = r.text.lower()
        check("Corps contient des liens <a href",
              "<a" in body or "href" in body,
              detail=f"Body reçu : {r.text[:200]!r}")
        check("Corps est du HTML",
              "<html" in body,
              detail=f"Body reçu : {r.text[:100]!r}")
    except Exception as e:
        err("GET /files/ autoindex", e)

    sub("Location / : autoindex OFF → index.html servi (pas de listing)")
    try:
        r = GET("/")
        check_r("GET /  → 200 (index.html, pas de listing)", r, 200)
    except Exception as e:
        err("GET / autoindex off", e)

    sub("Répertoire sans index et autoindex OFF → 403 ou 404")
    try:
        r = GET("/uploads/")
        status = r.status_code
        check("GET /uploads/ (autoindex OFF)  → 403 ou 404",
              status in (403, 404),
              detail=f"Attendu : 403 ou 404 | Reçu : {status}")
        info(f"→ {status}")
    except Exception as e:
        err("GET /uploads/ autoindex off", e)

# ═════════════════════════════════════════════════════════════════════════════
# ④  MÉTHODES HTTP  GET / POST / DELETE
# ═════════════════════════════════════════════════════════════════════════════
def test_methods():
    section("④  MÉTHODES HTTP  GET / POST / DELETE")
    if not check_server_still_up(): return

    sub("GET — ressources existantes")
    for path in ("/", "/index.html", "/files/eval_keep.txt", "/files/"):
        try:
            r = GET(path)
            check_r(f"GET {path}  → 200", r, 200)
        except Exception as e:
            err(f"GET {path}", e)

    sub("POST — upload corps brut vers /uploads")
    try:
        r = POST("/uploads",
                 data=b"Fichier eval test upload raw\n",
                 headers={"Content-Type": "multipart/form-data"})
        check("POST /uploads (raw)  → 2xx",
              r.status_code in (200, 201, 204),
              detail=f"Attendu : 2xx | Reçu : {r.status_code}")
        if r.status_code not in (200, 201, 204):
            _show_body(r.text)
    except Exception as e:
        err("POST /uploads raw", e)

    sub("POST — multipart/form-data vers /uploads")
    try:
        files = {"file": ("eval_mp.txt", b"multipart eval content\n", "text/plain")}
        r = POST("/uploads", files=files)
        check("POST /uploads (multipart)  → 2xx",
              r.status_code in (200, 201, 204),
              detail=f"Attendu : 2xx | Reçu : {r.status_code}")
        if r.status_code not in (200, 201, 204):
            _show_body(r.text)
    except Exception as e:
        err("POST /uploads multipart", e)

    sub("DELETE — suppression d'un fichier existant")
    try:
        r = DELETE("/files/eval_del1.txt")
        check("DELETE /files/eval_del1.txt  → 200 ou 204",
              r.status_code in (200, 204),
              detail=f"Attendu : 200 ou 204 | Reçu : {r.status_code}")
        if r.status_code in (200, 204):
            p = os.path.join(FILES_DIR, "eval_del1.txt")
            check("  fichier absent du disque après DELETE",
                  not os.path.exists(p),
                  detail="Le fichier est toujours présent sur le disque")
        else:
            _show_body(r.text)
    except Exception as e:
        err("DELETE /files/eval_del1.txt", e)

    sub("DELETE — fichier inexistant → 404")
    try:
        r = DELETE("/files/ghost_eval_xyz.txt")
        check_r("DELETE fichier inexistant  → 404", r, 404)
    except Exception as e:
        err("DELETE ghost", e)

    sub("eval_keep.txt n'a pas disparu (DELETE ciblé uniquement)")
    p_keep = os.path.join(FILES_DIR, "eval_keep.txt")
    check("eval_keep.txt toujours présent sur le disque",
          os.path.exists(p_keep))

    sub("Méthodes interdites sur / → 405")
    for fn, name in [(POST, "POST"), (DELETE, "DELETE")]:
        try:
            r = fn("/")
            check_r(f"{name} /  → 405 Method Not Allowed", r, 405)
        except Exception as e:
            err(f"{name} /", e)

    sub("DELETE interdit sur /uploads → 405")
    try:
        r = DELETE("/uploads/anything.txt")
        check_r("DELETE /uploads/anything.txt  → 405", r, 405)
    except Exception as e:
        err("DELETE /uploads", e)

# ═════════════════════════════════════════════════════════════════════════════
# ⑤  PAGES D'ERREUR PERSONNALISÉES
# ═════════════════════════════════════════════════════════════════════════════
def test_error_pages():
    section("⑤  PAGES D'ERREUR PERSONNALISÉES")
    if not check_server_still_up(): return

    cases = [
        (GET,    "/eval_404_nope",    404, "Not Found"),
        (DELETE, "/",                 405, "Method Not Allowed"),
        (POST,   "/",                 405, "Method Not Allowed"),
        (DELETE, "/new",              405, "Method Not Allowed"),
    ]
    for fn, path, expected, label in cases:
        try:
            r = fn(path)
            passed = check_r(f"{fn.__name__.upper()} {path}  → {expected} ({label})",
                             r, expected)
            if passed:
                check(f"  page d'erreur {expected} contient du HTML",
                      "<html" in r.text.lower(),
                      detail=f"Body reçu : {r.text[:150]!r}")
        except Exception as e:
            err(f"{fn.__name__.upper()} {path}", e)

    sub("400 Bad Request (raw — pas de Host en HTTP/1.1)")
    try:
        raw = raw_request(b"GET / HTTP/1.1\r\nConnection: close\r\n\r\n")
        warn_stale(raw)
        check_raw_status("HTTP/1.1 sans Host  → 400", raw, 400)
        check("  réponse 400 contient du HTML",
              "<html" in raw.lower(),
              detail="Pas de HTML dans la réponse 400")
    except Exception as e:
        err("400 raw", e)

# ═════════════════════════════════════════════════════════════════════════════
# ⑥  BODY SIZE LIMIT  (client_max_body_size)
# ═════════════════════════════════════════════════════════════════════════════
def test_body_size():
    section("⑥  BODY SIZE LIMIT  (client_max_body_size)")
    if not check_server_still_up(): return

    sub("Corps à la limite exacte (1 048 576 octets sur 8080) → 2xx")
    try:
        body = b"A" * (1024 * 1024)
        r = POST("/uploads", data=body,
                 headers={"Content-Type": "multipart/form-data"})
        info(f"POST exactement 1 MB  → {r.status_code}")
        check("POST 1 MB exact  → 2xx",
              r.status_code in (200, 201, 204),
              detail=f"Attendu : 2xx | Reçu : {r.status_code}")
    except Exception as e:
        err("POST 1 MB exact", e)

    sub("Corps dépassant la limite (1 MB + 1) → 413 Payload Too Large")
    if not check_server_still_up(): return
    try:
        body = b"B" * (1024 * 1024 + 1)
        r = POST("/uploads", data=body,
                 headers={"Content-Type": "multipart/form-data"})
        passed = check_r("POST 1 MB + 1 octet  → 413", r, 413)
        if passed:
            check("  page 413 en HTML", "<html" in r.text.lower(),
                  detail=f"Body : {r.text[:150]!r}")
    except Exception as e:
        err("POST > 1 MB", e)

    sub("Limite 512 octets sur port 8081 → 413 si dépassée")
    if server_up(PORT_8081):
        try:
            body = b"C" * 513
            r = POST("/blabla", base=BASE_8081, data=body)
            check_r("POST 513 B sur 8081  → 413", r, 413)
        except Exception as e:
            err("POST 513 B sur 8081", e)
    else:
        skip("Port 8081 non disponible")

    sub("Corps 0 octet — POST vide accepté")
    if not check_server_still_up(): return
    try:
        r = POST("/uploads", data=b"",
                 headers={"Content-Type": "multipart/form-data"})
        info(f"POST vide  → {r.status_code}  (pas de crash attendu)")
        check("POST vide  → pas de 5xx",
              r.status_code < 500,
              detail=f"Reçu : {r.status_code}")
    except Exception as e:
        err("POST vide", e)

# ═════════════════════════════════════════════════════════════════════════════
# ⑦  REDIRECTIONS  (301 / 302)
# ═════════════════════════════════════════════════════════════════════════════
def test_redirections():
    section("⑦  REDIRECTIONS  (301 / 302)")
    if not check_server_still_up(): return

    sub("301 /old → /new")
    try:
        r = GET("/old")
        passed = check_r("GET /old  → 301 Moved Permanently", r, 301)
        if passed:
            loc = r.headers.get("Location", "")
            check("  header Location présent",
                  loc != "",
                  detail=f"Reçu Location : {loc!r}")
            check("  Location pointe vers /new",
                  "/new" in loc,
                  detail=f"Attendu : '/new' dans Location | Reçu : {loc!r}")
        else:
            hdrs = dict(r.headers)
            info(f"Headers reçus : {hdrs}")
    except Exception as e:
        err("GET /old", e)

    sub("Cible de la redirection /new → 200")
    try:
        r = GET("/new")
        check_r("GET /new  → 200", r, 200)
    except Exception as e:
        err("GET /new", e)

    sub("Suivi automatique de la redirection (allow_redirects=True)")
    try:
        r = requests.get(BASE_8080 + "/old", timeout=TIMEOUT, allow_redirects=True)
        check_r("GET /old suivi auto  → 200 final", r, 200)
    except Exception as e:
        err("GET /old follow", e)

    sub("301 /blabla → /new sur port 8081")
    if server_up(PORT_8081):
        try:
            r = GET("/blabla", base=BASE_8081)
            check_r("GET /blabla 8081  → 301", r, 301)
        except Exception as e:
            err("GET /blabla 8081", e)
    else:
        skip("Port 8081 non disponible")

# ═════════════════════════════════════════════════════════════════════════════
# ⑧  CGI  (GET query string + POST stdin + timeout 504)
# ═════════════════════════════════════════════════════════════════════════════
def test_cgi():
    section("⑧  CGI  (GET query string + POST stdin + timeout 504)")
    if not check_server_still_up(): return

    sub("CGI GET — script.py (Hello from CGI)")
    try:
        r = GET("/cgi-bin/script.py")
        passed = check_r("GET /cgi-bin/script.py  → 200", r, 200)
        if passed:
            check("  corps contient du HTML",
                  "<html" in r.text.lower(),
                  detail=f"Body reçu : {r.text[:200]!r}")
    except Exception as e:
        err("GET /cgi-bin/script.py", e)

    sub("CGI GET — QUERY_STRING transmise dans l'environnement")
    try:
        r = GET("/cgi-bin/cgi_test.py?ecole=42&ville=paris")
        passed = check_r("GET /cgi-bin/cgi_test.py?ecole=42  → 200", r, 200)
        if passed:
            check("  '42' visible dans la réponse (QUERY_STRING reçue)",
                  "42" in r.text,
                  detail=f"Attendu : '42' dans body | Body : {r.text[:200]!r}")
            check("  REQUEST_METHOD=GET visible",
                  "GET" in r.text,
                  detail=f"Body : {r.text[:200]!r}")
    except Exception as e:
        err("CGI GET query string", e)

    sub("CGI POST — body transmis via stdin (form-urlencoded)")
    try:
        payload = "user=student42&token=abc123"
        r = POST("/cgi-bin/cgi_test.py", data=payload,
                 headers={"Content-Type": "application/x-www-form-urlencoded"})
        passed = check_r("POST /cgi-bin/cgi_test.py  → 200", r, 200)
        if passed:
            check("  'student42' visible dans la réponse (stdin bien lu)",
                  "student42" in r.text,
                  detail=f"Attendu : 'student42' dans body | Body : {r.text[:200]!r}")
            check("  REQUEST_METHOD=POST visible",
                  "POST" in r.text,
                  detail=f"Body : {r.text[:200]!r}")
    except Exception as e:
        err("CGI POST form", e)

    sub("CGI POST — corps JSON")
    try:
        import json
        payload = json.dumps({"ecole": "42", "projet": "webserv"})
        r = POST("/cgi-bin/cgi_test.py", data=payload,
                 headers={"Content-Type": "application/json"})
        passed = check_r("POST CGI JSON  → 200", r, 200)
        if passed:
            check("  payload JSON reflété dans la réponse",
                  "webserv" in r.text,
                  detail=f"Attendu : 'webserv' dans body | Body : {r.text[:200]!r}")
    except Exception as e:
        err("CGI POST JSON", e)

    sub("CGI CONTENT_LENGTH — vérification variable d'environnement")
    try:
        payload = "payload_length_test=hello"
        r = POST("/cgi-bin/cgi_test.py", data=payload,
                 headers={"Content-Type": "application/x-www-form-urlencoded"})
        if r.status_code == 200:
            cl_str = str(len(payload))
            check(f"  CONTENT_LENGTH={cl_str} visible dans la réponse",
                  cl_str in r.text,
                  detail=f"Attendu : '{cl_str}' dans body | Body : {r.text[:200]!r}")
    except Exception as e:
        err("CGI CONTENT_LENGTH", e)

    sub("CGI timeout — script infini → 504 Gateway Timeout")
    info("Envoi d'un CGI infini (post.py) — attente max 15s…")
    try:
        r = requests.get(BASE_8080 + "/cgi-bin/post.py",
                         timeout=15, allow_redirects=False)
        check_r("CGI boucle infinie (post.py)  → 504", r, 504)
        if r.status_code != 504:
            info("Le serveur doit tuer le CGI après timeout et répondre 504")
    except requests.exceptions.Timeout:
        fail("CGI infini : pas de 504 en 15s")
        info("Le serveur semble bloqué sur le CGI — timeout CGI non implémenté")
    except Exception as e:
        err("CGI timeout post.py", e)

    sub("CGI : méthode DELETE non autorisée → 405")
    if not check_server_still_up(): return
    try:
        r = DELETE("/cgi-bin/cgi_test.py")
        check_r("DELETE /cgi-bin  → 405", r, 405)
    except Exception as e:
        err("DELETE /cgi-bin", e)

# ═════════════════════════════════════════════════════════════════════════════
# ⑨  CYCLE UPLOAD  (POST → fichier sur disque)
# ═════════════════════════════════════════════════════════════════════════════
def test_upload_cycle():
    section("⑨  CYCLE UPLOAD  (POST → vérification sur le disque)")
    if not check_server_still_up(): return

    sub("POST /uploads — corps brut")
    t_before = time.time()
    try:
        content = b"Contenu eval upload cycle " + str(t_before).encode() + b"\n"
        r = POST("/uploads", data=content,
                 headers={"Content-Type": "text/plain",
                          "X-Filename": "eval_cycle.txt"})
        check("POST /uploads  → 2xx",
              r.status_code in (200, 201, 204),
              detail=f"Attendu : 2xx | Reçu : {r.status_code}")
        if r.status_code not in (200, 201, 204):
            _show_body(r.text)
            return
    except Exception as e:
        err("POST /uploads cycle", e)
        return

    sub("Vérification du fichier sur le disque (upload_path = ./var/www)")
    upload_root = os.path.join(WEBSERV_DIR, "var", "www")
    recent = []
    for fname in os.listdir(upload_root):
        p = os.path.join(upload_root, fname)
        if os.path.isfile(p) and os.path.getmtime(p) >= t_before - 1:
            recent.append(fname)
    check("Fichier uploadé présent sur le disque",
          len(recent) > 0,
          detail=f"Aucun fichier récent trouvé dans {upload_root}")
    if recent:
        info(f"Fichier(s) uploadé(s) détecté(s) : {', '.join(recent)}")
        for fname in recent:
            try:
                os.remove(os.path.join(upload_root, fname))
            except Exception:
                pass

# ═════════════════════════════════════════════════════════════════════════════
# ⑩  VIRTUAL HOSTING  (Host header + multi-serveur)
# ═════════════════════════════════════════════════════════════════════════════
def test_virtual_host():
    section("⑩  VIRTUAL HOSTING  (Host header + multi-serveur)")
    if not check_server_still_up(): return

    sub("Port 8080 répond avec Host: localhost")
    try:
        r = GET("/", headers={"Host": "localhost"})
        check_r("Host: localhost sur 8080  → 200", r, 200)
    except Exception as e:
        err("Host: localhost 8080", e)

    sub("Port 8080 — Host inconnu → réponse (pas de crash)")
    try:
        r = GET("/", headers={"Host": "unknown.domain.xyz"})
        check("Host: inconnu sur 8080  → réponse valide (< 600)",
              r.status_code < 600,
              detail=f"Reçu : {r.status_code}")
        info(f"→ {r.status_code} (le serveur par défaut répond, pas de crash)")
    except Exception as e:
        err("Host: inconnu 8080", e)

    sub("Port 8081 répond avec Host: test.local")
    if server_up(PORT_8081):
        try:
            r = GET("/", base=BASE_8081, headers={"Host": "test.local"})
            check("Host: test.local sur 8081  → 2xx ou 3xx",
                  r.status_code < 500,
                  detail=f"Attendu : <500 | Reçu : {r.status_code}")
        except Exception as e:
            err("Host: test.local 8081", e)
    else:
        skip("Port 8081 non disponible")

    sub("Les deux ports sont indépendants")
    if server_up(PORT_8081):
        try:
            r0 = GET("/", base=BASE_8080)
            r1 = GET("/", base=BASE_8081)
            check("8080 et 8081 répondent tous les deux",
                  r0.status_code < 500 and r1.status_code < 500,
                  detail=f"8080={r0.status_code}  8081={r1.status_code}")
        except Exception as e:
            err("Deux ports indépendants", e)
    else:
        skip("Port 8081 non disponible")

# ═════════════════════════════════════════════════════════════════════════════
# ⑪  KEEP-ALIVE  (connexion persistante)
# ═════════════════════════════════════════════════════════════════════════════
def test_keepalive():
    section("⑪  KEEP-ALIVE  (connexion persistante TCP)")
    if not check_server_still_up(): return

    sub("Deux requêtes GET successives sur la même socket")
    try:
        req1 = b"GET / HTTP/1.1\r\nHost: localhost\r\nConnection: keep-alive\r\n\r\n"
        req2 = b"GET /index.html HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n"

        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(TIMEOUT)
        s.connect((HOST, PORT_8080))
        s.sendall(req1)
        time.sleep(0.3)
        s.sendall(req2)

        data = b""
        try:
            while True:
                chunk = s.recv(4096)
                if not chunk:
                    break
                data += chunk
        except socket.timeout:
            pass
        s.close()

        decoded = data.decode("utf-8", errors="replace")
        n_responses = decoded.count("HTTP/1.1")
        check("Deux réponses HTTP/1.1 reçues sur une connexion keep-alive",
              n_responses >= 2,
              detail=f"Attendu : ≥2 blocs HTTP/1.1 | Reçu : {n_responses}")
        if n_responses < 2:
            info(f"Données reçues : {decoded[:300]!r}")
    except Exception as e:
        err("Keep-alive — deux requêtes", e)

    sub("Connection: close ferme la connexion après la réponse")
    try:
        raw = raw_request(
            b"GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n"
        )
        check_raw_status("Réponse reçue avec Connection: close  → 200", raw, 200)
    except Exception as e:
        err("Connection: close", e)

    sub("Requête pipelinée (deux requêtes sans attendre la première réponse)")
    try:
        req = (
            b"GET / HTTP/1.1\r\nHost: localhost\r\nConnection: keep-alive\r\n\r\n"
            b"GET /index.html HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n"
        )
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(TIMEOUT)
        s.connect((HOST, PORT_8080))
        s.sendall(req)
        data = b""
        try:
            while True:
                chunk = s.recv(4096)
                if not chunk:
                    break
                data += chunk
        except socket.timeout:
            pass
        s.close()
        decoded = data.decode("utf-8", errors="replace")
        n = decoded.count("HTTP/1.1")
        info(f"Pipelining : {n} réponse(s) reçue(s) (≥2 idéal, ≥1 acceptable)")
        check("Au moins une réponse à la requête pipelinée",
              n >= 1,
              detail=f"Reçu {n} réponse(s)")
    except Exception as e:
        err("Pipelining", e)

# ═════════════════════════════════════════════════════════════════════════════
# ⑫  STRESS TEST  (style siege — disponibilité ≥ 99 %)
# ═════════════════════════════════════════════════════════════════════════════
def test_siege():
    section("⑫  STRESS TEST  (style siege — disponibilité ≥ 99 %)")
    if not check_server_still_up(): return

    def run_siege(label, base, path, method, n_threads, n_req):
        total   = n_threads * n_req
        results = [None] * total

        def worker(tid):
            for i in range(n_req):
                t0 = time.perf_counter()
                try:
                    if method == "GET":
                        r = requests.get(base + path, timeout=TIMEOUT,
                                         allow_redirects=False)
                    else:
                        r = requests.post(base + path, timeout=TIMEOUT,
                                          allow_redirects=False,
                                          data=b"siege payload",
                                          headers={"Content-Type":
                                                   "application/octet-stream"})
                    results[tid * n_req + i] = (r.status_code,
                                                 time.perf_counter() - t0, None)
                except Exception as exc:
                    results[tid * n_req + i] = (0, time.perf_counter() - t0,
                                                 str(exc))

        sub(f"{label}  [{n_threads} threads × {n_req} req = {total} total]")
        t_start = time.perf_counter()
        threads = [threading.Thread(target=worker, args=(t,)) for t in range(n_threads)]
        for th in threads: th.start()
        for th in threads: th.join()
        elapsed = time.perf_counter() - t_start

        ok_set   = {200, 201, 204, 301, 302, 405, 413}
        hits     = [r for r in results if r and r[2] is None and r[0] in ok_set]
        net_errs = [r for r in results if r and r[2] is not None]
        srv_errs = [r for r in results if r and r[2] is None and r[0] >= 500]
        times    = [r[1] for r in results if r and r[2] is None]
        avail    = len(hits) * 100.0 / total if total else 0
        rps      = total / elapsed if elapsed > 0 else 0
        avg_ms   = sum(times) / len(times) * 1000 if times else 0
        max_ms   = max(times) * 1000 if times else 0

        check(f"Disponibilité ≥ 99 %  ({avail:.1f} %  {len(hits)}/{total})",
              avail >= 99.0,
              detail=f"Attendu : ≥99% | Obtenu : {avail:.1f}%  "
                     f"({len(net_errs)} err réseau, {len(srv_errs)} err 5xx)")
        check("Aucune erreur réseau / timeout",
              len(net_errs) == 0,
              detail=f"{len(net_errs)} connexion(s) échouée(s)")
        check("Aucun 5xx sous charge",
              len(srv_errs) == 0,
              detail=f"{len(srv_errs)} réponse(s) 5xx reçues")

        info(f"Durée : {elapsed:.2f}s  |  {rps:.0f} req/s  |  "
             f"avg {avg_ms:.1f}ms  |  max {max_ms:.1f}ms")
        if net_errs:
            info(f"1ère erreur réseau : {net_errs[0][2][:80]}")
        if srv_errs:
            info(f"1er code 5xx reçu  : {srv_errs[0][0]}")

    run_siege("GET /  (charge principale — pages statiques)",
              BASE_8080, "/",                    "GET",  100, 10)
    if not check_server_still_up(): return
    run_siege("GET /files/  (autoindex sous charge)",
              BASE_8080, "/files/",              "GET",   50, 10)
    if not check_server_still_up(): return
    run_siege("POST /uploads  (upload sous charge)",
              BASE_8080, "/uploads",             "POST",  50,  5)
    if not check_server_still_up(): return
    run_siege("GET /cgi-bin/cgi_test.py  (CGI sous charge)",
              BASE_8080, "/cgi-bin/cgi_test.py?stress=1", "GET", 20, 5)

    sub("Serveur toujours opérationnel après le stress")
    if not check_server_still_up(): return
    try:
        r = GET("/")
        check_r("GET / après stress  → 200", r, 200)
    except Exception as e:
        err("GET / post-stress", e)
        fail("Le serveur ne répond plus après le stress test")

# ═════════════════════════════════════════════════════════════════════════════
# MAIN
# ═════════════════════════════════════════════════════════════════════════════
def main():
    bar = "━" * 62
    print(f"\n{BOLD}{CYAN}{bar}{RESET}")
    print(f"{BOLD}{CYAN}  WEBSERV EVAL TESTER — École 42{RESET}")
    print(f"{BOLD}{CYAN}  {datetime.now().strftime('%Y-%m-%d  %H:%M:%S')}{RESET}")
    print(f"{BOLD}{CYAN}{bar}{RESET}")

    print(f"\n{BOLD}Vérification des serveurs…{RESET}")
    up8080 = server_up(PORT_8080)
    up8081 = server_up(PORT_8081)
    sym = lambda up: f"{GREEN}✓{RESET}" if up else f"{RED}✗{RESET}"
    print(f"  {sym(up8080)} port {PORT_8080}  {'OK' if up8080 else 'injoignable'}")
    print(f"  {sym(up8081)} port {PORT_8081}  {'OK' if up8081 else 'injoignable (tests 8081 sautés)'}")

    if not up8080:
        print(f"\n{RED}{BOLD}Serveur non accessible sur le port 8080.{RESET}")
        print(f"Lance :  {BOLD}./webserv full.cnf{RESET}\n")
        sys.exit(1)

    # Détection de réponse parasite au démarrage
    print(f"\n{DIM}Vérification réponse parasite (CGI/timeout traîné)…{RESET}")
    try:
        raw = raw_request(
            b"GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n"
        )
        n = raw.count("HTTP/1.1")
        if n > 1:
            print(f"  {YELLOW}{BOLD}⚠  Réponse parasite détectée : {n} blocs HTTP/1.1 reçus pour 1 requête{RESET}")
            print(f"  {DIM}   Le serveur envoie une réponse en attente d'un client précédent.{RESET}")
            print(f"  {DIM}   Les status codes reçus seront ceux du PREMIER bloc (souvent 500/504).{RESET}")
            print(f"  {DIM}   → Redémarre ./webserv full.cnf pour repartir propre.{RESET}")
        else:
            print(f"  {GREEN}✓{RESET}  Pas de réponse parasite")
    except Exception:
        pass

    print(f"\n{DIM}Préparation des fichiers de test…{RESET}")
    setup()

    test_http_protocol()
    test_static_files()
    test_autoindex()
    test_methods()
    test_error_pages()
    test_body_size()
    test_redirections()
    test_cgi()
    test_upload_cycle()
    test_virtual_host()
    test_keepalive()
    test_siege()

    cleanup()

    total = _passed + _failed + _errors
    pct   = (_passed * 100 // total) if total else 0

    print(f"\n{BOLD}{CYAN}{bar}{RESET}")
    print(f"{BOLD}  RÉSULTATS FINAUX{RESET}")
    print(f"{BOLD}{CYAN}{bar}{RESET}")
    print(f"  {GREEN}{BOLD}PASS   {_passed:3d}{RESET}")
    print(f"  {RED}{BOLD}FAIL   {_failed:3d}{RESET}")
    print(f"  {YELLOW}{BOLD}ERR    {_errors:3d}{RESET}")
    print(f"  {'─'*14}")
    print(f"  {BOLD}TOTAL  {total:3d}   ({pct}%){RESET}")

    if _failed == 0 and _errors == 0:
        print(f"\n  {GREEN}{BOLD}Parfait — tous les critères d'évaluation passent !{RESET}")
    else:
        print(f"\n  {YELLOW}{BOLD}{_failed} échec(s) et {_errors} erreur(s) à corriger.{RESET}")
    print(f"{BOLD}{CYAN}{bar}{RESET}\n")

    sys.exit(0 if (_failed == 0 and _errors == 0) else 1)


if __name__ == "__main__":
    main()
