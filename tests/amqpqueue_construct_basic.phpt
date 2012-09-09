--TEST--
AMQPQueue constructor
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();

$ch = new AMQPChannel($cnn);

$queue = new AMQPQueue($ch);
echo get_class($queue);
?>
--EXPECT--
AMQPQueue