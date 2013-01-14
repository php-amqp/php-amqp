--TEST--
AMQPConnection setTimeout deprecated
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setTimeout(0);
$cnn->getTimeout();
?>
--EXPECTF--
%s: AMQPConnection::setTimeout(): AMQPConnection::setTimeout($timeout) method is deprecated; use AMQPConnection::setReadTimeout($timeout) instead in %s on line 3

%s: AMQPConnection::getTimeout(): AMQPConnection::getTimeout() method is deprecated; use AMQPConnection::getReadTimeout() instead in %s on line 4
