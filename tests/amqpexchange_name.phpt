--TEST--
AMQPExchange getConnection test
--SKIPIF--
<?php
if (!extension_loaded("amqp")) {
    print "skip";
}
?>
--FILE--
<?php
$cnn = new AMQPConnection();

$cnn->connect();
$ch = new AMQPChannel($cnn);

$ex = new AMQPExchange($ch);
var_dump($ex->getName());
$ex->setName('exchange');
var_dump($ex->getName());
$ex->setName('');
var_dump($ex->getName());
$ex->setName(null);
var_dump($ex->getName());
?>
==DONE==
--EXPECT--
NULL
string(8) "exchange"
NULL
NULL
==DONE==