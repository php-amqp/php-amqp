--TEST--
AMQPConnection - TLS - SASL authentication
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$credentials = array(
    'host' => 'rabbitmq.example.org',
    'port' => 5671,
    'cacert' => __DIR__ . "/../infra/tls/certificates/testca/cacert.pem",
    'cert' => __DIR__ . "/../infra/tls/certificates/sasl-client/cert.pem",
    'key' => __DIR__ . "/../infra/tls/certificates/sasl-client/key.pem",
    'sasl_method' => AMQP_SASL_METHOD_EXTERNAL,
);

$cnn = new AMQPConnection($credentials);
$cnn->setSaslMethod(1);

var_dump($cnn);

$cnn->connect();

echo ($cnn->isConnected() ? 'connected' : 'disconnected'), PHP_EOL;

echo PHP_EOL;

$cnn = new AMQPConnection();
$cnn->setHost('rabbitmq.example.org');
$cnn->setPort(5671);
$cnn->setCACert(__DIR__ . "/../infra/tls/certificates/testca/cacert.pem");
$cnn->setCert(__DIR__ . "/../infra/tls/certificates/sasl-client/cert.pem");
$cnn->setKey(__DIR__ . "/../infra/tls/certificates/sasl-client/key.pem");
$cnn->setSaslMethod(AMQP_SASL_METHOD_EXTERNAL);

var_dump($cnn);

$cnn->connect();

echo ($cnn->isConnected() ? 'connected' : 'disconnected'), PHP_EOL;
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
  ["readTimeout":"AMQPConnection":private]=>
  float(0)
  ["writeTimeout":"AMQPConnection":private]=>
  float(0)
  ["connectTimeout":"AMQPConnection":private]=>
  float(0)
  ["rpcTimeout":"AMQPConnection":private]=>
  float(0)
  ["frameMax":"AMQPConnection":private]=>
  int(131072)
  ["channelMax":"AMQPConnection":private]=>
  int(256)
  ["heartbeat":"AMQPConnection":private]=>
  int(0)
  ["cacert":"AMQPConnection":private]=>
  string(%d) "%s/testca/cacert.pem"
  ["key":"AMQPConnection":private]=>
  string(%d) "%s/sasl-client/key.pem"
  ["cert":"AMQPConnection":private]=>
  string(%d) "%s/sasl-client/cert.pem"
  ["verify":"AMQPConnection":private]=>
  bool(true)
  ["saslMethod":"AMQPConnection":private]=>
  int(1)
  ["connectionName":"AMQPConnection":private]=>
  NULL
}
connected

object(AMQPConnection)#2 (18) {
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
  ["readTimeout":"AMQPConnection":private]=>
  float(0)
  ["writeTimeout":"AMQPConnection":private]=>
  float(0)
  ["connectTimeout":"AMQPConnection":private]=>
  float(0)
  ["rpcTimeout":"AMQPConnection":private]=>
  float(0)
  ["frameMax":"AMQPConnection":private]=>
  int(131072)
  ["channelMax":"AMQPConnection":private]=>
  int(256)
  ["heartbeat":"AMQPConnection":private]=>
  int(0)
  ["cacert":"AMQPConnection":private]=>
  string(%d) "%s/testca/cacert.pem"
  ["key":"AMQPConnection":private]=>
  string(%d) "%s/sasl-client/key.pem"
  ["cert":"AMQPConnection":private]=>
  string(%d) "%s/sasl-client/cert.pem"
  ["verify":"AMQPConnection":private]=>
  bool(true)
  ["saslMethod":"AMQPConnection":private]=>
  int(1)
  ["connectionName":"AMQPConnection":private]=>
  NULL
}
connected
