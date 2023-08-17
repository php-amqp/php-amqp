--TEST--
AMQPConnection setPassword
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip";
if (!getenv("PHP_AMQP_HOST")) print "skip";
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
var_dump($cnn->getPassword());
$cnn->setPassword('nonexistent');
var_dump($cnn->getPassword());
--EXPECTF--
string(5) "guest"
string(11) "nonexistent"
