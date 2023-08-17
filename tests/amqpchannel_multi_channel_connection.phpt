--TEST--
AMQPConnection - multiple AMQPChannels per AMQPConnection
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip";
if (!getenv("PHP_AMQP_HOST")) print "skip";
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->connect();

$ch = new AMQPChannel($cnn);
echo get_class($ch) . "\n";
echo $ch->isConnected() ? 'true' : 'false';
echo "\n";
$ch2 = new AMQPChannel($cnn);
echo get_class($ch) . "\n";
echo $ch->isConnected() ? 'true' : 'false';

unset($ch2);

$ch->setPrefetchCount(10);

?>
--EXPECT--
AMQPChannel
true
AMQPChannel
true
