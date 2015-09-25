--TEST--
AMQPConnection::getUsedChannels()
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
echo get_class($cnn), '::getUsedChannels():', $cnn->getUsedChannels(), PHP_EOL;
$cnn->connect();
echo get_class($cnn), '::getUsedChannels():', $cnn->getUsedChannels(), PHP_EOL;

$ch = new AMQPChannel($cnn);
echo get_class($cnn), '::getUsedChannels():', $cnn->getUsedChannels(), PHP_EOL;

$ch = null;
echo get_class($cnn), '::getUsedChannels():', $cnn->getUsedChannels(), PHP_EOL;

?>
--EXPECTF--
AMQPConnection::getUsedChannels():
Warning: AMQPConnection::getUsedChannels(): Connection is not connected. in %s on line %d
0
AMQPConnection::getUsedChannels():0
AMQPConnection::getUsedChannels():1
AMQPConnection::getUsedChannels():0
