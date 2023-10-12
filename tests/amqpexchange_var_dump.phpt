--TEST--
AMQPExchange var_dump
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
// Declare a new exchange
$ex = new AMQPExchange($ch);
$ex->setName('exchange-' . bin2hex(random_bytes(32)));
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
  string(%d) "exchange-%s"
  ["type":"AMQPExchange":private]=>
  string(6) "fanout"
  ["passive":"AMQPExchange":private]=>
  bool(false)
  ["durable":"AMQPExchange":private]=>
  bool(false)
  ["autoDelete":"AMQPExchange":private]=>
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
  string(%d) "exchange-%s"
  ["type":"AMQPExchange":private]=>
  string(6) "fanout"
  ["passive":"AMQPExchange":private]=>
  bool(false)
  ["durable":"AMQPExchange":private]=>
  bool(false)
  ["autoDelete":"AMQPExchange":private]=>
  bool(false)
  ["internal":"AMQPExchange":private]=>
  bool(false)
  ["arguments":"AMQPExchange":private]=>
  array(1) {
    ["x-ha-policy"]=>
    string(3) "all"
  }
}
