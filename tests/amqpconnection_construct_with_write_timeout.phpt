--TEST--
AMQPConnection constructor with write_timeout parameter in $cnntials
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
?>
--FILE--
<?php
$credentials = array('write_timeout' => 303.303);
$cnn = new AMQPConnection($credentials);
var_dump($cnn->getWriteTimeout());
?>
--EXPECT--
float(303.303)
