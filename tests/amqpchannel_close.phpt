--TEST--
AMQPChannel::close
--SKIPIF--
<?php
if (!extension_loaded("amqp")) {
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

while ($messages_count++ < 3) {
    $exchange_1->publish('test message #' . $messages_count, 'test');
}

$msg = $queue_1->get();
echo $msg->getBody(), PHP_EOL;

echo 'connected: ', var_export($channel_1->isConnected(), true), PHP_EOL;
$channel_1->close();
echo 'connected: ', var_export($channel_1->isConnected(), true), PHP_EOL;

try {
  $queue_1->get();
} catch (AMQPChannelException $e) {
    echo get_class($e), "({$e->getCode()}): " . $e->getMessage(), PHP_EOL;
}

$channel_1->close();
echo 'connected: ', var_export($channel_1->isConnected(), true), PHP_EOL;

?>
--EXPECT--
test message #1
connected: true
connected: false
AMQPChannelException(0): Could not get messages from queue. No channel available.
connected: false
