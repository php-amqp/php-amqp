--TEST--
Bug 19840: Connection Exception
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$lServer['host'] = 'ip.ad.dr.ess';
$lServer['port'] = '5672';
$lServer['vhost'] = '/test';
$lServer['user'] = 'test';
$lServer['password'] = 'test';

try {
    $conn = new AMQPConnection($lServer);
	$conn->connect();
    echo "No exception thrown\n";
} catch (Exception $e) {
    echo get_class($e), "({$e->getCode()}): ", $e->getMessage();
}
?>
--EXPECT--
AMQPConnectionException(0): Socket error: could not connect to host.