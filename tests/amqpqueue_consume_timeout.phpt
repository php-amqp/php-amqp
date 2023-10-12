--TEST--
AMQPQueue::consume with timeout
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
elseif (!getenv("PHP_AMQP_HOST")) print "skip PHP_AMQP_HOST environment variable is not set";
?>
--FILE--
<?php
function nop() {
}

$timeout = .68;
$cnn = new AMQPConnection(array('read_timeout' => $timeout));
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->connect();
$chan = new AMQPChannel($cnn);
$queue = new AMQPQueue($chan);
$queue->setFlags(AMQP_EXCLUSIVE);
$queue->declareQueue();
$start = microtime(true);
try {
	$queue->consume('nop');
} catch (AMQPException $e) {
	echo get_class($e), "({$e->getCode()}): ", $e->getMessage();
	echo PHP_EOL;
}
$end = microtime(true);
$error = $end - $start - $timeout;
$limit = abs(log10($timeout)); // empirical value

echo 'timeout: ', $timeout, PHP_EOL;
echo 'takes: ', $end - $start, PHP_EOL;
echo 'error: ', $error, PHP_EOL;
echo 'limit: ', $limit, PHP_EOL;

echo abs($error) <= $limit ? 'timings OK' : 'timings failed'; // error should be less than 5% of timeout value
$queue->delete();
?>
--EXPECTF--
AMQPQueueException(0): Consumer timeout exceed
timeout: %f
takes: %f
error: %f
limit: %f
timings OK
