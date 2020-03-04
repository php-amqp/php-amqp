--TEST--
AMQPChannel var_dump
--SKIPIF--
<?php
if (!extension_loaded("amqp") || version_compare(PHP_VERSION, '5.3', '<')) {
  print "skip";
}
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();
$ch = new AMQPChannel($cnn);

var_dump($ch);
$cnn->disconnect();
var_dump($ch);

?>
--EXPECT--
object(AMQPChannel)#2 (6) {
  ["connection":"AMQPChannel":private]=>
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
    ["read_timeout":"AMQPConnection":private]=>
    float(0)
    ["write_timeout":"AMQPConnection":private]=>
    float(0)
    ["connect_timeout":"AMQPConnection":private]=>
    float(0)
    ["rpc_timeout":"AMQPConnection":private]=>
    float(0)
    ["channel_max":"AMQPConnection":private]=>
    int(256)
    ["frame_max":"AMQPConnection":private]=>
    int(131072)
    ["heartbeat":"AMQPConnection":private]=>
    int(0)
    ["cacert":"AMQPConnection":private]=>
    string(0) ""
    ["key":"AMQPConnection":private]=>
    string(0) ""
    ["cert":"AMQPConnection":private]=>
    string(0) ""
    ["verify":"AMQPConnection":private]=>
    bool(true)
    ["sasl_method":"AMQPConnection":private]=>
    int(0)
    ["connection_name":"AMQPConnection":private]=>
    NULL
  }
  ["prefetch_count":"AMQPChannel":private]=>
  int(3)
  ["prefetch_size":"AMQPChannel":private]=>
  int(0)
  ["global_prefetch_count":"AMQPChannel":private]=>
  int(0)
  ["global_prefetch_size":"AMQPChannel":private]=>
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
    string(9) "localhost"
    ["vhost":"AMQPConnection":private]=>
    string(1) "/"
    ["port":"AMQPConnection":private]=>
    int(5672)
    ["read_timeout":"AMQPConnection":private]=>
    float(0)
    ["write_timeout":"AMQPConnection":private]=>
    float(0)
    ["connect_timeout":"AMQPConnection":private]=>
    float(0)
    ["rpc_timeout":"AMQPConnection":private]=>
    float(0)
    ["channel_max":"AMQPConnection":private]=>
    int(256)
    ["frame_max":"AMQPConnection":private]=>
    int(131072)
    ["heartbeat":"AMQPConnection":private]=>
    int(0)
    ["cacert":"AMQPConnection":private]=>
    string(0) ""
    ["key":"AMQPConnection":private]=>
    string(0) ""
    ["cert":"AMQPConnection":private]=>
    string(0) ""
    ["verify":"AMQPConnection":private]=>
    bool(true)
    ["sasl_method":"AMQPConnection":private]=>
    int(0)
    ["connection_name":"AMQPConnection":private]=>
    NULL
  }
  ["prefetch_count":"AMQPChannel":private]=>
  int(3)
  ["prefetch_size":"AMQPChannel":private]=>
  int(0)
  ["global_prefetch_count":"AMQPChannel":private]=>
  int(0)
  ["global_prefetch_size":"AMQPChannel":private]=>
  int(0)
  ["consumers":"AMQPChannel":private]=>
  array(0) {
  }
}
