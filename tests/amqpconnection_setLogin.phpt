--TEST--
AMQPConnection setLogin
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
var_dump($cnn->getLogin());
$cnn->setLogin('nonexistent');
var_dump($cnn->getLogin());
--EXPECTF--
string(5) "guest"
string(11) "nonexistent"
