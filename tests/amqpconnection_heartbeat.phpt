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

var_dump($cnn);

sleep($heartbeat*10);

try {
$ch = new AMQPChannel($cnn);

} catch (AMQPException $e) {
  echo get_class($e), ': ', $e->getMessage(), PHP_EOL;
}

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
  %s(256)
  ["frame_max":"AMQPConnection":private]=>
  %s(131072)
  ["heartbeat":"AMQPConnection":private]=>
  %s(2)
}
AMQPException: Library error: a socket error occurred