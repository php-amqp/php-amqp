--TEST--
AMQPConnection setTimeout float
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setTimeout(".34");
var_dump($cnn->getTimeout());
$cnn->setTimeout("4.7e-2");
var_dump($cnn->getTimeout());?>
--EXPECT--
float(0.34)
float(0.047)