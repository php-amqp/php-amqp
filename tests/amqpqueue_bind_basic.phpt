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
$ex->setName('exchange-' . bin2hex(random_bytes(32)));
$ex->setType(AMQP_EX_TYPE_DIRECT);
$ex->declareExchange();

$queue = new AMQPQueue($ch);
$queue->setName("queue-" . bin2hex(random_bytes(32)));
$queue->declareQueue();
var_dump($queue->bind($ex->getName(), 'routing.key'));

$queue->delete();
$ex->delete();
?>
--EXPECT--
NULL
