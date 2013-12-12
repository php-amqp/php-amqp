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
$ex->setType(AMQP_EX_TYPE_HEADERS);
$ex->declareExchange();

$queue = new AMQPQueue($ch);
$queue->setName("queue-" . time());
$queue->declareQueue();

$arguments = ['x-match' => 'all', 'type' => 'custom'];
var_dump($queue->bind($ex->getName(), '', $arguments));

$queue->delete();
$ex->delete();
?>
--EXPECT--
bool(true)
