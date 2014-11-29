--TEST--
AMQPQueue::consume multiple
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$time = microtime(true);

$cnn = new AMQPConnection();
$cnn->connect();

$ch = new AMQPChannel($cnn);

// Declare a new exchange
$ex = new AMQPExchange($ch);
$ex->setName('exchange-' . $time);
$ex->setType(AMQP_EX_TYPE_TOPIC);
$ex->declareExchange();

// Create and bind queues
$q1 = new AMQPQueue($ch);
$q1->setName('queue-one-' . $time);
$q1->declareQueue();
$q1->bind($ex->getName(), 'routing.one');

$q2 = new AMQPQueue($ch);
$q2->setName('queue-two-' . $time);
$q2->declareQueue();
$q2->bind($ex->getName(), 'routing.two');

$q3 = new AMQPQueue($ch);
$q3->setName('queue-three-' . $time);
$q3->declareQueue();
$q3->bind($ex->getName(), 'routing.three');


// Publish a message to the exchange with a routing key
$ex->publish('message1', 'routing.one');
$ex->publish('message2', 'routing.two');
$ex->publish('message3', 'routing.three');

$count = 0;

function consumeThings(AMQPEnvelope $message, AMQPQueue $queue)
{
    global $count;

    echo $message->getBody() . "-" . $message->getRoutingKey() . "\n";

    $count++;

    $queue->ack($message->getDeliveryTag());

    if ($count >= 2) {
        return false;
    }

    return true;
}

$q1->consume();
$q2->consume('consumeThings');

// This is important!
$q1->cancel();
$q2->cancel();

?>
--EXPECT--
message1-routing.one
message2-routing.two
