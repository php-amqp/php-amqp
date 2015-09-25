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
  long(10) refcount(1)
  ["frame_max":"AMQPConnection":private]=>
  long(10240) refcount(1)
  ["heartbeat":"AMQPConnection":private]=>
  long(5) refcount(1)
}