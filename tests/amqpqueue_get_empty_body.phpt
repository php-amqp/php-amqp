--TEST--
AMQPQueue::get empty body
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip";
if (!getenv("PHP_AMQP_HOST")) print "skip";
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->connect();

$ch = new AMQPChannel($cnn);

// Declare a new exchange
$ex = new AMQPExchange($ch);
$ex->setName('exchange' . bin2hex(random_bytes(32)));
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->declareExchange();

// Create a new queue
$q = new AMQPQueue($ch);
$q->setName('queue1' . bin2hex(random_bytes(32)));
$q->declareQueue();

// Bind it on the exchange to routing.key
$q->bind($ex->getName(), 'routing.*');

// Publish a message to the exchange with a routing key
$ex->publish('', 'routing.1');

// Read from the queue
$msg = $q->get(AMQP_AUTOACK);
echo "'" . $msg->getBody() . "'\n";

$msg = $q->get(AMQP_AUTOACK);

if ($msg === null) {
	echo "No more messages\n";
}

$ex->delete();
$q->delete();
?>
--EXPECT--
''
No more messages
