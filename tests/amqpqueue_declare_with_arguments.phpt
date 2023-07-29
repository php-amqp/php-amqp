--TEST--
AMQPQueue::declareQueue() - with arguments
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
var_dump($queue->setName("queue-" . microtime(true)));
var_dump($queue->setArgument('x-dead-letter-exchange', ''));
var_dump($queue->setArgument('x-dead-letter-routing-key', 'some key'));
var_dump($queue->setArgument('x-message-ttl', 42));
var_dump($queue->setFlags(AMQP_AUTODELETE));
var_dump($queue->declareQueue());

var_dump($queue->delete());
?>
--EXPECT--
NULL
NULL
NULL
NULL
NULL
int(0)
int(0)
