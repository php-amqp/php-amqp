--TEST--
AMQPExchange::delete()
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();

$ch = new AMQPChannel($cnn);

echo 'Channel id: ', $ch->getChannelId(), PHP_EOL;

$ch = new AMQPChannel($cnn);
$ex = new AMQPExchange($ch);

try {
    $ex->delete();
} catch (AMQPExchangeException $e) {
    echo get_class($e), "({$e->getCode()}): ", $e->getMessage(), PHP_EOL;
}

echo "Channel connected: ", $ch->isConnected() ? "true" : "false", PHP_EOL;
echo "Connection connected: ", $cnn->isConnected() ? "true" : "false", PHP_EOL;
?>
--EXPECT--
Channel id: 1
AMQPExchangeException(403): Server channel error: 403, message: ACCESS_REFUSED - operation not permitted on the default exchange
Channel connected: false
Connection connected: true
