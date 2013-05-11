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

$channel->setPrefetchSize(1024);
var_dump($channel->getPrefetchSize());
var_dump($channel->getPrefetchCount());


?>
==DONE==
--EXPECT--
int(0)
int(3)
int(0)
int(10)
int(1024)
int(0)
==DONE==
