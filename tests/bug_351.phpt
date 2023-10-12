--TEST--
AMQPEnvelope::getBody returns false instead of empty string
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
elseif (!getenv("PHP_AMQP_HOST")) print "skip PHP_AMQP_HOST environment variable is not set";
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->connect();

$ch = new AMQPChannel($cnn);

$ex = new AMQPExchange($ch);
$ex->setName('exchange' . bin2hex(random_bytes(32)));
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->declareExchange();

$q = new AMQPQueue($ch);
$q->setName('queue1' . bin2hex(random_bytes(32)));
$q->declareQueue();

$q->bind($ex->getName(), '*');

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