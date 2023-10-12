--TEST--
AMQPConnection setRpcTimeout float
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
elseif (!getenv("PHP_AMQP_HOST")) print "skip PHP_AMQP_HOST environment variable is not set";
?>
--FILE--
<?php
$timeout = .34;
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->setRpcTimeout($timeout);
var_dump($cnn->getRpcTimeout());
var_dump($timeout);
$timeout = 4.7e-2;
$cnn->setRpcTimeout($timeout);
var_dump($cnn->getRpcTimeout());
var_dump($timeout);
?>
--EXPECT--
float(0.34)
float(0.34)
float(0.047)
float(0.047)
