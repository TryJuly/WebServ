#!/usr/bin/env python3
"""
WebServ Tester — École 42
Tests : GET | POST | DELETE | Redirects | CGI GET+POST | Stress Test

Usage :
    ./webserv full.cnf &
    python3 tester.py
"""

import os
import sys
import time
import socket
import threading
import subprocess

try:
    import requests
except ImportError:
    print("Le module 'requests' est requis.  Installe-le avec :")
    print("    pip3 install requests")
    sys.exit(1)

from datetime import datetime
from concurrent.futures import ThreadPoolExecutor

# ─── Couleurs ANSI ───────────────────────────────────────────────────────────
RED    = "\033[91m"
GREEN  = "\033[92m"
YELLOW = "\033[93m"
BLUE   = "\033[94m"
CYAN   = "\033[96m"
BOLD   = "\033[1m"
DIM    = "\033[2m"
RESET  = "\033[0m"

# ─── Config ──────────────────────────────────────────────────────────────────
BASE_8080   = "http://127.0.0.1:8080"
BASE_8081   = "http://127.0.0.1:8081"
TIMEOUT     = 6
WEBSERV_DIR = os.path.dirname(os.path.abspath(__file__))
FILES_DIR   = os.path.join(WEBSERV_DIR, "var", "www", "html", "files")
CGI_DIR     = os.path.join(WEBSERV_DIR, "var", "www", "cgi-bin")

# ─── Compteurs thread-safe ───────────────────────────────────────────────────
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

# ─── Affichage ───────────────────────────────────────────────────────────────
def ok(msg):
    _incr("p")
    print(f"  {GREEN}[PASS]{RESET} {msg}")

def fail(msg, detail=""):
    _incr("f")
    sfx = f"  {DIM}→ {detail}{RESET}" if detail else ""
    print(f"  {RED}[FAIL]{RESET} {msg}{sfx}")

def skip(msg):
    print(f"  {YELLOW}[SKIP]{RESET} {msg}")

def err(msg, exc=""):
    _incr("e")
    print(f"  {YELLOW}[ERR ]{RESET} {msg}: {DIM}{exc}{RESET}")

def section(title):
    bar = "═" * 58
    print(f"\n{BOLD}{CYAN}╔{bar}╗{RESET}")
    print(f"{BOLD}{CYAN}║  {title:<56}║{RESET}")
    print(f"{BOLD}{CYAN}╚{bar}╝{RESET}")

def sub(title):
    print(f"\n  {BOLD}{BLUE}▸ {title}{RESET}")

def info(msg):
    print(f"    {DIM}{msg}{RESET}")

def check(name, cond, detail=""):
    if cond:
        ok(name)
    else:
        fail(name, detail)

# ─── Requêtes HTTP ───────────────────────────────────────────────────────────
def GET(path, base=BASE_8080, **kw):
    return requests.get(base + path, timeout=TIMEOUT, allow_redirects=False, **kw)

def POST(path, base=BASE_8080, **kw):
    return requests.post(base + path, timeout=TIMEOUT, allow_redirects=False, **kw)

def DELETE(path, base=BASE_8080, **kw):
    return requests.delete(base + path, timeout=TIMEOUT, allow_redirects=False, **kw)

# ─── Setup / Cleanup ─────────────────────────────────────────────────────────
_SETUP_FILES = []

def setup():
    """Crée les fichiers et dossiers nécessaires aux tests."""
    os.makedirs(FILES_DIR, exist_ok=True)

    # Fichiers à supprimer via DELETE
    for name in ("delete_me.txt", "delete_me2.txt", "delete_me3.txt"):
        p = os.path.join(FILES_DIR, name)
        with open(p, "w") as f:
            f.write(f"Fichier de test WebServ — {name}\n")
        _SETUP_FILES.append(p)

    # Fichier permanent (ne doit pas disparaître après un DELETE sur un autre)
    p = os.path.join(FILES_DIR, "keep_me.txt")
    with open(p, "w") as f:
        f.write("Ce fichier ne doit PAS être supprimé.\n")
    _SETUP_FILES.append(p)

    info(f"Fichiers de test créés dans {FILES_DIR}")

def cleanup():
    """Supprime les fichiers résiduels créés par setup()."""
    for p in _SETUP_FILES:
        if os.path.exists(p):
            os.remove(p)

# ─── Disponibilité du serveur ────────────────────────────────────────────────
def server_up(host, port):
    try:
        s = socket.create_connection((host, port), timeout=2)
        s.close()
        return True
    except Exception:
        return False

# ═════════════════════════════════════════════════════════════════════════════
# 1. TESTS GET
# ═════════════════════════════════════════════════════════════════════════════
def test_get():
    section("1 / 7  ──  GET  (port 8080)")

    sub("Pages existantes")
    for path, label in [("/", "racine"), ("/index.html", "index explicite")]:
        try:
            r = GET(path)
            check(f"GET {path}  [{label}]  → 200", r.status_code == 200,
                  f"status={r.status_code}")
        except Exception as e:
            err(f"GET {path}", e)

    sub("Fichiers dans /files (autoindex ON)")
    try:
        r = GET("/files/")
        check("GET /files/  → 200  (listing activé)", r.status_code == 200,
              f"status={r.status_code}")
        if r.status_code == 200:
            check("  réponse contient du HTML", "<" in r.text,
                  f"body={r.text[:80]!r}")
    except Exception as e:
        err("GET /files/", e)

    try:
        r = GET("/files/keep_me.txt")
        check("GET /files/keep_me.txt  → 200", r.status_code == 200,
              f"status={r.status_code}")
    except Exception as e:
        err("GET /files/keep_me.txt", e)

    sub("Ressource inexistante → 404")
    for path in ("/nope", "/files/ghost.txt", "/uploads/nothing"):
        try:
            r = GET(path)
            check(f"GET {path}  → 404", r.status_code == 404,
                  f"status={r.status_code}")
        except Exception as e:
            err(f"GET {path}", e)

    sub("Méthode interdite sur / → 405")
    try:
        r = DELETE("/")
        check("DELETE /  → 405", r.status_code == 405,
              f"status={r.status_code}")
    except Exception as e:
        err("DELETE /", e)

# ═════════════════════════════════════════════════════════════════════════════
# 2. TESTS POST
# ═════════════════════════════════════════════════════════════════════════════
def test_post():
    section("2 / 7  ──  POST  (port 8080)")

    sub("Upload corps brut vers /uploads")
    try:
        r = POST("/uploads",
                 data=b"Contenu brut du fichier de test\n",
                 headers={"Content-Type": "application/octet-stream",
                          "X-Filename": "raw_upload.txt"})
        check("POST /uploads  (raw)  → 2xx",
              r.status_code in (200, 201, 204),
              f"status={r.status_code}")
    except Exception as e:
        err("POST /uploads raw", e)

    sub("Upload multipart/form-data vers /uploads")
    try:
        files = {"file": ("mp_test.txt",
                          b"Contenu multipart de test\n",
                          "text/plain")}
        r = POST("/uploads", files=files)
        check("POST /uploads  (multipart)  → 2xx",
              r.status_code in (200, 201, 204),
              f"status={r.status_code}")
    except Exception as e:
        err("POST /uploads multipart", e)

    sub("POST non autorisé sur / → 405")
    try:
        r = POST("/", data=b"test")
        check("POST /  → 405", r.status_code == 405,
              f"status={r.status_code}")
    except Exception as e:
        err("POST /", e)

    sub("Corps trop grand → 413  (limite = 1 MB sur 8080)")
    try:
        big = b"X" * (1024 * 1024 + 1)
        r = POST("/uploads", data=big,
                 headers={"Content-Type": "application/octet-stream"})
        check("POST 1 MB + 1 octet  → 413",
              r.status_code == 413,
              f"status={r.status_code}")
    except Exception as e:
        err("POST body > 1 MB", e)

    sub("Corps exactement à la limite  (512 B sur port 8081)")
    try:
        r = POST("/blabla", base=BASE_8081, data=b"A" * 512)
        # /blabla redirige en 301, le POST peut renvoyer 301 ou 2xx
        info(f"POST /blabla 512 B  → {r.status_code}  (301/2xx attendu)")
    except Exception as e:
        err("POST /blabla 512B 8081", e)

    sub("Corps au-dessus de la limite (513 B sur port 8081)")
    try:
        r = POST("/blabla", base=BASE_8081, data=b"A" * 513)
        check("POST /blabla 513 B  → 413",
              r.status_code == 413,
              f"status={r.status_code}")
    except Exception as e:
        err("POST /blabla 513B 8081", e)

# ═════════════════════════════════════════════════════════════════════════════
# 3. TESTS DELETE
# ═════════════════════════════════════════════════════════════════════════════
def test_delete():
    section("3 / 7  ──  DELETE  (port 8080)")

    sub("Suppression de fichiers existants dans /files")
    for name in ("delete_me.txt", "delete_me2.txt"):
        p = os.path.join(FILES_DIR, name)
        try:
            r = DELETE(f"/files/{name}")
            check(f"DELETE /files/{name}  → 2xx",
                  r.status_code in (200, 204),
                  f"status={r.status_code}")
            if r.status_code in (200, 204):
                check(f"  fichier effacé du disque",
                      not os.path.exists(p),
                      "toujours présent sur le disque")
        except Exception as e:
            err(f"DELETE /files/{name}", e)

    sub("Fichier absent → 404")
    try:
        r = DELETE("/files/fantome.txt")
        check("DELETE fichier inexistant  → 404",
              r.status_code == 404,
              f"status={r.status_code}")
    except Exception as e:
        err("DELETE /files/fantome.txt", e)

    sub("DELETE interdit sur d'autres locations")
    for path in ("/", "/uploads/test.txt", "/new"):
        try:
            r = DELETE(path)
            check(f"DELETE {path}  → 405",
                  r.status_code == 405,
                  f"status={r.status_code}")
        except Exception as e:
            err(f"DELETE {path}", e)

    sub("keep_me.txt ne doit pas avoir été supprimé")
    p_keep = os.path.join(FILES_DIR, "keep_me.txt")
    check("keep_me.txt toujours sur le disque", os.path.exists(p_keep))

# ═════════════════════════════════════════════════════════════════════════════
# 4. REDIRECTIONS
# ═════════════════════════════════════════════════════════════════════════════
def test_redirects():
    section("4 / 7  ──  REDIRECTIONS")

    sub("301 /old → /new  (port 8080)")
    try:
        r = GET("/old")
        check("GET /old  → 301", r.status_code == 301,
              f"status={r.status_code}")
        if r.status_code == 301:
            loc = r.headers.get("Location", "")
            check("  header Location pointe vers /new", "/new" in loc,
                  f"Location={loc!r}")
    except Exception as e:
        err("GET /old", e)

    sub("GET /new  → 200  (cible de la redirection)")
    try:
        r = GET("/new")
        check("GET /new  → 200", r.status_code == 200,
              f"status={r.status_code}")
    except Exception as e:
        err("GET /new", e)

    sub("301 /blabla → /new  (port 8081)")
    try:
        r = GET("/blabla", base=BASE_8081)
        check("GET /blabla  → 301", r.status_code == 301,
              f"status={r.status_code}")
    except Exception as e:
        err("GET /blabla 8081", e)

# ═════════════════════════════════════════════════════════════════════════════
# 5. CGI
# ═════════════════════════════════════════════════════════════════════════════
def test_cgi():
    section("5 / 7  ──  CGI  (port 8080  /cgi-bin)")

    sub("CGI GET — script.py  (Hello from CGI)")
    try:
        r = GET("/cgi-bin/script.py")
        check("GET /cgi-bin/script.py  → 200",
              r.status_code == 200, f"status={r.status_code}")
        if r.status_code == 200:
            check("  corps contient 'CGI'",
                  "cgi" in r.text.lower(),
                  f"body={r.text[:120]!r}")
    except Exception as e:
        err("GET /cgi-bin/script.py", e)

    sub("CGI GET — cgi_test.py avec query string")
    try:
        r = GET("/cgi-bin/cgi_test.py?ecole=42&ville=paris")
        check("GET /cgi-bin/cgi_test.py?ecole=42  → 200",
              r.status_code == 200, f"status={r.status_code}")
        if r.status_code == 200:
            check("  query string '42' présente dans la réponse",
                  "42" in r.text,
                  f"body={r.text[:200]!r}")
            check("  méthode 'GET' dans la réponse",
                  "GET" in r.text,
                  f"body={r.text[:200]!r}")
    except Exception as e:
        err("GET /cgi-bin/cgi_test.py?ecole=42", e)

    sub("CGI POST — cgi_test.py  application/x-www-form-urlencoded")
    try:
        payload = "username=student42&password=secret"
        r = POST("/cgi-bin/cgi_test.py",
                 data=payload,
                 headers={"Content-Type": "application/x-www-form-urlencoded"})
        check("POST /cgi-bin/cgi_test.py  → 200",
              r.status_code == 200, f"status={r.status_code}")
        if r.status_code == 200:
            check("  corps du POST présent dans la réponse",
                  "student42" in r.text,
                  f"body={r.text[:200]!r}")
            check("  méthode 'POST' dans la réponse",
                  "POST" in r.text,
                  f"body={r.text[:200]!r}")
    except Exception as e:
        err("POST /cgi-bin/cgi_test.py (form)", e)

    sub("CGI POST — corps JSON")
    try:
        import json
        payload = json.dumps({"user": "42student", "action": "test"})
        r = POST("/cgi-bin/cgi_test.py",
                 data=payload,
                 headers={"Content-Type": "application/json"})
        check("POST /cgi-bin/cgi_test.py  (JSON)  → 200",
              r.status_code == 200, f"status={r.status_code}")
        if r.status_code == 200:
            check("  payload JSON reflété dans la réponse",
                  "42student" in r.text,
                  f"body={r.text[:200]!r}")
    except Exception as e:
        err("POST /cgi-bin/cgi_test.py (JSON)", e)

    sub("CGI POST — corps binaire")
    try:
        r = POST("/cgi-bin/cgi_test.py",
                 data=b"\x00\x01HELLO\xFF",
                 headers={"Content-Type": "application/octet-stream"})
        check("POST /cgi-bin/cgi_test.py  (binaire)  → 200",
              r.status_code == 200, f"status={r.status_code}")
    except Exception as e:
        err("POST /cgi-bin/cgi_test.py (binaire)", e)

    sub("CGI GET + POST — method=GET bien transmise")
    try:
        r = GET("/cgi-bin/cgi_test.py")
        if r.status_code == 200:
            check("  REQUEST_METHOD=GET visible dans la réponse",
                  "GET" in r.text,
                  f"body={r.text[:200]!r}")
    except Exception as e:
        err("GET /cgi-bin/cgi_test.py (method check)", e)

    sub("DELETE sur /cgi-bin → 405")
    try:
        r = DELETE("/cgi-bin/cgi_test.py")
        check("DELETE /cgi-bin/cgi_test.py  → 405",
              r.status_code == 405, f"status={r.status_code}")
    except Exception as e:
        err("DELETE /cgi-bin/cgi_test.py", e)

# ═════════════════════════════════════════════════════════════════════════════
# 6. PAGES D'ERREUR
# ═════════════════════════════════════════════════════════════════════════════
def test_errors():
    section("6 / 7  ──  PAGES D'ERREUR")

    cases = [
        ("GET",    "/introuvable",        404, "Not Found"),
        ("DELETE", "/",                   405, "Method Not Allowed"),
        ("POST",   "/",                   405, "Method Not Allowed"),
        ("GET",    "/files/ghost.txt",    404, "Not Found"),
    ]
    for method, path, expected, label in cases:
        func = {"GET": GET, "POST": POST, "DELETE": DELETE}[method]
        try:
            r = func(path)
            check(f"{method} {path}  → {expected}  ({label})",
                  r.status_code == expected,
                  f"status={r.status_code}")
            if r.status_code == expected:
                check(f"  page d'erreur {expected} en HTML",
                      "<html" in r.text.lower(),
                      "pas de HTML dans la réponse")
        except Exception as e:
            err(f"{method} {path}", e)

# ═════════════════════════════════════════════════════════════════════════════
# 7. STRESS TEST
# ═════════════════════════════════════════════════════════════════════════════
def _worker(base, path, method, slot, out):
    """Effectue une requête HTTP et stocke (status, elapsed, error) dans out[slot]."""
    t0 = time.perf_counter()
    try:
        if method == "GET":
            r = requests.get(base + path, timeout=TIMEOUT, allow_redirects=False)
        else:
            r = requests.post(base + path, timeout=TIMEOUT, allow_redirects=False,
                              data=b"stress")
        out[slot] = (r.status_code, time.perf_counter() - t0, None)
    except Exception as exc:
        out[slot] = (0, time.perf_counter() - t0, str(exc))


def _run_stress(label, base, path, method, n_threads, n_req_per_thread):
    total = n_threads * n_req_per_thread
    results = [None] * total
    threads = []

    sub(f"{label}  [{n_threads} threads × {n_req_per_thread} req = {total} total]")

    def thread_fn(tid):
        for i in range(n_req_per_thread):
            _worker(base, path, method, tid * n_req_per_thread + i, results)

    t_start = time.perf_counter()
    for t in range(n_threads):
        th = threading.Thread(target=thread_fn, args=(t,))
        threads.append(th)
    for th in threads:
        th.start()
    for th in threads:
        th.join()
    elapsed = time.perf_counter() - t_start

    ok_codes  = {200, 201, 204, 301, 302, 405}
    successes = [r for r in results if r and r[2] is None and r[0] in ok_codes]
    net_errs  = [r for r in results if r and r[2] is not None]
    srv_errs  = [r for r in results if r and r[2] is None and r[0] >= 500]
    times     = [r[1] for r in results if r and r[2] is None]

    avg_ms = (sum(times) / len(times) * 1000) if times else 0
    min_ms = (min(times) * 1000) if times else 0
    max_ms = (max(times) * 1000) if times else 0
    rps    = total / elapsed if elapsed > 0 else 0

    check(f"Toutes les requêtes répondent  ({len(successes)}/{total})",
          len(successes) == total,
          f"{len(net_errs)} erreurs réseau, {len(srv_errs)} erreurs 5xx")
    check("Aucune erreur réseau / timeout",
          len(net_errs) == 0,
          f"{len(net_errs)} connexions échouées")
    check("Aucun 5xx retourné par le serveur",
          len(srv_errs) == 0,
          f"{len(srv_errs)} réponses 5xx")

    info(f"Durée totale : {elapsed:.2f}s  |  ~{rps:.0f} req/s")
    info(f"Temps réponse : min={min_ms:.1f}ms  avg={avg_ms:.1f}ms  max={max_ms:.1f}ms")
    if net_errs:
        info(f"1ère erreur réseau : {net_errs[0][2][:80]}")


def test_stress():
    section("7 / 7  ──  STRESS TEST  (connexions simultanées)")

    _run_stress("GET /  (pages statiques)",
                BASE_8080, "/",                   "GET",  50, 10)
    _run_stress("GET /files/  (autoindex)",
                BASE_8080, "/files/",             "GET",  30,  5)
    _run_stress("POST /uploads  (body 6 octets)",
                BASE_8080, "/uploads",            "POST", 30,  5)
    _run_stress("GET /cgi-bin/script.py  (CGI)",
                BASE_8080, "/cgi-bin/script.py",  "GET",  20,  5)
    _run_stress("GET /cgi-bin/cgi_test.py  (CGI query)",
                BASE_8080, "/cgi-bin/cgi_test.py?stress=1", "GET", 20, 5)

# ═════════════════════════════════════════════════════════════════════════════
# MAIN
# ═════════════════════════════════════════════════════════════════════════════
def main():
    bar = "━" * 62
    print(f"\n{BOLD}{CYAN}{bar}{RESET}")
    print(f"{BOLD}{CYAN}  WEBSERV TESTER — École 42{RESET}")
    print(f"{BOLD}{CYAN}  {datetime.now().strftime('%Y-%m-%d  %H:%M:%S')}{RESET}")
    print(f"{BOLD}{CYAN}{bar}{RESET}")

    # Vérification des serveurs
    print(f"\n{BOLD}Vérification des serveurs…{RESET}")
    up_8080 = server_up("127.0.0.1", 8080)
    up_8081 = server_up("127.0.0.1", 8081)

    sym_8080 = f"{GREEN}✓ port 8080 OK{RESET}" if up_8080 else f"{RED}✗ port 8080 injoignable{RESET}"
    sym_8081 = f"{GREEN}✓ port 8081 OK{RESET}" if up_8081 else f"{YELLOW}⚠ port 8081 injoignable (certains tests sautés){RESET}"
    print(f"  {sym_8080}")
    print(f"  {sym_8081}")

    if not up_8080:
        print(f"\n{RED}{BOLD}Serveur non accessible sur le port 8080.{RESET}")
        print(f"Lance ton webserv d'abord :  {BOLD}./webserv full.cnf{RESET}\n")
        sys.exit(1)

    print(f"\n{DIM}Préparation des fichiers de test…{RESET}")
    setup()

    # Lancement des tests
    test_get()
    test_post()
    test_delete()
    test_redirects()
    test_cgi()
    test_errors()
    test_stress()

    cleanup()

    # Résumé final
    total = _passed + _failed + _errors
    print(f"\n{BOLD}{CYAN}{bar}{RESET}")
    print(f"{BOLD}  RÉSULTATS{RESET}")
    print(f"{BOLD}{CYAN}{bar}{RESET}")
    print(f"  {GREEN}{BOLD}PASS   {_passed:3d}{RESET}")
    print(f"  {RED}{BOLD}FAIL   {_failed:3d}{RESET}")
    print(f"  {YELLOW}{BOLD}ERR    {_errors:3d}{RESET}")
    print(f"  {'─'*14}")
    print(f"  {BOLD}TOTAL  {total:3d}{RESET}")

    if _failed == 0 and _errors == 0:
        print(f"\n  {GREEN}{BOLD}Tous les tests passent ! Félicitations.{RESET}")
    else:
        pct = (_passed * 100 // total) if total else 0
        print(f"\n  {YELLOW}{BOLD}{pct}% de succès — {_failed} échec(s), {_errors} erreur(s){RESET}")

    print(f"{BOLD}{CYAN}{bar}{RESET}\n")
    sys.exit(0 if (_failed == 0 and _errors == 0) else 1)


if __name__ == "__main__":
    main()
