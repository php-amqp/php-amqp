--TEST--
AMQPQueue
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();

$ch = new AMQPChannel($cnn);

$ex = new AMQPExchange($ch);
$ex->setName('exchange-' . time());
$ex->setType(AMQP_EX_TYPE_DIRECT);
$ex->declare();

$queue = new AMQPQueue($ch);
$queue->setName("queue-" . time());
$queue->declare();
var_dump($queue->bind($ex->getName()));
var_dump($queue->unbind($ex->getName()));

$queue->delete();
$ex->delete();
?>
--EXPECT--
bool(true)
bool(true)
