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
  long(256) refcount(1)
  ["max_frame_size"]=>
  long(131072) refcount(1)
  ["heartbeat_interval"]=>
  long(2) refcount(1)
}
AMQPException: Library error: a socket error occurred
