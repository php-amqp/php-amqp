--TEST--
Constructing AMQPQueue with AMQPConnection segfaults
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
elseif (!getenv("PHP_AMQP_HOST")) print "skip PHP_AMQP_HOST environment variable is not set";
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->connect();
$chan = new AMQPChannel($cnn);

if (!class_exists('TypeError')) {
    class TypeError extends Exception {}
}

try {
    error_reporting(error_reporting() & ~E_WARNING);
    $queue = new AMQPQueue($cnn);
} catch (TypeError $e) {
    echo get_class($e), "({$e->getCode()}): ", $e->getMessage(), '.', PHP_EOL; // we pad exception message with dot to make EXPETF be the same on PHP 5 and PHP 7
}

?>
--EXPECTF--
%s AMQPChannel%s AMQPConnection%s
