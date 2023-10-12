--TEST--
AMQPConnection constructor with verify parameter set to false in $params
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
?>
--FILE--
<?php
$params = array('verify' => false);
$cnn = new AMQPConnection($params);
var_dump($cnn->getVerify());
?>
--EXPECT--
bool(false)
