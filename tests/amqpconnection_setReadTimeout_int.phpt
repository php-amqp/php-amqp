--TEST--
AMQPConnection setReadTimeout int
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip";
if (!getenv("PHP_AMQP_HOST")) print "skip";
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->setReadTimeout(3);
var_dump($cnn->getReadTimeout());
?>
--EXPECT--
float(3)
