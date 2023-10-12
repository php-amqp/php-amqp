--TEST--
#147 Segfault when catchable fatal error happens in consumer
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
elseif (!getenv("PHP_AMQP_HOST")) print "skip PHP_AMQP_HOST environment variable is not set";
?>
--FILE--
<?php

// Register error handler to be able to catch the error
// as it's not catchable with try/catch before PHP 7.4
// see https://github.com/php/php-src/pull/3887
function exception_error_handler($severity, $message, $file, $line) {
    echo $message;
    exit;
}
set_error_handler("exception_error_handler");

$time = microtime(true);

$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->connect();

$channel = new AMQPChannel($cnn);
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

echo 'start'.PHP_EOL;
try {
    $queue->consume(function(AMQPEnvelope $e) use (&$consume) {
        echo 'consuming'.PHP_EOL;
        $e . 'should fail';

        return false;
    });
} catch (Throwable $ex) {
    // Exception is only thrown as of PHP 7.4
    echo $ex->getMessage();
    exit;
}

echo 'done', PHP_EOL;


?>
--EXPECTF--
start
consuming
Object of class %s could not be converted to string