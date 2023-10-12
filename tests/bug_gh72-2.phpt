--TEST--
#72 Publishing to an exchange with an empty name is valid and should not throw an exception (2)
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
$channel = new AMQPChannel($cnn);

$exchange = new AMQPExchange($channel);

try {
	$exchange->setName(str_repeat('a', 256));
} catch (AMQPExchangeException $e) {
	echo get_class($e), "({$e->getCode()}): ", $e->getMessage(), PHP_EOL;
}
?>
--EXPECT--
AMQPExchangeException(0): Invalid exchange name given, must be less than 255 characters long.
