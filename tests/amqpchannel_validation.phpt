--TEST--
AMQPChannel parameter validation
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip";
if (!getenv("PHP_AMQP_HOST")) print "skip";
?>
--FILE--
<?php


$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->connect();

$chan = new AMQPChannel($cnn);
try {
    $chan->setPrefetchSize(-1);
} catch (\Throwable $t) {
    printf("%s: %s\n", get_class($t), $t->getMessage());
}
try {
    $chan->setPrefetchSize(PHP_INT_MAX);
} catch (\Throwable $t) {
    printf("%s: %s\n", get_class($t), $t->getMessage());
}
try {
    $chan->setGlobalPrefetchSize(-1);
} catch (\Throwable $t) {
    printf("%s: %s\n", get_class($t), $t->getMessage());
}
try {
    $chan->setGlobalPrefetchSize(PHP_INT_MAX);
} catch (\Throwable $t) {
    printf("%s: %s\n", get_class($t), $t->getMessage());
}
try {
    $chan->setPrefetchCount(-1);
} catch (\Throwable $t) {
    printf("%s: %s\n", get_class($t), $t->getMessage());
}
try {
    $chan->setPrefetchCount(PHP_INT_MAX);
} catch (\Throwable $t) {
    printf("%s: %s\n", get_class($t), $t->getMessage());
}
try {
    $chan->setGlobalPrefetchCount(-1);
} catch (\Throwable $t) {
    printf("%s: %s\n", get_class($t), $t->getMessage());
}
try {
    $chan->setGlobalPrefetchCount(PHP_INT_MAX);
} catch (\Throwable $t) {
    printf("%s: %s\n", get_class($t), $t->getMessage());
}
?>
==DONE==
--EXPECTF--
AMQPConnectionException: Parameter 'prefetchSize' must be between 0 and 4294967295.
AMQPConnectionException: Parameter 'prefetchSize' must be between 0 and 4294967295.
AMQPConnectionException: Parameter 'globalPrefetchSize' must be between 0 and 4294967295.
AMQPConnectionException: Parameter 'globalPrefetchSize' must be between 0 and 4294967295.
AMQPConnectionException: Parameter 'prefetchCount' must be between 0 and 65535.
AMQPConnectionException: Parameter 'prefetchCount' must be between 0 and 65535.
AMQPConnectionException: Parameter 'globalPrefetchCount' must be between 0 and 65535.
AMQPConnectionException: Parameter 'globalPrefetchCount' must be between 0 and 65535.
==DONE==