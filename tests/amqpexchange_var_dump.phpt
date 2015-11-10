--TEST--
AMQPExchange var_dump
--SKIPIF--
<?php
if (!extension_loaded("amqp") || version_compare(PHP_VERSION, '5.3', '<')) {
  print "skip";
}
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();
$ch = new AMQPChannel($cnn);
// Declare a new exchange
$ex = new AMQPExchange($ch);
$ex->setName('exchange-' . microtime(true));
$ex->setType(AMQP_EX_TYPE_FANOUT);
var_dump($ex);
$ex->setArguments(array("x-ha-policy" => "all"));
var_dump($ex);
?>
--EXPECTF--
object(AMQPExchange)#3 (9) {
  ["connection":"AMQPExchange":private]=>
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
    int(5672)
    ["read_timeout":"AMQPConnection":private]=>
    float(0)
    ["write_timeout":"AMQPConnection":private]=>
    float(0)
    ["connect_timeout":"AMQPConnection":private]=>
    float(0)
    ["channel_max":"AMQPConnection":private]=>
    int(256)
    ["frame_max":"AMQPConnection":private]=>
    int(131072)
    ["heartbeat":"AMQPConnection":private]=>
    int(0)
  }
  ["channel":"AMQPExchange":private]=>
  object(AMQPChannel)#2 (3) {
    ["connection":"AMQPChannel":private]=>
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
      int(5672)
      ["read_timeout":"AMQPConnection":private]=>
      float(0)
      ["write_timeout":"AMQPConnection":private]=>
      float(0)
      ["connect_timeout":"AMQPConnection":private]=>
      float(0)
      ["channel_max":"AMQPConnection":private]=>
      int(256)
      ["frame_max":"AMQPConnection":private]=>
      int(131072)
      ["heartbeat":"AMQPConnection":private]=>
      int(0)
    }
    ["prefetch_count":"AMQPChannel":private]=>
    int(3)
    ["prefetch_size":"AMQPChannel":private]=>
    int(0)
  }
  ["name":"AMQPExchange":private]=>
  string(%d) "exchange-%f"
  ["type":"AMQPExchange":private]=>
  string(6) "fanout"
  ["passive":"AMQPExchange":private]=>
  bool(false)
  ["durable":"AMQPExchange":private]=>
  bool(false)
  ["auto_delete":"AMQPExchange":private]=>
  bool(false)
  ["internal":"AMQPExchange":private]=>
  bool(false)
  ["arguments":"AMQPExchange":private]=>
  array(0) {
  }
}
object(AMQPExchange)#3 (9) {
  ["connection":"AMQPExchange":private]=>
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
    int(5672)
    ["read_timeout":"AMQPConnection":private]=>
    float(0)
    ["write_timeout":"AMQPConnection":private]=>
    float(0)
    ["connect_timeout":"AMQPConnection":private]=>
    float(0)
    ["channel_max":"AMQPConnection":private]=>
    int(256)
    ["frame_max":"AMQPConnection":private]=>
    int(131072)
    ["heartbeat":"AMQPConnection":private]=>
    int(0)
  }
  ["channel":"AMQPExchange":private]=>
  object(AMQPChannel)#2 (3) {
    ["connection":"AMQPChannel":private]=>
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
      int(5672)
      ["read_timeout":"AMQPConnection":private]=>
      float(0)
      ["write_timeout":"AMQPConnection":private]=>
      float(0)
      ["connect_timeout":"AMQPConnection":private]=>
      float(0)
      ["channel_max":"AMQPConnection":private]=>
      int(256)
      ["frame_max":"AMQPConnection":private]=>
      int(131072)
      ["heartbeat":"AMQPConnection":private]=>
      int(0)
    }
    ["prefetch_count":"AMQPChannel":private]=>
    int(3)
    ["prefetch_size":"AMQPChannel":private]=>
    int(0)
  }
  ["name":"AMQPExchange":private]=>
  string(%d) "exchange-%f"
  ["type":"AMQPExchange":private]=>
  string(6) "fanout"
  ["passive":"AMQPExchange":private]=>
  bool(false)
  ["durable":"AMQPExchange":private]=>
  bool(false)
  ["auto_delete":"AMQPExchange":private]=>
  bool(false)
  ["internal":"AMQPExchange":private]=>
  bool(false)
  ["arguments":"AMQPExchange":private]=>
  array(1) {
    ["x-ha-policy"]=>
    string(3) "all"
  }
}