--TEST--
AMQPQueue::consume basic
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();

$ch = new AMQPChannel($cnn);

// Declare a new exchange
$ex = new AMQPExchange($ch);
$ex->setName('exchange-' . microtime(true));
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->declareExchange();

// Create a new queue
$q = new AMQPQueue($ch);
$q->setName('queue-' . microtime(true));
$q->declareQueue();

// Bind it on the exchange to routing.key
$q->bind($ex->getName(), 'routing.*');

// Publish a message to the exchange with a routing key
$ex->publish('message1', 'routing.1', AMQP_NOPARAM, array('content_type' => 'plain/test', 'headers' => array('foo' => 'bar')));
$ex->publish('message2', 'routing.2', AMQP_NOPARAM, array('delivery_mode' => AMQP_DURABLE));
$ex->publish('message3', 'routing.3', AMQP_DURABLE); // this is wrong way to make messages persistent

$count = 0;

function dump_message(AMQPEnvelope $msg) {
  echo get_class($msg), PHP_EOL;
  var_dump($msg->getBody());
  var_dump($msg->getContentType());
  var_dump($msg->getRoutingKey());
  var_dump($msg->getDeliveryTag());
  var_dump($msg->getDeliveryMode());
  var_dump($msg->getExchangeName());
  var_dump($msg->isRedelivery());
  var_dump($msg->getContentEncoding());
  var_dump($msg->getType());
  var_dump($msg->getTimeStamp());
  var_dump($msg->getPriority());
  var_dump($msg->getExpiration());
  var_dump($msg->getUserId());
  var_dump($msg->getAppId());
  var_dump($msg->getMessageId());
  var_dump($msg->getReplyTo());
  var_dump($msg->getCorrelationId());
  var_dump($msg->getHeaders());
}

function consumeThings($message, $queue) {
	global $count;

    echo "call #$count", PHP_EOL;
    // Read from the queue
    dump_message($message);
    echo PHP_EOL;
	$count++;

	if ($count >= 2) {
		return false;
	}

	return true;
}

// Read from the queue
$q->consume("consumeThings", AMQP_AUTOACK);

$q->delete();
$ex->delete();

?>
--EXPECTF--
call #0
AMQPEnvelope
string(8) "message1"
string(10) "plain/test"
string(9) "routing.1"
int(1)
int(0)
string(%d) "exchange-%f"
bool(false)
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
array(1) {
  ["foo"]=>
  string(3) "bar"
}

call #1
AMQPEnvelope
string(8) "message2"
string(10) "text/plain"
string(9) "routing.2"
int(2)
int(2)
string(%d) "exchange-%f"
bool(false)
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
