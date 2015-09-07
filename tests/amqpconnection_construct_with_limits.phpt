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
  bool(false)
  ["connection_resource"]=>
  resource(4) of type (AMQP Connection Resource) refcount(3)
  ["used_channels"]=>
  int(0)
  ["max_channel_id"]=>
  int(10)
  ["max_frame_size"]=>
  int(10240)
  ["heartbeat_interval"]=>
  int(5)
}