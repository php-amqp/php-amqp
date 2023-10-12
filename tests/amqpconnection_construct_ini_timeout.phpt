--TEST--
AMQPConnection constructor with amqp.timeout ini value set
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
elseif (!getenv("PHP_AMQP_HOST")) print "skip PHP_AMQP_HOST environment variable is not set";
?>
--INI--
amqp.timeout=101.101
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
var_dump($cnn->getReadTimeout());
?>
--EXPECTF--
%s: AMQPConnection::__construct(): INI setting 'amqp.timeout' is deprecated; use 'amqp.read_timeout' instead in %s on line 2
float(101.101)
