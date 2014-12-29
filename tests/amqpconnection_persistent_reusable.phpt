--TEST--
AMQPConnection persistent connection are reusable
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->pconnect();
echo get_class($cnn), PHP_EOL;
echo $cnn->isConnected() ? 'true' : 'false', PHP_EOL;

$cnn = null;
echo PHP_EOL;

$cnn = new AMQPConnection();
$cnn->pconnect();
echo get_class($cnn), PHP_EOL;
echo $cnn->isConnected() ? 'true' : 'false', PHP_EOL;

$cnn->pdisconnect();

?>
--EXPECT--
AMQPConnection
true

AMQPConnection
true
