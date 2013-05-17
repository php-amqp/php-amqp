--TEST--
AMQPExchange
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();

$ch = new AMQPChannel($cnn);

$ex = new AMQPExchange($ch);
$ex->setName("exchange-" . time());
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->declareExchange();
echo $ex->publish('message', 'routing.key') ? 'true' : 'false';
$ex->delete();
?>
--EXPECT--
true
