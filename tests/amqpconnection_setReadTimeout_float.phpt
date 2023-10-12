--TEST--
AMQPConnection setReadTimeout float
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
elseif (!getenv("PHP_AMQP_HOST")) print "skip PHP_AMQP_HOST environment variable is not set";
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->setReadTimeout(.34);
var_dump($cnn->getReadTimeout());
$cnn->setReadTimeout(4.7e-2);
var_dump($cnn->getReadTimeout());?>
--EXPECT--
float(0.34)
float(0.047)
