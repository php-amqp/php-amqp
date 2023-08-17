--TEST--
#72 Publishing to an exchange with an empty name is valid and should not throw an exception (1)
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip";
if (!getenv("PHP_AMQP_HOST")) print "skip";
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->connect();
$channel = new AMQPChannel($cnn);

$exchange = new AMQPExchange($channel);
$exchange->setName('');

$exchange->publish('msg', 'key');
?>
==DONE==
--EXPECT--
==DONE==
