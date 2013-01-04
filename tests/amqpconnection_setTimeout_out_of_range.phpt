--TEST--
AMQPConnection setTimeout float
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
try {
	@$cnn->setTimeout(-1);
} catch (Exception $e) {
	echo get_class($e);
	echo PHP_EOL;
    echo $e->getMessage();
}
?>
--EXPECT--
AMQPConnectionException
Parameter 'timeout' must be greater than or equal to zero.
