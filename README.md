*This project has been created as part of the 42 curriculum by cbezenco, strieste.*

# Webserv

## Description

An HTTP/1.1 server written in C++98, inspired by NGINX. The server handles multiple clients simultaneously using a single non-blocking `poll()` event loop. It supports static file serving, file uploads, HTTP redirections, directory listing, virtual hosts, and CGI execution for PHP and Python scripts.

## Features

- Non-blocking I/O with a single `poll()` loop (all sockets, pipes, and CGI fds)
- HTTP methods: `GET`, `POST`, `DELETE`
- Static website serving
- File upload (raw body and multipart/form-data)
- File deletion
- Directory listing (`autoindex`)
- HTTP redirections (301)
- Default error pages (400, 403, 404, 405, 409, 500…)
- CGI support: `.php` (php-cgi) and `.py` (python3)
- Virtual hosts (multiple `server {}` blocks, multiple ports)
- Configurable `client_max_body_size`
- Compatible with standard web browsers and `curl`

## Instructions

### Compilation

```bash
make
```

Requires a C++98-compliant compiler (`g++` or `clang++`) with flags `-Wall -Wextra -Werror`.

### Run

```bash
./webserv [configuration_file]
```

If no config file is given, the server looks for a default config path.

Examples:

```bash
./webserv default.cnf
./webserv full.cnf
```

### Configuration file

Inspired by NGINX `server {}` blocks. Example (`full.cnf`):

```nginx
server {
    listen 127.0.0.1:8080;
    server_name localhost;
    root ./var/www/html;
    client_max_body_size 1048576;

    error_page 404 ./var/www/errors/404.html;

    location / {
        index index.html;
        methods GET;
        autoindex off;
    }

    location /uploads {
        methods GET POST;
        root ./var/www/;
        upload_path ./var/www/uploads;
    }

    location /files {
        methods GET DELETE;
        autoindex on;
    }

    location /cgi-bin {
        root ./var/www;
        methods GET POST;
        cgi .py /usr/bin/python3;
        cgi .php /usr/bin/php-cgi;
    }
}
```

**Directives:**

| Directive | Description |
|-----------|-------------|
| `listen` | `host:port` to bind |
| `server_name` | Virtual host name |
| `root` | Document root |
| `client_max_body_size` | Max request body size in bytes |
| `error_page` | Custom error pages |
| `location` | Route block |
| `methods` | Allowed HTTP methods for the route |
| `index` | Default file for directory requests |
| `autoindex` | Enable/disable directory listing |
| `redirect` | HTTP redirection (e.g. `301 /new`) |
| `upload_path` | Directory where uploaded files are stored |
| `cgi` | CGI handler by extension (e.g. `.php /usr/bin/php-cgi`) |

### Usage examples

```bash
# GET index
curl http://localhost:8080

# GET static file
curl http://localhost:8080/index.html

# Upload file (raw)
curl -X POST -H "Content-Type: application/octet-stream" \
     --data-binary @test.txt http://localhost:8080/uploads

# Upload file (multipart)
curl -F "file=@test.txt" http://localhost:8080/uploads

# Delete a file
curl -X DELETE http://localhost:8080/files/index.html

# Unknown/unsupported method — expect 405
curl -X PUT http://localhost:8080/uploads

# Virtual host
curl --resolve test.local:8081:127.0.0.1 http://test.local:8081/
```

### CGI requirements

- PHP: `php-cgi` must be installed (`/usr/bin/php-cgi` or `/opt/homebrew/bin/php-cgi` on macOS)
- Python: `python3` must be installed (`/usr/bin/python3`)
- CGI scripts go in `./var/www/cgi-bin/`

## Resources

- [HTTP/1.1 RFC 7230–7235](https://datatracker.ietf.org/doc/html/rfc7230)
- [NGINX configuration guide](https://www.plesk.com/blog/various/nginx-configuration-guide/)
- [MDN HTTP status codes](https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Status)
- [HTTP course slides (FR)](https://www.irif.fr/~sangnier/enseignement/15-16/Reseaux/reseaux-cours5.pdf)

### AI usage

AI was used for:
- Understanding How HTTP Requests Work.
- Help understanding what a CGI is and how it actually works.
- Help with generating CGI scripts for testing.

---

<!-- Original notes -->

# WebServ


- upload a file (raw data) :
    curl -X POST -H "Content-Type: application/octet-stream" --data-binary @test.txt http://localhost:8080/uploads

- upload a file (multipart) :
    curl -F "file=@test.txt" http://localhost:8080/uploads

- delete a file (dont forget to add/upload a file before) :
    curl -X DELETE http://localhost:8080/files/index.html

- Get / :
    curl http://localhost:8080

- Get static file :
    curl http://localhost:8080/index.html

- Unknown method / Method not implemented :
    curl -X PUT http://localhost:8080/uploads ; should send error 405
    curl -X BLABLA http://localhost:8080 ; should send error 400

- Virtual Host
curl --resolve test.local:8080:127.0.0.1 http://test.local:8080/ -v

curl -X POST -F "file=@Makefile" http://127.0.0.1:8080/uploads -v

siege -c 10 -r 20 "http://127.0.0.1:8080/uploads POST < 404.html" == 415 normal


curl -v -X POST http://127.0.0.1:8080/uploads \ -H "Content-Type: text/plain" \ --data-binary "Hello WebServ!"