--TEST--
AMQPExchange::bind with arguments
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

$time = time();

var_dump($ex->bind($ex2->getName(), 'test', array('test' => 'passed', 'at' => $time)));
var_dump($ex->bind($ex2->getName(), 'test', array('test' => 'passed', 'at' => $time)));

?>
--EXPECT--
bool(true)
bool(true)