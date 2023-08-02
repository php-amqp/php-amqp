--TEST--
AMQPQueue::consume basic
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
require '_test_helpers.php.inc';

$cnn = new AMQPConnection();
$cnn->connect();

$ch = new AMQPChannel($cnn);

// Declare a new exchange
$ex = new AMQPExchange($ch);
$ex->setName('exchange-' . bin2hex(random_bytes(32)));
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->declareExchange();

// Create a new queue
$q = new AMQPQueue($ch);
$q->setName('queue-' . bin2hex(random_bytes(32)));
$q->declareQueue();

// Bind it on the exchange to routing.key
$q->bind($ex->getName());

// Publish a message to the exchange with a routing key
$ex->publish('message1');

$count = 0;

function consume($message, $queue) {
	global $count;
	var_dump($count++);
	return false;
}

$q->consume(null, AMQP_AUTOACK, null);

$q->delete();
$ex->delete();
?>
==DONE==
--EXPECTF--
==DONE==