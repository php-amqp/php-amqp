--TEST--
AMQPQueue::nack
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip";
if (!getenv("PHP_AMQP_HOST")) print "skip";
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->connect();

$ch = new AMQPChannel($cnn);

$ex = new AMQPExchange($ch);
$ex->setName('testrecover-' . bin2hex(random_bytes(32)));
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->declareExchange();
$exchangeName = $ex->getName();

$q = new AMQPQueue($ch);
$q->setName('testrecover-' . bin2hex(random_bytes(32)));
$q->declareQueue();
$q->bind($exchangeName);

$ex->publish('message');

function inspectMessage(AMQPQueue $q) {
    $msg = $q->get();
    echo $msg->getBody(), PHP_EOL;
    echo $msg->isRedelivery() ? 'true' : 'false';
    echo PHP_EOL;
}

inspectMessage($q);
$q->recover();

inspectMessage($q);
$q->recover(true);

inspectMessage($q);

try {
    $q->recover(false);
} catch (AMQPConnectionException $e) {
    echo $e->getMessage(), PHP_EOL;
}
--EXPECT--
message
false
message
true
message
true
Server connection error: 540, message: NOT_IMPLEMENTED - requeue=false
