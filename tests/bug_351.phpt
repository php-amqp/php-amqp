--TEST--
AMQPEnvelope::getBody returns false instead of empty string
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();

$ch = new AMQPChannel($cnn);

// Declare a new exchange
$ex = new AMQPExchange($ch);
$ex->setName('exchange' . microtime(true));
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->declareExchange();

// Create a new queue
$q = new AMQPQueue($ch);
$q->setName('queue1' . microtime(true));
$q->declareQueue();

// Bind it on the exchange to routing.key
$q->bind($ex->getName(), '*');

// Publish a message to the exchange with a routing key
$ex->publish('');

$msg = $q->get(AMQP_AUTOACK);
echo "MSG1\n";
var_dump($msg->getBody(), $msg->getRoutingKey(), $msg->getConsumerTag(), $msg->getDeliveryTag(), $msg->getExchangeName());

$msg2 = new AMQPEnvelope();
echo "MSG2\n";
var_dump($msg2->getBody(), $msg2->getRoutingKey(), $msg2->getConsumerTag(), $msg2->getDeliveryTag(), $msg2->getExchangeName());
?>
==DONE==
--EXPECTF--
MSG1
string(0) ""
string(0) ""
string(0) ""
int(%d)
string(%d) "exchange%s"
MSG2
string(0) ""
string(0) ""
NULL
NULL
NULL
==DONE==