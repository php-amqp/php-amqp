--TEST--
AMQPConnection constructor with both timeout and read_timeout parameters in credentials
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$credentials = array('timeout' => 101.101, 'read_timeout' => 202.202);
$cnn = new AMQPConnection($credentials);
var_dump($cnn->getReadTimeout());
?>
--EXPECTF--
Notice: AMQPConnection::__construct(): Parameter 'timeout' is deprecated, 'read_timeout' used instead in %s on line 3
float(202.202)
