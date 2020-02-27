--TEST--
AMQPConnection var_dump
--SKIPIF--
<?php
if (!extension_loaded("amqp") || version_compare(PHP_VERSION, '5.3', '<')) {
  print "skip";
}
?>
--FILE--
<?php
$cnn = new AMQPConnection();
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
--EXPECT--
bool(false)
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
  int(5672)
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
  string(0) ""
  ["key":"AMQPConnection":private]=>
  string(0) ""
  ["cert":"AMQPConnection":private]=>
  string(0) ""
  ["verify":"AMQPConnection":private]=>
  bool(true)
  ["sasl_method":"AMQPConnection":private]=>
  int(0)
  ["connection_name":"AMQPConnection":private]=>
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
  string(9) "localhost"
  ["vhost":"AMQPConnection":private]=>
  string(1) "/"
  ["port":"AMQPConnection":private]=>
  int(5672)
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
  string(0) ""
  ["key":"AMQPConnection":private]=>
  string(0) ""
  ["cert":"AMQPConnection":private]=>
  string(0) ""
  ["verify":"AMQPConnection":private]=>
  bool(true)
  ["sasl_method":"AMQPConnection":private]=>
  int(0)
  ["connection_name":"AMQPConnection":private]=>
  NULL
}
bool(false)
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
  int(5672)
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
  string(0) ""
  ["key":"AMQPConnection":private]=>
  string(0) ""
  ["cert":"AMQPConnection":private]=>
  string(0) ""
  ["verify":"AMQPConnection":private]=>
  bool(true)
  ["sasl_method":"AMQPConnection":private]=>
  int(0)
  ["connection_name":"AMQPConnection":private]=>
  NULL
}
