--TEST--
AMQPConnection setWriteTimeout int
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
elseif (!getenv("PHP_AMQP_HOST")) print "skip PHP_AMQP_HOST environment variable is not set";
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->setWriteTimeout(3);
var_dump($cnn->getWriteTimeout());
?>
--EXPECT--
float(3)
