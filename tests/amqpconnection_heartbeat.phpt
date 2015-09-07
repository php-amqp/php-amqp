--TEST--
AMQPConnection heartbeats support
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$heartbeat = 2;
$credentials = array('heartbeat' => $heartbeat);
$cnn = new AMQPConnection($credentials);
$cnn->connect();

debug_zval_dump($cnn);

sleep($heartbeat*10);

try {
$ch = new AMQPChannel($cnn);

} catch (AMQPException $e) {
  echo get_class($e), ': ', $e->getMessage(), PHP_EOL;
}

?>
--EXPECTF--
object(AMQPConnection)#1 (15) refcount(2){
  ["login"]=>
  string(5) "guest" refcount(2)
  ["password"]=>
  string(5) "guest" refcount(2)
  ["host"]=>
  string(9) "localhost" refcount(2)
  ["vhost"]=>
  string(1) "/" refcount(2)
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
  int(256)
  ["max_frame_size"]=>
  int(131072)
  ["heartbeat_interval"]=>
  int(2)
}
AMQPException: Library error: a socket error occurred
