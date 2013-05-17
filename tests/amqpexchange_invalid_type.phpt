--TEST--
AMQPExchange
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();

$ch = new AMQPChannel($cnn);

$ex = new AMQPExchange($ch);
$ex->setName("exchange-" . time());
$ex->setType("invalid_exchange_type");
try {
	$ex->declareExchange();
} catch (Exception $e) {
	echo get_class($e);
	echo PHP_EOL;
	echo $e->getMessage();
}
?>
--EXPECT--
AMQPExchangeException
Server connection error: 503, message: COMMAND_INVALID - unknown exchange type 'invalid_exchange_type'
