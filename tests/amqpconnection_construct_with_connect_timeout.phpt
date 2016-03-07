--TEST--
AMQPConnection constructor with timeout parameter in credentials
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
try {
    $cnn = new AMQPConnection(array('connect_timeout' => -1.5));
} catch (AMQPConnectionException $e) {
    echo $e->getMessage(), PHP_EOL;
}

$timeout = 10.5;

// resolve hostname to don't waste time on resolve inside library while resolve operations are not under timings limit (yet)
$credentials = array('host' => gethostbyname('google.com'), 'connect_timeout' => $timeout);
//$credentials = array('host' => 'google.com', 'connect_timeout' => $timeout);
$cnn = new AMQPConnection($credentials);

$start = microtime(true);

try {
    $cnn->connect();
} catch (AMQPConnectionException $e) {
    echo $e->getMessage(), PHP_EOL;
    $end = microtime(true);

    $error = $end - $start - $timeout;

    $limit = abs(log10($timeout));  // empirical value

    echo 'error: ', $error, PHP_EOL;
    echo 'limit: ', $limit, PHP_EOL;

    echo abs($error) <= $limit ? 'timings OK' : 'timings failed'; // error should be less than 5% of timeout value
}
?>
--EXPECTF--
Parameter 'connect_timeout' must be greater than or equal to zero.
Socket error: could not connect to host.
error: %f
limit: %f
timings OK
