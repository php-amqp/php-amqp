--TEST--
AMQPExchange::unbind without key with arguments
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();

$ch = new AMQPChannel($cnn);

// Declare a new exchange
$ex = new AMQPExchange($ch);
$ex->setName('exchange-unbind-' . microtime(true));
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->declareExchange();

// Declare a new exchange
$ex2 = new AMQPExchange($ch);
$ex2->setName('exchange2-unbind-' . microtime(true));
$ex2->setType(AMQP_EX_TYPE_FANOUT);
$ex2->declareExchange();

$time = microtime(true);

var_dump($ex->bind($ex2->getName(), null, array('test' => 'passed', 'at' => $time, 'i am' => 'first')));
var_dump($ex->unbind($ex2->getName(), null, array('test' => 'passed', 'at' => $time, 'i am' => 'first')));
var_dump($ex->unbind($ex2->getName(), null, array('test' => 'passed', 'at' => $time, 'i am' => 'first')));

var_dump($ex->bind($ex2->getName(), '', array('test' => 'passed', 'at' => $time, 'i am' => 'second')));
var_dump($ex->unbind($ex2->getName(), '', array('test' => 'passed', 'at' => $time, 'i am' => 'second')));
var_dump($ex->unbind($ex2->getName(), '', array('test' => 'passed', 'at' => $time, 'i am' => 'second')));

?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)