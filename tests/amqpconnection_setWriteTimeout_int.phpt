--TEST--
AMQPConnection setWriteTimeout int
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setWriteTimeout(3);
var_dump($cnn->getWriteTimeout());
?>
--EXPECT--
float(3)
