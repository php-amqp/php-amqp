--TEST--
AMQPConnection persitent constructor
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->pconnect();
echo get_class($cnn) . "\n";
echo $cnn->isConnected() ? 'true' : 'false', PHP_EOL;
echo $cnn->isPersistent() ? 'true' : 'false', PHP_EOL;
?>
--EXPECT--
AMQPConnection
true
true