--TEST--
AMQPEnvelope var_dump
--SKIPIF--
<?php
if (!extension_loaded("amqp") || version_compare(PHP_VERSION, '5.3', '<')) {
  print "skip";
}
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
	var_dump($message);
	return false;
}
// Read from the queue
$q->consume("consumeThings");
?>
--EXPECT--
object(AMQPEnvelope)#5 (18) {
  ["body"]=>
  string(7) "message"
  ["content_type"]=>
  string(10) "text/plain"
  ["routing_key"]=>
  string(9) "routing.1"
  ["delivery_tag"]=>
  int(1)
  ["delivery_mode"]=>
  int(0)
  ["exchange_name"]=>
  string(9) "exchange1"
  ["is_redelivery"]=>
  int(0)
  ["content_encoding"]=>
  string(0) ""
  ["type"]=>
  string(0) ""
  ["timestamp"]=>
  int(0)
  ["priority"]=>
  int(0)
  ["expiration"]=>
  string(0) ""
  ["user_id"]=>
  string(0) ""
  ["app_id"]=>
  string(0) ""
  ["message_id"]=>
  string(0) ""
  ["reply_to"]=>
  string(0) ""
  ["correlation_id"]=>
  string(0) ""
  ["headers"]=>
  array(0) {
  }
}
