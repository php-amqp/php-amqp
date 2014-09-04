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
$ex->setName('exchange-' . microtime(true));
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->declareExchange();

$queue = new AMQPQueue($ch);
$queue->setName("queue-" . microtime(true));
$queue->declareQueue();
$queue->bind($ex->getName());

var_dump($queue->delete());
var_dump($queue->delete());

$queue->declareQueue();
$queue->bind($ex->getName());

$ex->publish('test 1');
$ex->publish('test 2');
$ex->publish('test 3');

var_dump($queue->delete());

$ex->publish('test 1');
$ex->publish('test 2');
$ex->publish('test 3');

var_dump($queue->delete());

$ex->delete();
?>
--EXPECT--
int(0)
int(0)
int(3)
int(0)
