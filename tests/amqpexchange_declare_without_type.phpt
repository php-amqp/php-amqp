--TEST--
AMQPExchange::declareExchange() without type set
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

$exchangge_name = "exchange-" . bin2hex(random_bytes(32));

$ex = new AMQPExchange($ch);
$ex->setName($exchangge_name);

try {
    $ex->declareExchange();
    echo 'Exchange declared', PHP_EOL;
} catch (AMQPException $e) {
    echo get_class($e), "({$e->getCode()}): ", $e->getMessage(), PHP_EOL;
}
?>
--EXPECTF--
AMQPExchangeException(0): Could not declare exchange. Exchanges must have a type.
