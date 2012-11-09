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
$ex->setName('exchange1');
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->setArguments(array("x-ha-policy" => "all"));
var_dump($ex);
?>
--EXPECT--
object(AMQPExchange)#3 (5) {
  ["name"]=>
  string(9) "exchange1"
  ["type"]=>
  string(6) "fanout"
  ["passive"]=>
  int(0)
  ["durable"]=>
  int(0)
  ["arguments"]=>
  array(1) {
    ["x-ha-policy"]=>
    string(3) "all"
  }
}
