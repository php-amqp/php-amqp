--TEST--
AMQPQueue::get basic
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();

$ch = new AMQPChannel($cnn);

// Declare a new exchange
$ex = new AMQPExchange($ch);
$ex->setName('exchange1');
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->declareExchange();

// Create a new queue
$q = new AMQPQueue($ch);
$q->setName('queue1' . time());
$q->declareQueue();

// Bind it on the exchange to routing.key
$q->bind($ex->getName(), 'routing.*');
// Publish a message to the exchange with a routing key
$ex->publish('message', 'routing.1');
$ex->publish('message2', 'routing.2');
$ex->publish('message3', 'routing.3');


for ($i = 0; $i < 3; $i++) {
	// Read from the queue
	$msg = $q->get(AMQP_AUTOACK);
    echo $msg->getBody() . "\n";
}

$ex->delete();
?>
--EXPECT--
message
message2
message3
