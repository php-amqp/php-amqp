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
  %a
  ["channel":"AMQPExchange":private]=>
  %a
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
  %a
  ["channel":"AMQPExchange":private]=>
  %a
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
