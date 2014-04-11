--TEST--
AMQPQueue::consume with timeout
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
function nop() {
}

$timeout = .68;
$conn = new AMQPConnection(array('read_timeout' => $timeout));
$conn->connect();
$chan = new AMQPChannel($conn);
$queue = new AMQPQueue($chan);
$queue->setFlags(AMQP_EXCLUSIVE);
$queue->declareQueue();
$start = microtime(true);
try {
	$queue->consume('nop');
} catch (Exception $e) {
	echo get_class($e);
	echo PHP_EOL;
}
$end = microtime(true);
$error = $end - $start - $timeout;
$limit = abs(log10($timeout)); // empirical value

echo 'error: ', $error, PHP_EOL;
echo 'limit: ', $limit, PHP_EOL;

echo abs($error) <= $limit ? 'timings OK' : 'timings failed'; // error should be less than 5% of timeout value
$queue->delete();
?>
--EXPECTF--
AMQPConnectionException
error: %f
limit: %f
timings OK
