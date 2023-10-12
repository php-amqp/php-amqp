--TEST--
AMQPConnection constructor with amqp.read_timeout ini value set
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
elseif (!getenv("PHP_AMQP_HOST")) print "skip PHP_AMQP_HOST environment variable is not set";
?>
--INI--
amqp.read_timeout=202.202
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
var_dump($cnn->getReadTimeout());
?>
--EXPECTF--
float(202.202)
