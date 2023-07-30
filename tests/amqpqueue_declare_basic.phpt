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
var_dump($queue->setName("queue-" . bin2hex(random_bytes(32))));
var_dump($queue->declareQueue());
var_dump($queue->bind($ex->getName(), 'routing.key'));

var_dump($queue->delete());
var_dump($ex->delete());
?>
--EXPECT--
NULL
int(0)
NULL
int(0)
NULL
