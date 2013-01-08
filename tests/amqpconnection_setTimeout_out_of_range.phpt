--TEST--
AMQPConnection setTimeout out of range
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
try {
	$cnn->setTimeout(-1);
} catch (Exception $e) {
	echo get_class($e);
	echo PHP_EOL;
    echo $e->getMessage();
}
?>
--EXPECTF--
%s: AMQPConnection::setTimeout(): AMQPConnection::setTimeout($timeout) method is deprecated; use AMQPConnection::setReadTimeout($timeout) instead in %s on line 4
AMQPConnectionException
Parameter 'timeout' must be greater than or equal to zero.
