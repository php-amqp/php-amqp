--TEST--
AMQPExchange::bind
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

// Declare a new exchange
$ex = new AMQPExchange($ch);
$ex->setName('exchange-' . bin2hex(random_bytes(32)));
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->declareExchange();

// Declare a new exchange
$ex2 = new AMQPExchange($ch);
$ex2->setName('exchange2-' . bin2hex(random_bytes(32)));
$ex2->setType(AMQP_EX_TYPE_FANOUT);
$ex2->declareExchange();

var_dump($ex->bind($ex2->getName(), 'test-key-1'));
var_dump($ex->bind($ex2->getName(), 'test-key-1'));

?>
--EXPECT--
NULL
NULL