--TEST--
AMQPConnection constructor
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
function nop() {
}

$timeout = .68;
$conn = new AMQPConnection(array('timeout' => $timeout));
$conn->connect();
$chan = new AMQPChannel($conn);
$queue = new AMQPQueue($chan);
$queue->setFlags(AMQP_EXCLUSIVE);
$queue->declare();
$start = microtime(true);
try {
	$queue->consume('nop');
} catch (Exception $e) {
	echo get_class($e);
	echo PHP_EOL;
	echo $e->getCode() == constant('AMQP_OS_SOCKET_TIMEOUT_ERRNO') ? 'true' : 'false';
	echo PHP_EOL;
}
$end = microtime(true);
echo abs($end - $start - $timeout) < 0.005 ? 'true' : 'false';
?>
--EXPECT--
AMQPConnectionException
true
true