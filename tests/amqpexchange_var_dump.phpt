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
object(AMQPExchange)#3 (7) {
  ["name"]=>
  string(%d) "exchange-%f"
  ["type"]=>
  string(6) "fanout"
  ["passive"]=>
  bool(false)
  ["durable"]=>
  bool(false)
  ["auto_delete"]=>
  bool(false)
  ["internal"]=>
  bool(false)
  ["arguments"]=>
  array(0) {
  }
}
object(AMQPExchange)#3 (7) {
  ["name"]=>
  string(%d) "exchange-%f"
  ["type"]=>
  string(6) "fanout"
  ["passive"]=>
  bool(false)
  ["durable"]=>
  bool(false)
  ["auto_delete"]=>
  bool(false)
  ["internal"]=>
  bool(false)
  ["arguments"]=>
  array(1) {
    ["x-ha-policy"]=>
    string(3) "all"
  }
}