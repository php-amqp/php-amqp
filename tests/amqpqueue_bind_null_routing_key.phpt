--TEST--
AMQPQueue bind/unbind with explicit null routing key
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
$ex->setName('exchange-' . bin2hex(random_bytes(32)));
$ex->setType(AMQP_EX_TYPE_DIRECT);
var_dump($ex->declareExchange());

$queue = new AMQPQueue($ch);
$queue->setName("queue-" . bin2hex(random_bytes(32)));
var_dump($queue->declareQueue());
var_dump($queue->bind($ex->getName(), null));
var_dump($queue->unbind($ex->getName(), null));

var_dump($queue->delete());
var_dump($ex->delete());
?>
--EXPECT--
NULL
int(0)
NULL
NULL
int(0)
NULL
