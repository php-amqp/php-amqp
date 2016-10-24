--TEST--
Upgrade to RabbitMQ 3.1.0-1: AMQPConnectionException: connection closed unexpectedly
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$connection = new AMQPConnection();
$connection->connect();

$channel = new AMQPChannel($connection);
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
var_dump($connection->isConnected());
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
