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
$queue->setName("queue-" . microtime(true));
$queue->setArgument('x-dead-letter-exchange', '');
$queue->setArgument('x-dead-letter-routing-key', 'some key');
$queue->setArgument('x-message-ttl', 42);
$queue->setFlags(AMQP_AUTODELETE);
$res = $queue->declareQueue();

var_dump($res);

$queue->delete();
?>
--EXPECT--
int(0)
