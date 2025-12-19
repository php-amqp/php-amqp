--TEST--
AMQPExchange
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

$ch = new AMQPChannel($cnn);

$ex = new AMQPExchange($ch);
$ex->setName("exchange-" . bin2hex(random_bytes(32)));
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
--EXPECTF--
Channel connected
Connection connected
AMQPExchangeException(%d): %s - unknown exchange type 'invalid_exchange_type'
Channel disconnected
Connection connected
