--TEST--
AMQPEnvelope var_dump
--SKIPIF--
<?php
if (!extension_loaded("amqp") || version_compare(PHP_VERSION, '5.3', '<')) {
  print "skip";
}
--FILE--
<?php
require '_test_helpers.php.inc';

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
$q->setName('queue1' . microtime(true));
$q->declareQueue();
// Bind it on the exchange to routing.key
$q->bind($ex->getName(), 'routing.*');
// Publish a message to the exchange with a routing key
$ex->publish('message', 'routing.1');
$ex->publish('message', 'routing.1', AMQP_NOPARAM, array("headers" => array("test" => "passed")));

// Read from the queue
$q->consume("consumeThings");
$q->consume("consumeThings");
?>
--EXPECTF--
object(AMQPEnvelope)#5 (20) {
  ["content_type":"AMQPBasicProperties":private]=>
  string(10) "text/plain"
  ["content_encoding":"AMQPBasicProperties":private]=>
  string(0) ""
  ["headers":"AMQPBasicProperties":private]=>
  array(0) {
  }
  ["delivery_mode":"AMQPBasicProperties":private]=>
  int(1)
  ["priority":"AMQPBasicProperties":private]=>
  int(0)
  ["correlation_id":"AMQPBasicProperties":private]=>
  string(0) ""
  ["reply_to":"AMQPBasicProperties":private]=>
  string(0) ""
  ["expiration":"AMQPBasicProperties":private]=>
  string(0) ""
  ["message_id":"AMQPBasicProperties":private]=>
  string(0) ""
  ["timestamp":"AMQPBasicProperties":private]=>
  int(0)
  ["type":"AMQPBasicProperties":private]=>
  string(0) ""
  ["user_id":"AMQPBasicProperties":private]=>
  string(0) ""
  ["app_id":"AMQPBasicProperties":private]=>
  string(0) ""
  ["cluster_id":"AMQPBasicProperties":private]=>
  string(0) ""
  ["body":"AMQPEnvelope":private]=>
  string(7) "message"
  ["consumer_tag":"AMQPEnvelope":private]=>
  string(31) "amq.ctag-%s"
  ["delivery_tag":"AMQPEnvelope":private]=>
  int(1)
  ["is_redelivery":"AMQPEnvelope":private]=>
  bool(false)
  ["exchange_name":"AMQPEnvelope":private]=>
  string(9) "exchange1"
  ["routing_key":"AMQPEnvelope":private]=>
  string(9) "routing.1"
}
object(AMQPEnvelope)#5 (20) {
  ["content_type":"AMQPBasicProperties":private]=>
  string(10) "text/plain"
  ["content_encoding":"AMQPBasicProperties":private]=>
  string(0) ""
  ["headers":"AMQPBasicProperties":private]=>
  array(1) {
    ["test"]=>
    string(6) "passed"
  }
  ["delivery_mode":"AMQPBasicProperties":private]=>
  int(1)
  ["priority":"AMQPBasicProperties":private]=>
  int(0)
  ["correlation_id":"AMQPBasicProperties":private]=>
  string(0) ""
  ["reply_to":"AMQPBasicProperties":private]=>
  string(0) ""
  ["expiration":"AMQPBasicProperties":private]=>
  string(0) ""
  ["message_id":"AMQPBasicProperties":private]=>
  string(0) ""
  ["timestamp":"AMQPBasicProperties":private]=>
  int(0)
  ["type":"AMQPBasicProperties":private]=>
  string(0) ""
  ["user_id":"AMQPBasicProperties":private]=>
  string(0) ""
  ["app_id":"AMQPBasicProperties":private]=>
  string(0) ""
  ["cluster_id":"AMQPBasicProperties":private]=>
  string(0) ""
  ["body":"AMQPEnvelope":private]=>
  string(7) "message"
  ["consumer_tag":"AMQPEnvelope":private]=>
  string(31) "amq.ctag-%s"
  ["delivery_tag":"AMQPEnvelope":private]=>
  int(2)
  ["is_redelivery":"AMQPEnvelope":private]=>
  bool(false)
  ["exchange_name":"AMQPEnvelope":private]=>
  string(9) "exchange1"
  ["routing_key":"AMQPEnvelope":private]=>
  string(9) "routing.1"
}
