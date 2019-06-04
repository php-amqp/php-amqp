--TEST--
AMQPConnection constructor with rpc_timeout parameter in creadentials
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$credentials = array('rpc_timeout' => 303.303);
$cnn = new AMQPConnection($credentials);
var_dump($cnn->getRpcTimeout());
?>
--EXPECT--
float(303.303)
