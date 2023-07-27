--TEST--
AMQPExchange getConnection test
--SKIPIF--
<?php
if (!extension_loaded("amqp") || version_compare(PHP_VERSION, '5.3', '<')) {
    print "skip";
}
?>
--FILE--
<?php
$cnn = new AMQPConnection();

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