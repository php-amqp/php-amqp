--TEST--
AMQPConnection constructor with timeout parameter in creadentials
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
try {
    $cnn = new AMQPConnection(array('connect_timeout' => -1.5));
} catch (AMQPConnectionException $e) {
    echo $e->getMessage(), PHP_EOL;
}

$cnn = new AMQPConnection(array('connect_timeout' => 0));

$credentials = array('host' => 'google.com', 'connect_timeout' => 1.5);
$cnn = new AMQPConnection($credentials);

$t = microtime(true);

try {
    $cnn->connect();
} catch (AMQPConnectionException $e) {
    echo $e->getMessage(), PHP_EOL;
    $t = microtime(true) - $t;

    echo ($t > 1 && $t < 2) ? 'timings OK' : 'timings failed', PHP_EOL;
}
?>
--EXPECT--
Parameter 'connect_timeout' must be greater than or equal to zero.
Socket error: could not connect to host.
timings OK