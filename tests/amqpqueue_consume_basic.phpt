--TEST--
AMQPQueue::consume basic
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();

$ch = new AMQPChannel($cnn);

// Declare a new exchange
$ex = new AMQPExchange($ch);
$ex->setName('exchange-' . time());
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->declareExchange();

// Declare a new exchange
$ex2 = new AMQPExchange($ch);
$ex2->setName('exchange2-' . time());
$ex2->setType(AMQP_EX_TYPE_FANOUT);
$ex2->declareExchange();


// Create a new queue
$q = new AMQPQueue($ch);
$q->setName('queue-' . time());
$q->declareQueue();

// Bind it on the exchange to routing.key
$q->bind($ex->getName(), 'routing.*');

// Publish a message to the exchange with a routing key
$ex->publish('message1', 'routing.1');
$ex->publish('message2', 'routing.2');
$ex->publish('message3', 'routing.3');

$count = 0;

function consumeThings($message, $queue) {
	global $count;

	echo $message->getBody() . "-" . $message->getRoutingKey() . "\n";

	$count++;

	if ($count >= 2) {
		global $ex, $ex2;
		global $q;

        $ex->bind($ex2->getName(), 'first-*', array('test' => 'passed', 'at' => time()));
        $ex->bind($ex2->getName(), 'second-*');
        $q->delete();
		return false;
	}
	return true;
}

// Read from the queue
$q->consume("consumeThings");


?>
--EXPECT--
message1-routing.1
message2-routing.2
