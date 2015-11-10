<?php

set_time_limit(getenv('TEST_TIMEOUT') ?: 120);

try {
    $ext =new ReflectionExtension('xdebug');
    $xdebug = '(with xdebug '.$ext->getVersion().')';
} catch (Exception $e) {
    $xdebug = '(without xdebug)';
}

$ext = new ReflectionExtension('amqp');
$srcVersion = $ext->getVersion();

echo 'Running benchmark for php-amqp ', $srcVersion, ' on PHP ', PHP_VERSION, ' ', $xdebug, PHP_EOL;

$iterations = 10000;

if (isset($argv[1])) {
    $iterations = max((int) $argv[1], 0) ?: $iterations;
}
echo '  running ', $iterations, ' iterations:', PHP_EOL, PHP_EOL;


$conn = new AMQPConnection();
$conn->connect();

$ch = new AMQPChannel($conn);

$exchange = new AMQPExchange($ch);
$exchange->setType(AMQP_EX_TYPE_FANOUT);
$exchange->setFlags(AMQP_AUTODELETE);
$exchange->setName('benchmark_exchange_' . microtime(true));
$exchange->declareExchange();

$q = new AMQPQueue($ch);
$q->setFlags(AMQP_AUTODELETE);
$q->declareQueue();
$q->bind($exchange->getName());


$message = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";

$timer = microtime(true);
for ($i = 0; $i < $iterations; $i++) {
    $exchange->publish($message);
}
$timer = microtime(true) - $timer;

echo 'Publish: ', $iterations, ' iterations took ', $timer, 'sec', PHP_EOL;


$timer = microtime(true);
for ($i = 0; $i < $iterations; $i++) {
    if (!$q->get(AMQP_AUTOACK)) {
        echo 'GET failed', PHP_EOL;
    }
}
$timer = microtime(true) - $timer;

echo '    Get: ', $iterations, ' iterations took ', $timer, 'sec', PHP_EOL;

$q->delete();
$exchange->delete();

echo PHP_EOL;
// ==================================


$exchange = new AMQPExchange($ch);
$exchange->setType(AMQP_EX_TYPE_FANOUT);
$exchange->setFlags(AMQP_AUTODELETE);
$exchange->setName('benchmark_exchange_' . microtime(true));
$exchange->declareExchange();

$q = new AMQPQueue($ch);
$q->setFlags(AMQP_AUTODELETE);
$q->declareQueue();
$q->bind($exchange->getName());

$timer = microtime(true);
for ($i = 0; $i < $iterations; $i++) {
    $exchange->publish($message);
}
$timer = microtime(true) - $timer;

echo 'Publish: ', $iterations, ' iterations took ', $timer, 'sec', PHP_EOL;

$consumer_iterations = $iterations;

$timer = microtime(true);
$q->consume(
    function () use (&$consumer_iterations) {
        return (--$consumer_iterations > 0);
    },
    AMQP_AUTOACK);
$timer = microtime(true) - $timer;

echo 'Consume: ', $iterations, ' iterations took ', $timer, 'sec', PHP_EOL;

$q->delete();
$exchange->delete();
