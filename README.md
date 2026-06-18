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
