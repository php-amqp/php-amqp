--TEST--
AMQPConnection setReadTimeout int
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setReadTimeout(3);
var_dump($cnn->getReadTimeout());
?>
--EXPECT--
float(3)
