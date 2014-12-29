--TEST--
AMQPQueue declared with empty name
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();
$ch = new AMQPChannel($cnn);

$ex = new AMQPExchange($ch);
$ex->setName('exchange-' . microtime(true));
$ex->setType(AMQP_EX_TYPE_DIRECT);
$ex->declareExchange();

$queue = new AMQPQueue($ch);
$queue->declareQueue();
var_dump(substr($queue->getName(), 0, strlen('amq.gen-')));

$queue->delete();
$ex->delete();
?>
--EXPECT--
string(8) "amq.gen-"
