--TEST--
AMQPConnection setReadTimeout out of range
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
try {
	$cnn->setReadTimeout(-1);
} catch (Exception $e) {
	echo get_class($e);
	echo PHP_EOL;
    echo $e->getMessage();
}
?>
--EXPECT--
AMQPConnectionException
Parameter 'read_timeout' must be greater than or equal to zero.
