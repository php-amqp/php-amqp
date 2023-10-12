--TEST--
AMQPConnection constructor with amqp.timeout ini value set in code to it default value
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
elseif (!getenv("PHP_AMQP_HOST")) print "skip PHP_AMQP_HOST environment variable is not set";
?>
--FILE--
<?php
ini_set('amqp.timeout', ini_get('amqp.timeout'));

$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
var_dump($cnn->getReadTimeout());
?>
--EXPECTF--
float(0)
