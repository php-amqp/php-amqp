--TEST--
AMQPConnection setHost
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
?>
--FILE--
<?php
$cnn = new AMQPConnection();
var_dump($cnn->getHost());
$cnn->setHost('nonexistent');
var_dump($cnn->getHost());
--EXPECTF--
string(9) "localhost"
string(11) "nonexistent"
