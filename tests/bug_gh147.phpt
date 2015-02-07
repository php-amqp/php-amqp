--TEST--
#147 Segfault when catchable fatal error happens in consumer
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$time = microtime(true);

$connection = new AMQPConnection();
$connection->connect();

$channel = new AMQPChannel($connection);
$channel->setPrefetchCount(2);

$exchange = new AMQPExchange($channel);
$exchange->setType(AMQP_EX_TYPE_TOPIC);
$exchange->setName('test_' . $time);
$exchange->setFlags(AMQP_AUTODELETE);
$exchange->declareExchange();

$queue = new AMQPQueue($channel);
$queue->setName('test_' . $time);
$queue->declareQueue();

$queue->bind($exchange->getName(), 'test');

$exchange->publish('test message', 'test');

echo 'start', PHP_EOL;
$queue->consume(function(AMQPEnvelope $e) use (&$consume) {
    echo 'consuming';
    $e . 'should fail';

    return false;
});

echo 'done', PHP_EOL;


?>
--EXPECTF--
start
consuming
Catchable fatal error: Object of class %s could not be converted to string in %s on line %d