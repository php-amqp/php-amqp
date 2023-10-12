--TEST--
AMQPConnection constructor
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
elseif (!getenv("PHP_AMQP_HOST")) print "skip PHP_AMQP_HOST environment variable is not set";
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->connect();
echo get_class($cnn) . "\n";
echo $cnn->isConnected() ? 'true' : 'false', PHP_EOL;
echo $cnn->isPersistent() ? 'true' : 'false', PHP_EOL;
?>
--EXPECT--
AMQPConnection
true
false