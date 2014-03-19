--TEST--
AMQPConnection setVhost
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
var_dump($cnn->getVhost());
$cnn->setVhost('nonexistent');
var_dump($cnn->getVhost());
--EXPECTF--
string(1) "/"
string(11) "nonexistent"
