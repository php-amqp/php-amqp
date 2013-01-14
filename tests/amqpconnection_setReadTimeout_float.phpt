--TEST--
AMQPConnection setReadTimeout float
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setReadTimeout(.34);
var_dump($cnn->getReadTimeout());
$cnn->setReadTimeout(4.7e-2);
var_dump($cnn->getReadTimeout());?>
--EXPECT--
float(0.34)
float(0.047)
