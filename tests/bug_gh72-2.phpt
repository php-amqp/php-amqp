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
	var_dump($e->getMessage());
}
?>
==DONE==
--EXPECT--
string(67) "Invalid exchange name given, must be less than 255 characters long."
==DONE==
