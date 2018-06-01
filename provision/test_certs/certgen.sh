mkdir testca; cd testca; mkdir private
openssl req -new -x509 -keyout private/cakey.pem -out cacert.pem -days 365 -nodes -subj "/C=PT/ST=Lisboa/L=Lisboa/O=Example Org"

cd ..
mkdir server;cd server
openssl genrsa -out key.pem 2048
openssl req -new -key key.pem -out req.pem  -subj "/C=PT/ST=Lisboa/L=Lisboa/O=Example Org/CN=*.example.org"
openssl x509 -req -in req.pem -CA ../testca/cacert.pem -CAkey ../testca/private/cakey.pem -CAcreateserial -out cert.pem -days 365

cd ..
mkdir sasl-client; cd sasl-client
openssl genrsa -out key.pem 2048
openssl req -new -key key.pem -out req.pem  -subj "/C=PT/ST=Lisboa/L=Lisboa/O=Example Org/CN=sasl-client.example.org"
openssl x509 -req -in req.pem -CA ../testca/cacert.pem -CAkey ../testca/private/cakey.pem -CAcreateserial -out cert.pem -days 365

cd ..
mkdir client; cd client
openssl genrsa -out key.pem 2048
openssl req -new -key key.pem -out req.pem  -subj "/C=PT/ST=Lisboa/L=Lisboa/O=Example Org/CN=client.example.org"
openssl x509 -req -in req.pem -CA ../testca/cacert.pem -CAkey ../testca/private/cakey.pem -CAcreateserial -out cert.pem -days 365