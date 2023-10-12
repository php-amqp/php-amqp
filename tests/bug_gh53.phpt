--TEST--
Upgrade to RabbitMQ 3.1.0-1: AMQPConnectionException: connection closed unexpectedly
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

$channel = new AMQPChannel($cnn);
var_dump($channel->getPrefetchSize());
var_dump($channel->getPrefetchCount());

$channel->setPrefetchCount(10);
var_dump($channel->getPrefetchSize());
var_dump($channel->getPrefetchCount());

// NOTE: RabbitMQ Doesn't support prefetch size
try {
    $channel->setPrefetchSize(1024);

} catch (AMQPConnectionException $e) {
    echo get_class($e), "({$e->getCode()}): ", $e->getMessage(), PHP_EOL;
}
var_dump($channel->isConnected());
var_dump($cnn->isConnected());
var_dump($channel->getPrefetchSize());
var_dump($channel->getPrefetchCount());

?>
--EXPECTF--
int(0)
int(3)
int(0)
int(10)
AMQPConnectionException(540): Server connection error: 540, message: NOT_IMPLEMENTED - prefetch_size!=0 (%d)
bool(false)
bool(false)
int(0)
int(10)
