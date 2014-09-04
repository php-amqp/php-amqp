--TEST--
AMQPQueue::nack
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
ini_set('amqp.auto_ack', false);
$cnn = new AMQPConnection();
$cnn->connect();

$ch = new AMQPChannel($cnn);

// Declare a new exchange
$ex = new AMQPExchange($ch);
$ex->setName('testnack' . microtime(true));
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->declareExchange();
$exchangeName = $ex->getName();

// Create a new queue
$q = new AMQPQueue($ch);
$q->setName('testnack' . microtime(true));
$q->declareQueue();
$q->bind($exchangeName, '#');

// Bind it on the exchange to routing.key
// Publish a message to the exchange with a routing key
$ex->publish('message', 'foo');

$env = $q->get(AMQP_NOPARAM);
echo $env->getBody() . PHP_EOL;
echo $env->isRedelivery() ? 'true' : 'false';
echo PHP_EOL;

// send the message back to the queue
$q->nack($env->getDeliveryTag(), AMQP_REQUEUE);

// read again
$env2  = $q->get(AMQP_NOPARAM);
if (false !== $env2) {
    $q->ack($env2->getDeliveryTag());
    echo $env2->getBody() . PHP_EOL;
    echo $env2->isRedelivery() ? 'true' : 'false';
    echo PHP_EOL;
} else {
    echo "could not read message" . PHP_EOL;
}

$ex->delete();
$q->delete();
--EXPECT--
message
false
message
true
