--TEST--
AMQPConnection setVhost
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
elseif (!getenv("PHP_AMQP_HOST")) print "skip PHP_AMQP_HOST environment variable is not set";
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
var_dump($cnn->getVhost());
$cnn->setVhost('nonexistent');
var_dump($cnn->getVhost());
--EXPECTF--
string(1) "/"
string(11) "nonexistent"
