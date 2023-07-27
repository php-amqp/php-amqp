--TEST--
AMQPConnection setPort with string
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$port = '12345';
var_dump($cnn->setPort($port));
var_dump($cnn->getPort());
var_dump($port);
?>
--EXPECT--
NULL
int(12345)
string(5) "12345"
