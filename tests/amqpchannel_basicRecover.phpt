--TEST--
AMQPChannel::basicRecover
--SKIPIF--
<?php
if (!extension_loaded("amqp") || version_compare(PHP_VERSION, '5.3', '<')) {
    print "skip";
}
?>
--FILE--
<?php
$time = microtime(true);

$connection_1 = new AMQPConnection();
$connection_1->connect();

$channel_1 = new AMQPChannel($connection_1);
$channel_1->setPrefetchCount(5);

$exchange_1 = new AMQPExchange($channel_1);
$exchange_1->setType(AMQP_EX_TYPE_TOPIC);
$exchange_1->setName('test_' . $time);
$exchange_1->setFlags(AMQP_AUTODELETE);
$exchange_1->declareExchange();

$queue_1 = new AMQPQueue($channel_1);
$queue_1->setName('test_' . $time);
$queue_1->setFlags(AMQP_DURABLE);
$queue_1->declareQueue();

$queue_1->bind($exchange_1->getName(), 'test');

$messages_count = 0;
while ($messages_count++ < 10) {
    $exchange_1->publish('test message #' . $messages_count, 'test');
    //echo 'published test message #' . $messages_count, PHP_EOL;
}

$consume = 2; // NOTE: by default prefetch-count=3, so in consumer below we will ignore prefetched messages 3-5,
              //       and they will not seen by other consumers until we redeliver it.
$queue_1->consume(function(AMQPEnvelope $e, AMQPQueue $q) use (&$consume) {
    echo 'consumed ', $e->getBody(), ' ', ($e->isRedelivery() ? '(redelivered)' : '(original)'), PHP_EOL;
    $q->ack($e->getDeliveryTag());

    return (-- $consume > 0);
});
$queue_1->cancel(); // we have to do that to prevent redelivering to the same consumer

$connection_2 = new AMQPConnection();
$connection_2->setReadTimeout(1);

$connection_2->connect();
$channel_2 = new AMQPChannel($connection_2);
$channel_2->setPrefetchCount(8);


$queue_2 = new AMQPQueue($channel_2);
$queue_2->setName('test_' . $time);

$consume = 10;
try {

    $queue_2->consume(function (AMQPEnvelope $e, AMQPQueue $q) use (&$consume) {
        echo 'consumed ' . $e->getBody(), ' ', ($e->isRedelivery() ? '(redelivered)' : '(original)'), PHP_EOL;
        $q->ack($e->getDeliveryTag());

        return (--$consume > 0);
    });

} catch (AMQPException $e) {
    echo get_class($e), "({$e->getCode()}): ", $e->getMessage(), PHP_EOL;
}
$queue_2->cancel();
//var_dump($connection_2, $channel_2);die;


// yes, we do it repeatedly, basic.recover works in a slightly different way than it looks like. As it said,
// it "asks the server to redeliver all unacknowledged messages on a specified channel.
// ZERO OR MORE messages MAY BE redelivered"
$channel_1->basicRecover();

echo 'redelivered', PHP_EOL;

$consume = 10;
try {

    $queue_2->consume(function (AMQPEnvelope $e, AMQPQueue $q) use (&$consume) {
        echo 'consumed ' . $e->getBody(), ' ', ($e->isRedelivery() ? '(redelivered)' : '(original)'), PHP_EOL;
        $q->ack($e->getDeliveryTag());

        return (--$consume > 0);
    });

} catch (AMQPException $e) {
    echo get_class($e), "({$e->getCode()}): ", $e->getMessage(), PHP_EOL;
}


?>
--EXPECT--
consumed test message #1 (original)
consumed test message #2 (original)
consumed test message #8 (original)
consumed test message #9 (original)
consumed test message #10 (original)
AMQPQueueException(0): Consumer timeout exceed
redelivered
consumed test message #3 (redelivered)
consumed test message #4 (redelivered)
consumed test message #5 (redelivered)
consumed test message #6 (redelivered)
consumed test message #7 (redelivered)
AMQPQueueException(0): Consumer timeout exceed
