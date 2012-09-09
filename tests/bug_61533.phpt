--TEST--
Constructing AMQPQueue with AMQPConnection segfaults
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$conn = new AMQPConnection();
$conn->connect();
$chan = new AMQPChannel($conn);
try {
    error_reporting(error_reporting() & ~E_WARNING);
    $queue = new AMQPQueue($conn);
} catch (AMQPQueueException $e) {
    echo $e->getMessage();
}

?>
--EXPECTF--
Parameter must be an instance of AMQPChannel.
