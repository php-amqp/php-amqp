--TEST--
AMQPConnection - ssl support
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
-cert /home/vagrant/php-amqp/provision/test_certs/client/cert.pem \
-key /home/vagrant/php-amqp/provision/test_certs/client/key.pem \
-CAfile /home/vagrant/php-amqp/provision/test_certs/testca/cacert.pem

# to connect to rabbitmq server manually
openssl s_client -connect localhost:5671 \
-cert /home/vagrant/php-amqp/provision/test_certs/client/cert.pem \
-key /home/vagrant/php-amqp/provision/test_certs/client/key.pem \
-CAfile /home/vagrant/php-amqp/provision/test_certs/testca/cacert.pem

*/
$credentials = array(
    //'host' => gethostname(), //'vagrant-ubuntu-wily-64',
    'port' => 5671,

    'verify' => false,

    'cacert' => __DIR__ . "/../provision/test_certs/testca/cacert.pem",
    'cert' => __DIR__ . "/../provision/test_certs/client/cert.pem",
    'key' => __DIR__ . "/../provision/test_certs/client/key.pem",
);

$cnn = new AMQPConnection($credentials);

var_dump($cnn);

$cnn->connect();

echo ($cnn->isConnected() ? 'connected' : 'disconnected'), PHP_EOL;

echo PHP_EOL;

$cnn = new AMQPConnection();
$cnn->setPort(5671);
$cnn->setCACert(__DIR__ . "/../provision/test_certs/testca/cacert.pem");
$cnn->setCert(__DIR__ . "/../provision/test_certs/client/cert.pem");
$cnn->setKey(__DIR__ . "/../provision/test_certs/client/key.pem");
$cnn->setVerify(false);
var_dump($cnn);

$cnn->connect();

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
  string(%d) "%s/provision/test_certs/client/key.pem"
  ["cert":"AMQPConnection":private]=>
  string(%d) "%s/provision/test_certs/client/cert.pem"
  ["verify":"AMQPConnection":private]=>
  bool(false)
  ["sasl_method":"AMQPConnection":private]=>
  int(0)
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
  string(%d) "%s/provision/test_certs/client/key.pem"
  ["cert":"AMQPConnection":private]=>
  string(%d) "%s/provision/test_certs/client/cert.pem"
  ["verify":"AMQPConnection":private]=>
  bool(false)
  ["sasl_method":"AMQPConnection":private]=>
  int(0)
  ["connection_name":"AMQPConnection":private]=>
  NULL
}
connected
