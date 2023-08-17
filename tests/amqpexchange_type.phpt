--TEST--
AMQPExchange getConnection test
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip";
if (!getenv("PHP_AMQP_HOST")) print "skip";
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));

$cnn->connect();
$ch = new AMQPChannel($cnn);

$ex = new AMQPExchange($ch);
var_dump($ex->getType());
$ex->setType(AMQP_EX_TYPE_FANOUT);
var_dump($ex->getType());
$ex->setType(null);
var_dump($ex->getType());
$ex->setType(AMQP_EX_TYPE_DIRECT);
var_dump($ex->getType());
$ex->setType("");
var_dump($ex->getType());
?>
==DONE==
--EXPECT--
NULL
string(6) "fanout"
NULL
string(6) "direct"
NULL
==DONE==