# WebServ

https://www.irif.fr/~sangnier/enseignement/15-16/Reseaux/reseaux-cours5.pdf

https://www.plesk.com/blog/various/nginx-configuration-guide/

https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Status

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