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
$ex->setType(AMQP_EX_TYPE_DIRECT);
$ex->declareExchange();

$queue = new AMQPQueue($ch);
$queue->setName("queue-" . microtime(true));
$queue->declareQueue();

$arguments = array('x-match' => 'all', 'type' => 'custom');

var_dump($queue->bind($ex->getName(), '', $arguments));
var_dump($queue->unbind($ex->getName(), '', $arguments));

$queue->delete();
$ex->delete();
?>
--EXPECT--
bool(true)
bool(true)
