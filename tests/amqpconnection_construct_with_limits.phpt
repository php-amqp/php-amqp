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

var_dump($cnn);
?>
--EXPECTF--
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
  %s(5672)
  ["read_timeout":"AMQPConnection":private]=>
  %s(0)
  ["write_timeout":"AMQPConnection":private]=>
  %s(0)
  ["connect_timeout":"AMQPConnection":private]=>
  %s(0)
  ["channel_max":"AMQPConnection":private]=>
  %s(10)
  ["frame_max":"AMQPConnection":private]=>
  %s(10240)
  ["heartbeat":"AMQPConnection":private]=>
  %s(5)
}