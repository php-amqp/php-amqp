--TEST--
AMQPConnection constructor with channel_max, frame_max and heartbeat limits
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$credentials = array(
    'channel_max' => 10,
    'frame_max' => 10240,
    'heartbeat' => 5,
);
$cnn = new AMQPConnection($credentials);
$cnn->connect();

debug_zval_dump($cnn);
?>
--EXPECTF--
object(AMQPConnection)#1 (15) refcount(2){
  ["login"]=>
  string(5) "guest" refcount(1)
  ["password"]=>
  string(5) "guest" refcount(1)
  ["host"]=>
  string(9) "localhost" refcount(1)
  ["vhost"]=>
  string(1) "/" refcount(1)
  ["port"]=>
  long(5672) refcount(1)
  ["read_timeout"]=>
  double(0) refcount(1)
  ["write_timeout"]=>
  double(0) refcount(1)
  ["connect_timeout"]=>
  double(0) refcount(1)
  ["is_connected"]=>
  bool(true) refcount(1)
  ["is_persistent"]=>
  bool(false) refcount(1)
  ["connection_resource"]=>
  resource(4) of type (AMQP Connection Resource) refcount(1)
  ["used_channels"]=>
  long(0) refcount(1)
  ["max_channel_id"]=>
  long(10) refcount(1)
  ["max_frame_size"]=>
  long(10240) refcount(1)
  ["heartbeat_interval"]=>
  long(5) refcount(1)
}