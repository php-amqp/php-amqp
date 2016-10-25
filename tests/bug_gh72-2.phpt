--TEST--
#72 Publishing to an exchange with an empty name is valid and should not throw an exception (2)
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$connection = new AMQPConnection();
$connection->connect();
$channel = new AMQPChannel($connection);

$exchange = new AMQPExchange($channel);

try {
	$exchange->setName(str_repeat('a', 256));
} catch (AMQPExchangeException $e) {
	echo get_class($e), "({$e->getCode()}): ", $e->getMessage(), PHP_EOL;
}
?>
--EXPECT--
AMQPExchangeException(0): Invalid exchange name given, must be less than 255 characters long.
