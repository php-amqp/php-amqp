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
--EXPECT--
object(AMQPConnection)#1 (18) {
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
  ["readTimeout":"AMQPConnection":private]=>
  float(0)
  ["writeTimeout":"AMQPConnection":private]=>
  float(0)
  ["connectTimeout":"AMQPConnection":private]=>
  float(0)
  ["rpcTimeout":"AMQPConnection":private]=>
  float(0)
  ["frameMax":"AMQPConnection":private]=>
  int(10240)
  ["channelMax":"AMQPConnection":private]=>
  int(10)
  ["heartbeat":"AMQPConnection":private]=>
  int(5)
  ["cacert":"AMQPConnection":private]=>
  NULL
  ["key":"AMQPConnection":private]=>
  NULL
  ["cert":"AMQPConnection":private]=>
  NULL
  ["verify":"AMQPConnection":private]=>
  bool(true)
  ["saslMethod":"AMQPConnection":private]=>
  int(0)
  ["connectionName":"AMQPConnection":private]=>
  NULL
}
