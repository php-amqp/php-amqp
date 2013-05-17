--TEST--
AMQPEnvelope test get*() accessors
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();
$ch = new AMQPChannel($cnn);
// Declare a new exchange
$ex = new AMQPExchange($ch);
$ex->setName('exchange1');
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->declareExchange();
// Create a new queue
$q = new AMQPQueue($ch);
$q->setName('queue1' . time());
$q->declareQueue();
// Bind it on the exchange to routing.key
$q->bind($ex->getName(), 'routing.*');
// Publish a message to the exchange with a routing key
$ex->publish('message', 'routing.1');
function consumeThings($message, $queue) {
	var_dump($message->getBody());
	var_dump($message->getRoutingKey());
	var_dump($message->getDeliveryTag());
	var_dump($message->getDeliveryMode());
	var_dump($message->getExchangeName());
	var_dump($message->isRedelivery());
	var_dump($message->getContentType());
	var_dump($message->getContentEncoding());
	var_dump($message->getType());
	var_dump($message->getTimestamp());
	var_dump($message->getPriority());
	var_dump($message->getExpiration());
	var_dump($message->getUserId());
	var_dump($message->getAppId());
	var_dump($message->getMessageId());
	var_dump($message->getReplyTo());
	var_dump($message->getCorrelationId());
	var_dump($message->getHeaders());
	var_dump($message->getHeader("header"));

	return false;
}
// Read from the queue
$q->consume("consumeThings");
?>
--EXPECT--
string(7) "message"
string(9) "routing.1"
int(1)
int(0)
string(9) "exchange1"
bool(false)
string(10) "text/plain"
string(0) ""
string(0) ""
int(0)
int(0)
string(0) ""
string(0) ""
string(0) ""
string(0) ""
string(0) ""
string(0) ""
array(0) {
}
bool(false)
