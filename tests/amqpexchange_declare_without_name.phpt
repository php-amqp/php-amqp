--TEST--
AMQPExchange::declareExchange() without name set
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();

$ch = new AMQPChannel($cnn);


$ex = new AMQPExchange($ch);
$ex->setType(AMQP_EX_TYPE_FANOUT);

try {
    $ex->declareExchange();
    echo 'Exchange declared', PHP_EOL;
} catch (AMQPException $e) {
    echo get_class($e), ': ', $e->getMessage(), PHP_EOL;
}
?>
--EXPECTF--
AMQPExchangeException: Could not declare exchange. Exchanges must have a name.
