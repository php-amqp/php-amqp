--TEST--
AMQPConnection persistent connections
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->pconnect();

$cnn2 = new AMQPConnection();
$cnn2->pconnect();

?>
--EXPECT--
