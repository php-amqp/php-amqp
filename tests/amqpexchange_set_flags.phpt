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
  object(AMQPConnection)#1 (15) {
    ["login"]=>
    string(5) "guest"
    ["password"]=>
    string(5) "guest"
    ["host"]=>
    string(9) "localhost"
    ["vhost"]=>
    string(1) "/"
    ["port"]=>
    int(5672)
    ["read_timeout"]=>
    float(0)
    ["write_timeout"]=>
    float(0)
    ["connect_timeout"]=>
    float(0)
    ["is_connected"]=>
    bool(true)
    ["is_persistent"]=>
    bool(false)
    ["connection_resource"]=>
    resource(4) of type (AMQP Connection Resource)
    ["used_channels"]=>
    int(1)
    ["max_channel_id"]=>
    int(256)
    ["max_frame_size"]=>
    int(131072)
    ["heartbeat_interval"]=>
    int(0)
  }
  ["channel":"AMQPExchange":private]=>
  object(AMQPChannel)#2 (4) {
    ["channel_id"]=>
    int(1)
    ["prefetch_count"]=>
    int(3)
    ["prefetch_size"]=>
    int(0)
    ["is_connected"]=>
    bool(true)
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