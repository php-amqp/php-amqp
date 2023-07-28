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
  %a
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
