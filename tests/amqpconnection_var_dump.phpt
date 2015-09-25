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
var_dump($cnn);
$cnn->connect();
$cnn->connect();
var_dump($cnn);

$cnn->disconnect();
var_dump($cnn);
?>
--EXPECT--
object(AMQPConnection)#1 (11) {
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
  ["channel_max":"AMQPConnection":private]=>
  int(256)
  ["frame_max":"AMQPConnection":private]=>
  int(131072)
  ["heartbeat":"AMQPConnection":private]=>
  int(0)
}
object(AMQPConnection)#1 (11) {
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
  ["channel_max":"AMQPConnection":private]=>
  int(256)
  ["frame_max":"AMQPConnection":private]=>
  int(131072)
  ["heartbeat":"AMQPConnection":private]=>
  int(0)
}
object(AMQPConnection)#1 (11) {
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
  ["channel_max":"AMQPConnection":private]=>
  int(256)
  ["frame_max":"AMQPConnection":private]=>
  int(131072)
  ["heartbeat":"AMQPConnection":private]=>
  int(0)
}