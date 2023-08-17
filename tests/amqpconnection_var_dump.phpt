--TEST--
AMQPConnection var_dump
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip";
if (!getenv("PHP_AMQP_HOST")) print "skip";
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
var_dump($cnn->isConnected());
var_dump($cnn);
$cnn->connect();
var_dump($cnn->isConnected());
$cnn->connect();
var_dump($cnn->isConnected());
var_dump($cnn);

$cnn->disconnect();
var_dump($cnn->isConnected());
var_dump($cnn);
?>
--EXPECTF--
bool(false)
object(AMQPConnection)#1 (18) {
  ["login":"AMQPConnection":private]=>
  string(5) "guest"
  ["password":"AMQPConnection":private]=>
  string(5) "guest"
  ["host":"AMQPConnection":private]=>
  string(%d) "%s"
  ["vhost":"AMQPConnection":private]=>
  string(1) "/"
  ["port":"AMQPConnection":private]=>
  int(5672)
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
  NULL
  ["key":"AMQPConnection":private]=>
  NULL
  ["cert":"AMQPConnection":private]=>
  NULL
  ["verify":"AMQPConnection":private]=>
  bool(true)
  ["saslMethod":"AMQPConnection":private]=>
  int(0)
  ["connectionName":"AMQPConnection":private]=>
  NULL
}
bool(true)
bool(true)
object(AMQPConnection)#1 (18) {
  ["login":"AMQPConnection":private]=>
  string(5) "guest"
  ["password":"AMQPConnection":private]=>
  string(5) "guest"
  ["host":"AMQPConnection":private]=>
  string(%d) "%s"
  ["vhost":"AMQPConnection":private]=>
  string(1) "/"
  ["port":"AMQPConnection":private]=>
  int(5672)
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
  NULL
  ["key":"AMQPConnection":private]=>
  NULL
  ["cert":"AMQPConnection":private]=>
  NULL
  ["verify":"AMQPConnection":private]=>
  bool(true)
  ["saslMethod":"AMQPConnection":private]=>
  int(0)
  ["connectionName":"AMQPConnection":private]=>
  NULL
}
bool(false)
object(AMQPConnection)#1 (18) {
  ["login":"AMQPConnection":private]=>
  string(5) "guest"
  ["password":"AMQPConnection":private]=>
  string(5) "guest"
  ["host":"AMQPConnection":private]=>
  string(%d) "%s"
  ["vhost":"AMQPConnection":private]=>
  string(1) "/"
  ["port":"AMQPConnection":private]=>
  int(5672)
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
  NULL
  ["key":"AMQPConnection":private]=>
  NULL
  ["cert":"AMQPConnection":private]=>
  NULL
  ["verify":"AMQPConnection":private]=>
  bool(true)
  ["saslMethod":"AMQPConnection":private]=>
  int(0)
  ["connectionName":"AMQPConnection":private]=>
  NULL
}
