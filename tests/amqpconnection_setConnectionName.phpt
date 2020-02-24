--TEST--
AMQPConnection setConnectionName
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
var_dump($cnn->getConnectionName());
$cnn->setConnectionName('custom connection name');
var_dump($cnn->getConnectionName());
$cnn->setConnectionName(null);
var_dump($cnn->getConnectionName());
--EXPECTF--
NULL
string(22) "custom connection name"
NULL