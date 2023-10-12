--TEST--
AMQPChannel::setPrefetchCount
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
$ch = new AMQPChannel($cnn);

var_dump($ch->getGlobalPrefetchCount());
var_dump($ch->getGlobalPrefetchSize());
var_dump($ch->getPrefetchCount());
var_dump($ch->getPrefetchSize());

$ch->setPrefetchCount(123);

var_dump($ch->getGlobalPrefetchCount());
var_dump($ch->getGlobalPrefetchSize());
var_dump($ch->getPrefetchCount());
var_dump($ch->getPrefetchSize());

?>
--EXPECT--
int(0)
int(0)
int(3)
int(0)
int(0)
int(0)
int(123)
int(0)
