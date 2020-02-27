--TEST--
AMQPConnection - ssl support / authentication using certificates
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
<?php if (true) print "skip - run ssl connection test manually"; ?>
--FILE--
<?php
/*
openssl s_server -accept 8443 \
-cert /home/vagrant/php-amqp/provision/test_certs/server/cert.pem \
-key /home/vagrant/php-amqp/provision/test_certs/server/key.pem \
-CAfile /home/vagrant/php-amqp/provision/test_certs/testca/cacert.pem

openssl s_client -connect localhost:8443 \
-cert /home/vagrant/php-amqp/provision/test_certs/sasl-client/cert.pem \
-key /home/vagrant/php-amqp/provision/test_certs/sasl-client/key.pem \
-CAfile /home/vagrant/php-amqp/provision/test_certs/testca/cacert.pem

# to connect to rabbitmq server manually
openssl s_client -connect localhost:5671 \
-cert /home/vagrant/php-amqp/provision/test_certs/sasl-client/cert.pem \
-key /home/vagrant/php-amqp/provision/test_certs/sasl-client/key.pem \
-CAfile /home/vagrant/php-amqp/provision/test_certs/testca/cacert.pem

Certificate authentication needs the hostname to be as the certificate CN one:
e.g.: server CN = *.example.org
=> hostname must be in same domain
e.g. rabbitmq.example.org
if verify = true
=> you need a /etc/hosts entry, so that the server can resolve the name supplied by the client
127.0.0.1   rabbitmq.example.org rabbitmq
CN in client certificate must also match CN scheme
CN in server is *.example.org
CN in clients used are e.g.:
    client.example.org
    sasl-client.example.org
users have to exist with appropriate rights in rabbitmq
you can validate authentication by looking at /var/log/rabbitmq/rabbit@vagrant-sasl.log
connection <0.1104.0> (127.0.0.1:48898 -> 127.0.0.1:5671): user 'sasl-client.example.org' authenticated and granted access to vhost '/'
*/
$credentials = array(
    'host' => 'rabbitmq.example.org',
    'port' => 5671,

    'verify' => true,

    'cacert' => __DIR__ . "/../provision/test_certs/testca/cacert.pem",
    'cert' => __DIR__ . "/../provision/test_certs/sasl-client/cert.pem",
    'key' => __DIR__ . "/../provision/test_certs/sasl-client/key.pem",
    'sasl_method' => 1
);

$cnn = new AMQPConnection($credentials);
$cnn->setSaslMethod(1);

var_dump($cnn);

$cnn->connect();

echo ($cnn->isConnected() ? 'connected' : 'disconnected'), PHP_EOL;

echo PHP_EOL;

$cnn = new AMQPConnection();
$cnn->setHost('localhost');
$cnn->setPort(5671);
$cnn->setCACert(__DIR__ . "/../provision/test_certs/testca/cacert.pem");
$cnn->setCert(__DIR__ . "/../provision/test_certs/sasl-client/cert.pem");
$cnn->setKey(__DIR__ . "/../provision/test_certs/sasl-client/key.pem");
$cnn->setSaslMethod(1);
$cnn->setVerify(true);
var_dump($cnn);
try {
    $cnn->connect();
} catch(Exception $e) {
    echo get_class($e), "({$e->getCode()}): ", $e->getMessage(), PHP_EOL;
}

echo ($cnn->isConnected() ? 'connected' : 'disconnected'), PHP_EOL;

//sleep(100);

?>
--EXPECTF--
object(AMQPConnection)#1 (18) {
  ["login":"AMQPConnection":private]=>
  string(5) "guest"
  ["password":"AMQPConnection":private]=>
  string(5) "guest"
  ["host":"AMQPConnection":private]=>
  string(20) "rabbitmq.example.org"
  ["vhost":"AMQPConnection":private]=>
  string(1) "/"
  ["port":"AMQPConnection":private]=>
  int(5671)
  ["read_timeout":"AMQPConnection":private]=>
  float(0)
  ["write_timeout":"AMQPConnection":private]=>
  float(0)
  ["connect_timeout":"AMQPConnection":private]=>
  float(0)
  ["rpc_timeout":"AMQPConnection":private]=>
  float(0)
  ["channel_max":"AMQPConnection":private]=>
  int(256)
  ["frame_max":"AMQPConnection":private]=>
  int(131072)
  ["heartbeat":"AMQPConnection":private]=>
  int(0)
  ["cacert":"AMQPConnection":private]=>
  string(%d) "%s/provision/test_certs/testca/cacert.pem"
  ["key":"AMQPConnection":private]=>
  string(%d) "%s/provision/test_certs/sasl-client/key.pem"
  ["cert":"AMQPConnection":private]=>
  string(%d) "%s/provision/test_certs/sasl-client/cert.pem"
  ["verify":"AMQPConnection":private]=>
  bool(true)
  ["sasl_method":"AMQPConnection":private]=>
  int(1)
  ["connection_name":"AMQPConnection":private]=>
  NULL
}
connected

object(AMQPConnection)#2 (18) {
  ["login":"AMQPConnection":private]=>
  string(5) "guest"
  ["password":"AMQPConnection":private]=>
  string(5) "guest"
  ["host":"AMQPConnection":private]=>
  string(9) "localhost"
  ["vhost":"AMQPConnection":private]=>
  string(1) "/"
  ["port":"AMQPConnection":private]=>
  int(5671)
  ["read_timeout":"AMQPConnection":private]=>
  float(0)
  ["write_timeout":"AMQPConnection":private]=>
  float(0)
  ["connect_timeout":"AMQPConnection":private]=>
  float(0)
  ["rpc_timeout":"AMQPConnection":private]=>
  float(0)
  ["channel_max":"AMQPConnection":private]=>
  int(256)
  ["frame_max":"AMQPConnection":private]=>
  int(131072)
  ["heartbeat":"AMQPConnection":private]=>
  int(0)
  ["cacert":"AMQPConnection":private]=>
  string(%d) "%s/provision/test_certs/testca/cacert.pem"
  ["key":"AMQPConnection":private]=>
  string(%d) "%s/provision/test_certs/sasl-client/key.pem"
  ["cert":"AMQPConnection":private]=>
  string(%d) "%s/provision/test_certs/sasl-client/cert.pem"
  ["verify":"AMQPConnection":private]=>
  bool(true)
  ["sasl_method":"AMQPConnection":private]=>
  int(1)
  ["connection_name":"AMQPConnection":private]=>
  NULL
}
AMQPConnectionException(0): Socket error: could not connect to host.
disconnected
