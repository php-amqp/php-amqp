--TEST--
AMQPConnection constructor with verify parameter set to false in $params
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip";
?>
--FILE--
<?php
$params = array('verify' => false);
$cnn = new AMQPConnection($params);
var_dump($cnn->getVerify());
?>
--EXPECT--
bool(false)
