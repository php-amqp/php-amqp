--TEST--
AMQPConnection constructor with timeout parameter in credentials
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$credentials = array('timeout' => 101.101);
$cnn = new AMQPConnection($credentials);
var_dump($cnn->getReadTimeout());
?>
--EXPECTF--
%s: AMQPConnection::__construct(): Parameter 'timeout' is deprecated; use 'read_timeout' instead in %s on line 3
float(101.101)
