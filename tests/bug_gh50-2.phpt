--TEST--
Channel creation race condition (https://github.com/pdezwart/php-amqp/issues/50) (2)
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

for ($i = 0; $i < 3; $i++) {

    $channel = new AMQPChannel($cnn);
    var_dump($channel->getChannelId());

    $queue = new AMQPQueue($channel);
    $queue->setName('test' . $i);

    $queue->declareQueue();
    $queue->delete();

	unset($queue);
	unset($channel);
}
?>
==DONE==
--EXPECT--
int(1)
int(1)
int(1)
==DONE==
