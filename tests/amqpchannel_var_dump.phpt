--TEST--
AMQPChannel var_dump
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
elseif (!getenv("PHP_AMQP_HOST")) print "skip PHP_AMQP_HOST environment variable is not set";
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->connect();
$ch = new AMQPChannel($cnn);

var_dump($ch);
$cnn->disconnect();
var_dump($ch);

?>
--EXPECTF--
object(AMQPChannel)#2 (6) {
  ["connection":"AMQPChannel":private]=>
  object(AMQPConnection)#1 (18) {
    ["login":"AMQPConnection":private]=>
    string(5) "guest"
    ["password":"AMQPConnection":private]=>
    string(5) "guest"
    ["host":"AMQPConnection":private]=>
    string(%d) "%s"
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
    int(131072)
    ["channelMax":"AMQPConnection":private]=>
    int(256)
    ["heartbeat":"AMQPConnection":private]=>
    int(0)
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
  ["prefetchCount":"AMQPChannel":private]=>
  int(3)
  ["prefetchSize":"AMQPChannel":private]=>
  int(0)
  ["globalPrefetchCount":"AMQPChannel":private]=>
  int(0)
  ["globalPrefetchSize":"AMQPChannel":private]=>
  int(0)
  ["consumers":"AMQPChannel":private]=>
  array(0) {
  }
}
object(AMQPChannel)#2 (6) {
  ["connection":"AMQPChannel":private]=>
  object(AMQPConnection)#1 (18) {
    ["login":"AMQPConnection":private]=>
    string(5) "guest"
    ["password":"AMQPConnection":private]=>
    string(5) "guest"
    ["host":"AMQPConnection":private]=>
    string(%d) "%s"
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
    int(131072)
    ["channelMax":"AMQPConnection":private]=>
    int(256)
    ["heartbeat":"AMQPConnection":private]=>
    int(0)
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
  ["prefetchCount":"AMQPChannel":private]=>
  int(3)
  ["prefetchSize":"AMQPChannel":private]=>
  int(0)
  ["globalPrefetchCount":"AMQPChannel":private]=>
  int(0)
  ["globalPrefetchSize":"AMQPChannel":private]=>
  int(0)
  ["consumers":"AMQPChannel":private]=>
  array(0) {
  }
}
