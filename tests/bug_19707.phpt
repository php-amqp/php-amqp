--TEST--
AMQPQueue::get() doesn't return the message
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();

$ch = new AMQPChannel($cnn);

$ex = new AMQPExchange($ch);
$ex->setName("exchange_testing_19707");
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->declareExchange();

$q = new AMQPQueue($ch);
$q->setName('queue' . bin2hex(random_bytes(32)));
$q->setFlags(AMQP_DURABLE);
$q->declareQueue();

$q->bind($ex->getName(), 'routing.key');

$ex->publish('message', 'routing.key');

$msg = $q->get();

echo "message received from get:\n";
$funcs = array(
  'getAppId', 'getBody', 'getContentEncoding', 'getContentType',
  'getCorrelationId', 'getDeliveryTag', 'getExchangeName', 'getExpiration',
  'getHeaders', 'getMessageId', 'getPriority', 'getReplyTo', 'getRoutingKey',
  'getTimeStamp', 'getType', 'getUserId', 'isRedelivery'
);
foreach ($funcs as $func) {
  printf("%s => %s\n", $func, var_export($msg->$func(), true));
};

$q->delete();
$ex->delete();
?>
--EXPECT--
message received from get:
getAppId => NULL
getBody => 'message'
getContentEncoding => NULL
getContentType => 'text/plain'
getCorrelationId => NULL
getDeliveryTag => 1
getExchangeName => 'exchange_testing_19707'
getExpiration => NULL
getHeaders => array (
)
getMessageId => NULL
getPriority => 0
getReplyTo => NULL
getRoutingKey => 'routing.key'
getTimeStamp => 0
getType => NULL
getUserId => NULL
isRedelivery => false