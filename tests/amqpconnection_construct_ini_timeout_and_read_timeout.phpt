--TEST--
AMQPConnection constructor with both amqp.timeout and amqp.read_timeout ini values set
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--INI--
amqp.timeout = 101.101
amqp.read_timeout = 202.202
--FILE--
<?php
$cnn = new AMQPConnection();
var_dump($cnn->getReadTimeout());
?>
--EXPECTF--
%s: AMQPConnection::__construct(): INI setting 'amqp.timeout' is deprecated; use 'amqp.read_timeout' instead in %s on line 2

Notice: AMQPConnection::__construct(): INI setting 'amqp.read_timeout' will be used instead of 'amqp.timeout' in %s on line 2
float(202.202)
