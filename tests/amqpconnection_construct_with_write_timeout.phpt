--TEST--
AMQPConnection constructor with write_timeout parameter in creadentials
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$credentials = array('write_timeout' => 303.303);
$cnn = new AMQPConnection($credentials);
var_dump($cnn->getWriteTimeout());
?>
--EXPECT--
float(303.303)
