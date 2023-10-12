--TEST--
AMQPConnection setRpcTimeout int
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
elseif (!getenv("PHP_AMQP_HOST")) print "skip PHP_AMQP_HOST environment variable is not set";
?>
--FILE--
<?php
$timeout = 3;
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->setRpcTimeout($timeout);
var_dump($cnn->getRpcTimeout());
var_dump($timeout);
?>
--EXPECT--
float(3)
int(3)
