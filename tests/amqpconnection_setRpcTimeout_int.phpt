--TEST--
AMQPConnection setRpcTimeout int
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$timeout = 3;
$cnn = new AMQPConnection();
$cnn->setRpcTimeout($timeout);
var_dump($cnn->getRpcTimeout());
var_dump($timeout);
?>
--EXPECT--
float(3)
int(3)
