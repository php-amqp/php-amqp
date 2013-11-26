--TEST--
AMQPExchange
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();

$ch = new AMQPChannel($cnn);

$exchangge_name = "exchange-" . microtime(true);

$ex = new AMQPExchange($ch);
$ex->setName($exchangge_name);
$ex->setType(AMQP_EX_TYPE_FANOUT);
echo "Exchange declared: ", $ex->declareExchange() ? "true" : "false", PHP_EOL;

try {
    $ex = new AMQPExchange($ch);
    $ex->setName($exchangge_name);
    $ex->setType(AMQP_EX_TYPE_TOPIC);
    echo "Exchange declared: ", $ex->declareExchange() ? "true" : "false", PHP_EOL;
} catch (AMQPExchangeException $e) {
    echo $e->getMessage(), PHP_EOL;
}

echo "Channel connected: ", $ch->isConnected() ? "true" : "false", PHP_EOL;
echo "Connection connected: ", $cnn->isConnected() ? "true" : "false", PHP_EOL;

?>
--EXPECTF--
Exchange declared: true
Server channel error: 406, message: PRECONDITION_FAILED - cannot redeclare exchange 'exchange-%d.%d' in vhost '/' with different type, durable, internal or autodelete value
Channel connected: false
Connection connected: true
