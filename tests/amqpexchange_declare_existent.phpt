--TEST--
AMQPExchange
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();

$ch = new AMQPChannel($cnn);

echo 'Channel id: ', $ch->getChannelId(), PHP_EOL;

$exchangge_name = "exchange-" . microtime(true);

$ex = new AMQPExchange($ch);
$ex->setName($exchangge_name);
$ex->setType(AMQP_EX_TYPE_FANOUT);
echo "Exchange declared: ", $ex->declareExchange() ? "true" : "false", PHP_EOL;

try {
    $ex = new AMQPExchange($ch);
    $ex->setName($exchangge_name);
    $ex->setType(AMQP_EX_TYPE_TOPIC);
    $ex->declareExchange();
} catch (AMQPExchangeException $e) {
    echo $e->getMessage(), PHP_EOL;
}

echo "Channel connected: ", $ch->isConnected() ? "true" : "false", PHP_EOL;
echo "Connection connected: ", $cnn->isConnected() ? "true" : "false", PHP_EOL;

try {
    $ex = new AMQPExchange($ch);
} catch (AMQPChannelException $e) {
    echo $e->getMessage(), PHP_EOL;
}
?>
--EXPECTF--
Channel id: %d
Exchange declared: true
Server channel error: 406, message: PRECONDITION_FAILED - %s exchange 'exchange-%d.%d' in vhost '/'%s
Channel connected: false
Connection connected: true
Could not create exchange. No channel available.
