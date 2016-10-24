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
$ex->setName("exchange-" . microtime(true));
$ex->setType("invalid_exchange_type");

echo "Channel ", $ch->isConnected() ? 'connected' : 'disconnected', PHP_EOL;
echo "Connection ", $cnn->isConnected() ? 'connected' : 'disconnected', PHP_EOL;

try {
	$ex->declareExchange();
} catch (AMQPException $e) {
	echo get_class($e), "({$e->getCode()}): ", $e->getMessage(), PHP_EOL;
}

echo "Channel ", $ch->isConnected() ? 'connected' : 'disconnected', PHP_EOL;
echo "Connection ", $cnn->isConnected() ? 'connected' : 'disconnected', PHP_EOL;

?>
--EXPECT--
Channel connected
Connection connected
AMQPConnectionException(503): Server connection error: 503, message: COMMAND_INVALID - unknown exchange type 'invalid_exchange_type'
Channel disconnected
Connection disconnected