--TEST--
AMQPQueue var_dump
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
$ex->setName('exchange1');
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->declareExchange();
// Create a new queue
$q = new AMQPQueue($ch);
$q->setName('queue_var_dump');
$q->declareQueue();
var_dump($q);
?>
--EXPECT--
object(AMQPQueue)#4 (9) {
  ["connection":"AMQPQueue":private]=>
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
  ["channel":"AMQPQueue":private]=>
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
  ["name":"AMQPQueue":private]=>
  string(14) "queue_var_dump"
  ["consumer_tag":"AMQPQueue":private]=>
  NULL
  ["passive":"AMQPQueue":private]=>
  bool(false)
  ["durable":"AMQPQueue":private]=>
  bool(false)
  ["exclusive":"AMQPQueue":private]=>
  bool(false)
  ["auto_delete":"AMQPQueue":private]=>
  bool(true)
  ["arguments":"AMQPQueue":private]=>
  array(0) {
  }
}
