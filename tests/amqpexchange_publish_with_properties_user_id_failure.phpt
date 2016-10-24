--TEST--
AMQPExchange publish with properties - user_id failure
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();

$ch = new AMQPChannel($cnn);

$ex = new AMQPExchange($ch);
$ex->setName("exchange-" . microtime(true));
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->declareExchange();

echo "Channel ", $ch->isConnected() ? 'connected' : 'disconnected', PHP_EOL;
echo "Connection ", $cnn->isConnected() ? 'connected' : 'disconnected', PHP_EOL;

try {
    // NOTE: basic.publish is asynchronous, so ...
    echo $ex->publish('message', 'routing.key', AMQP_NOPARAM, array('user_id' => 'unknown-' . microtime(true))) ? 'true' : 'false', PHP_EOL;
} catch (AMQPException $e) {
    echo get_class($e), "({$e->getCode()}): ", $e->getMessage(), PHP_EOL;
}

echo "Channel ", $ch->isConnected() ? 'connected' : 'disconnected', PHP_EOL;
echo "Connection ", $cnn->isConnected() ? 'connected' : 'disconnected', PHP_EOL;

try {
    // NOTE: ... the next socket (not only channel) operation will fail, which may lead to very strange issues
    //       if we operate here,on different entity, for example on queue.

    $q = new AMQPQueue($ch);
    $q->declareQueue();

    echo "Queue declared", PHP_EOL;
} catch (AMQPException $e) {
    echo get_class($e), "({$e->getCode()}): ", $e->getMessage(), PHP_EOL;
}

echo "Channel ", $ch->isConnected() ? 'connected' : 'disconnected', PHP_EOL;
echo "Connection ", $cnn->isConnected() ? 'connected' : 'disconnected', PHP_EOL;

?>
--EXPECTF--
Channel connected
Connection connected
true
Channel connected
Connection connected
AMQPQueueException(406): Server channel error: 406, message: PRECONDITION_FAILED - user_id property set to 'unknown-%f' but authenticated user was 'guest'
Channel disconnected
Connection connected