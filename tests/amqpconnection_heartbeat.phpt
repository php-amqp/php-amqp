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
--EXPECT--
object(AMQPConnection)#1 (11) refcount(2){
  ["login":"AMQPConnection":private]=>
  string(5) "guest" refcount(1)
  ["password":"AMQPConnection":private]=>
  string(5) "guest" refcount(1)
  ["host":"AMQPConnection":private]=>
  string(9) "localhost" refcount(1)
  ["vhost":"AMQPConnection":private]=>
  string(1) "/" refcount(1)
  ["port":"AMQPConnection":private]=>
  long(5672) refcount(1)
  ["read_timeout":"AMQPConnection":private]=>
  double(0) refcount(1)
  ["write_timeout":"AMQPConnection":private]=>
  double(0) refcount(1)
  ["connect_timeout":"AMQPConnection":private]=>
  double(0) refcount(1)
  ["channel_max":"AMQPConnection":private]=>
  long(256) refcount(1)
  ["frame_max":"AMQPConnection":private]=>
  long(131072) refcount(1)
  ["heartbeat":"AMQPConnection":private]=>
  long(2) refcount(1)
}
AMQPException: Library error: a socket error occurred