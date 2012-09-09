--TEST--
AMQPConnection constructor
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();
echo get_class($cnn) . "\n";
echo $cnn->isConnected() ? 'true' : 'false';
?>
--EXPECT--
AMQPConnection
true