--TEST--
AMQPConnection constructor with connection_name parameter in credentials
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
?>
--FILE--
<?php
// Test connection name as a string
$credentials = array('connection_name' => "custom connection name");
$cnn = new AMQPConnection($credentials);
var_dump($cnn->getConnectionName());
// Test explicitly setting connection name as null
$credentials = array('connection_name' => null);
$cnn = new AMQPConnection($credentials);
var_dump($cnn->getConnectionName());
?>
--EXPECT--
string(22) "custom connection name"
NULL
