--TEST--
AMQPConnection setWriteTimeout string
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setWriteTimeout(".34");
var_dump($cnn->getWriteTimeout());
$cnn->setWriteTimeout("4.7e-2");
var_dump($cnn->getWriteTimeout());
?>
--EXPECT--
float(0.34)
float(0.047)
