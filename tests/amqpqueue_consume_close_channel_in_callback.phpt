--TEST--
AMQPQueue::consume() - closing the channel inside the callback throws instead of crashing
--EXTENSIONS--
amqp
--SKIPIF--
<?php
if (!getenv("PHP_AMQP_HOST")) print "skip PHP_AMQP_HOST environment variable is not set";
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->connect();

$channel = new AMQPChannel($cnn);

$queue = new AMQPQueue($channel);
$queue->setName('consume-close-' . bin2hex(random_bytes(16)));
$queue->setFlags(AMQP_AUTODELETE);
$queue->declareQueue();

$exchange = new AMQPExchange($channel);
$exchange->setType(AMQP_EX_TYPE_DIRECT);
$exchange->setName('');
$exchange->publish('payload', $queue->getName());

try {
    $queue->consume(function (AMQPEnvelope $envelope, AMQPQueue $q) use ($channel) {
        echo 'received: ', $envelope->getBody(), "\n";
        $channel->close();
        return true;
    });
    echo "consume returned\n";
} catch (AMQPQueueException $e) {
    echo $e::class, ': ', $e->getMessage(), "\n";
}
?>
--EXPECT--
received: payload
AMQPQueueException: Channel was disconnected during the consume callback.
