--TEST--
Upgrade to RabbitMQ 3.1.0-1: AMQPConnectionException: connection closed unexpectedly
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$connection = new AMQPConnection();
$connection->connect();

$channel = new AMQPChannel($connection);
print_r($channel);

$channel->setPrefetchCount(10);
print_r($channel);

$channel->setPrefetchCount(3);
print_r($channel);


?>
==DONE==
--EXPECTF--
AMQPChannel Object
(
    [channel_id] => 1
    [prefetch_count] => 3
    [prefetch_size] => 0
)
AMQPChannel Object
(
    [channel_id] => 1
    [prefetch_count] => 10
    [prefetch_size] => 0
)
AMQPChannel Object
(
    [channel_id] => 1
    [prefetch_count] => 3
    [prefetch_size] => 0
)
==DONE==
