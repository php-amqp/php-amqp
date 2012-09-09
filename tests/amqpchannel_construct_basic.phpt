--TEST--
AMQPChannel constructor
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();
$ch = new AMQPChannel($cnn);
echo get_class($ch) . "\n";
echo $ch->isConnected() ? 'true' : 'false';
?>
--EXPECT--
AMQPChannel
true