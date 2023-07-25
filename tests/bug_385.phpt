--TEST--
#385 Testing consumer cleared on cancel
--SKIPIF--
<?php if (!extension_loaded("amqp")) {
    print "skip";
} ?>
--FILE--
<?php
$conn = new AMQPConnection();
$conn->connect();

$channel  = new AMQPChannel($conn);
$exchange = new AMQPExchange($channel);

$queue = new AMQPQueue($channel);
$queue->declareQueue();

// Asynchronously consuming
$queue->consume(null, AMQP_NOPARAM, 'someid');

// Showing that current consumer tag is from last consume call
echo $queue->getConsumerTag() ?? 'none';

echo PHP_EOL;

// Cancel by a different consumer tag than the latest one -> expecting the consumer tag to not clear
$queue->cancel('someotherid');

echo $queue->getConsumerTag() ?? 'none';

echo PHP_EOL;

// Cancel by consumer tag -> expecting the current consumer tag to clear
$queue->cancel('someid');

// Current consumer tag should be null as consumer has been cancelled
echo $queue->getConsumerTag() ?? 'none';
?>
--EXPECTF--
someid
someid
none
