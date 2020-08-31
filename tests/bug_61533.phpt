--TEST--
Constructing AMQPQueue with AMQPConnection segfaults
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$conn = new AMQPConnection();
$conn->connect();
$chan = new AMQPChannel($conn);

if (!class_exists('TypeError')) {
    class TypeError extends Exception {}
}

try {
    error_reporting(error_reporting() & ~E_WARNING);
    $queue = new AMQPQueue($conn);
} catch (TypeError $e) {
    echo get_class($e), "({$e->getCode()}): ", $e->getMessage(), '.', PHP_EOL; // we pad exception message with dot to make EXPETF be the same on PHP 5 and PHP 7
}

?>
--EXPECTF--
%s AMQPChannel%s AMQPConnection%s
