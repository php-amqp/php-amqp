--TEST--
AMQPExchange::setArgument() test
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$credentials = array();

$cnn = new AMQPConnection($credentials);
$cnn->connect();

$ch = new AMQPChannel($cnn);

$heartbeat   = 10;
$e_name_ae   = 'test.exchange.ae.' . bin2hex(random_bytes(32));
$e_name = 'test.exchange.' . bin2hex(random_bytes(32));

$ex_ae = new AMQPExchange($ch);
$ex_ae->setName($e_name_ae);
$ex_ae->setFlags(AMQP_AUTODELETE);
$ex_ae->setType(AMQP_EX_TYPE_FANOUT);
$ex_ae->declareExchange();

var_dump($ex_ae);


$ex = new AMQPExchange($ch);
$ex->setName($e_name);
$ex->setFlags(AMQP_AUTODELETE);
$ex->setType(AMQP_EX_TYPE_FANOUT);
// some real keys
$ex->setArgument("x-ha-policy", "all");
$ex->setArgument("alternate-exchange", $e_name_ae);
// some custom keys to test various cases
$ex->setArgument('x-empty-string', '');
$ex->setArgument('x-alternate-exchange-one-more-time', $e_name_ae);
$ex->setArgument('x-numeric-argument', $heartbeat * 10 * 1000);
$ex->declareExchange();

var_dump($ex);
$ex->setArgument('x-alternate-exchange-one-more-time', null); // remov key
var_dump($ex);
?>
--EXPECTF--
object(AMQPExchange)#3 (9) {
  ["connection":"AMQPExchange":private]=>
  %a
  ["channel":"AMQPExchange":private]=>
  %a
  ["name":"AMQPExchange":private]=>
  string(%d) "test.exchange.ae.%s"
  ["type":"AMQPExchange":private]=>
  string(6) "fanout"
  ["passive":"AMQPExchange":private]=>
  bool(false)
  ["durable":"AMQPExchange":private]=>
  bool(false)
  ["autoDelete":"AMQPExchange":private]=>
  bool(true)
  ["internal":"AMQPExchange":private]=>
  bool(false)
  ["arguments":"AMQPExchange":private]=>
  array(0) {
  }
}
object(AMQPExchange)#4 (9) {
  ["connection":"AMQPExchange":private]=>
  %a
  ["channel":"AMQPExchange":private]=>
  %a
  ["name":"AMQPExchange":private]=>
  string(%d) "test.exchange.%s"
  ["type":"AMQPExchange":private]=>
  string(6) "fanout"
  ["passive":"AMQPExchange":private]=>
  bool(false)
  ["durable":"AMQPExchange":private]=>
  bool(false)
  ["autoDelete":"AMQPExchange":private]=>
  bool(true)
  ["internal":"AMQPExchange":private]=>
  bool(false)
  ["arguments":"AMQPExchange":private]=>
  array(5) {
    ["x-ha-policy"]=>
    string(3) "all"
    ["alternate-exchange"]=>
    string(%d) "test.exchange.ae.%s"
    ["x-empty-string"]=>
    string(0) ""
    ["x-alternate-exchange-one-more-time"]=>
    string(%d) "test.exchange.ae.%s"
    ["x-numeric-argument"]=>
    int(100000)
  }
}
object(AMQPExchange)#4 (9) {
  ["connection":"AMQPExchange":private]=>
  %a
  ["channel":"AMQPExchange":private]=>
  %a
  ["name":"AMQPExchange":private]=>
  string(%d) "test.exchange.%s"
  ["type":"AMQPExchange":private]=>
  string(6) "fanout"
  ["passive":"AMQPExchange":private]=>
  bool(false)
  ["durable":"AMQPExchange":private]=>
  bool(false)
  ["autoDelete":"AMQPExchange":private]=>
  bool(true)
  ["internal":"AMQPExchange":private]=>
  bool(false)
  ["arguments":"AMQPExchange":private]=>
  array(4) {
    ["x-ha-policy"]=>
    string(3) "all"
    ["alternate-exchange"]=>
    string(%d) "test.exchange.ae.%s"
    ["x-empty-string"]=>
    string(0) ""
    ["x-numeric-argument"]=>
    int(100000)
  }
}
