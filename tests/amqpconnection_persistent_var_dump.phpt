--TEST--
AMQPConnection: persistent connection var_dump
--SKIPIF--
<?php if (!extension_loaded("amqp") || version_compare(PHP_VERSION, '5.3', '<') ) print "skip"; ?>
--FILE--
<?php

$cnn = new AMQPConnection();
var_dump($cnn);
$cnn->pconnect();
var_dump($cnn);

$ch = new AMQPChannel($cnn);
var_dump($cnn);

$cnn->pdisconnect();
var_dump($cnn);

?>
--EXPECT--
object(AMQPConnection)#1 (13) {
  ["login"]=>
  string(5) "guest"
  ["password"]=>
  string(5) "guest"
  ["host"]=>
  string(9) "localhost"
  ["vhost"]=>
  string(1) "/"
  ["port"]=>
  int(5672)
  ["read_timeout"]=>
  float(0)
  ["write_timeout"]=>
  float(0)
  ["connect_timeout"]=>
  float(0)
  ["is_connected"]=>
  bool(false)
  ["is_persistent"]=>
  bool(false)
  ["connection_resource"]=>
  NULL
  ["used_channels"]=>
  NULL
  ["max_channel_id"]=>
  NULL
}
object(AMQPConnection)#1 (13) {
  ["login"]=>
  string(5) "guest"
  ["password"]=>
  string(5) "guest"
  ["host"]=>
  string(9) "localhost"
  ["vhost"]=>
  string(1) "/"
  ["port"]=>
  int(5672)
  ["read_timeout"]=>
  float(0)
  ["write_timeout"]=>
  float(0)
  ["connect_timeout"]=>
  float(0)
  ["is_connected"]=>
  bool(true)
  ["is_persistent"]=>
  bool(true)
  ["connection_resource"]=>
  resource(4) of type (AMQP Connection Resource)
  ["used_channels"]=>
  int(0)
  ["max_channel_id"]=>
  int(256)
}
object(AMQPConnection)#1 (13) {
  ["login"]=>
  string(5) "guest"
  ["password"]=>
  string(5) "guest"
  ["host"]=>
  string(9) "localhost"
  ["vhost"]=>
  string(1) "/"
  ["port"]=>
  int(5672)
  ["read_timeout"]=>
  float(0)
  ["write_timeout"]=>
  float(0)
  ["connect_timeout"]=>
  float(0)
  ["is_connected"]=>
  bool(true)
  ["is_persistent"]=>
  bool(true)
  ["connection_resource"]=>
  resource(4) of type (AMQP Connection Resource)
  ["used_channels"]=>
  int(1)
  ["max_channel_id"]=>
  int(256)
}
object(AMQPConnection)#1 (13) {
  ["login"]=>
  string(5) "guest"
  ["password"]=>
  string(5) "guest"
  ["host"]=>
  string(9) "localhost"
  ["vhost"]=>
  string(1) "/"
  ["port"]=>
  int(5672)
  ["read_timeout"]=>
  float(0)
  ["write_timeout"]=>
  float(0)
  ["connect_timeout"]=>
  float(0)
  ["is_connected"]=>
  bool(false)
  ["is_persistent"]=>
  bool(false)
  ["connection_resource"]=>
  NULL
  ["used_channels"]=>
  NULL
  ["max_channel_id"]=>
  NULL
}