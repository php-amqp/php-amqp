--TEST--
Channel creation race condition (https://github.com/pdezwart/php-amqp/issues/50) (1)
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$connection = new AMQPConnection();
$connection->connect();

for ($i = 0; $i < 3; $i++) {

    $channel = new AMQPChannel($connection);
    var_dump($channel->getChannelId());

    $queue = new AMQPQueue($channel);
    $queue->setName('test' . $i);

    $queue->declareQueue();
    $queue->delete();
}
?>
==DONE==
--EXPECT--
int(1)
int(2)
int(1)
==DONE==
