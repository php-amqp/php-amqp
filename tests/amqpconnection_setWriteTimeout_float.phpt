--TEST--
AMQPConnection setWriteTimeout float
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip";
if (!getenv("PHP_AMQP_HOST")) print "skip";
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->setWriteTimeout(.34);
var_dump($cnn->getWriteTimeout());
$cnn->setWriteTimeout(4.7e-2);
var_dump($cnn->getWriteTimeout());?>
--EXPECT--
float(0.34)
float(0.047)
