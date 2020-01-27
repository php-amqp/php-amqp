--TEST--
AMQPConnection constructor with connection_name parameter in creadentials
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$credentials = array('connection_name' => "custom connection name");
$cnn = new AMQPConnection($credentials);
var_dump($cnn->getConnectionName());
?>
--EXPECT--
string(22) "custom connection name"
