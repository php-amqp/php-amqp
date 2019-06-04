--TEST--
AMQPConnection setRpcTimeout float
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$timeout = .34;
$cnn = new AMQPConnection();
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
