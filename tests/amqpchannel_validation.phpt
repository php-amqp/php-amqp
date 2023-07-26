--TEST--
AMQPChannel parameter validation
--SKIPIF--
<?php
if (!extension_loaded("amqp")) {
    print "skip";
}
?>
--FILE--
<?php


$con = new AMQPConnection();
$con->connect();

$chan = new AMQPChannel($con);
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
AMQPConnectionException: Parameter 'prefetch_size' must be between 0 and 4294967295.
AMQPConnectionException: Parameter 'prefetch_size' must be between 0 and 4294967295.
AMQPConnectionException: Parameter 'global_prefetch_size' must be between 0 and 4294967295.
AMQPConnectionException: Parameter 'global_prefetch_size' must be between 0 and 4294967295.
AMQPConnectionException: Parameter 'prefetch_count' must be between 0 and 65535.
AMQPConnectionException: Parameter 'prefetch_count' must be between 0 and 65535.
AMQPConnectionException: Parameter 'global_prefetch_count' must be between 0 and 65535.
AMQPConnectionException: Parameter 'global_prefetch_count' must be between 0 and 65535.
==DONE==