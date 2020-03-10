--TEST--
AMQPExchange setFlags()
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
$ex->setArguments(array("x-ha-policy" => "all"));
$ex->setFlags(AMQP_PASSIVE | AMQP_DURABLE | AMQP_AUTODELETE | AMQP_INTERNAL);

var_dump($ex);
?>
--EXPECTF--
object(AMQPExchange)#3 (9) {
  ["connection":"AMQPExchange":private]=>
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
  ["channel":"AMQPExchange":private]=>
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
  ["name":"AMQPExchange":private]=>
  string(%d) "exchange-%f"
  ["type":"AMQPExchange":private]=>
  string(6) "fanout"
  ["passive":"AMQPExchange":private]=>
  bool(true)
  ["durable":"AMQPExchange":private]=>
  bool(true)
  ["auto_delete":"AMQPExchange":private]=>
  bool(true)
  ["internal":"AMQPExchange":private]=>
  bool(true)
  ["arguments":"AMQPExchange":private]=>
  array(1) {
    ["x-ha-policy"]=>
    string(3) "all"
  }
}
