--TEST--
AMQPConnection constructor with amqp.read_timeout ini value set
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--INI--
amqp.read_timeout=202.202
--FILE--
<?php
$cnn = new AMQPConnection();
var_dump($cnn->getReadTimeout());
?>
--EXPECTF--
float(202.202)
