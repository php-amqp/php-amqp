--TEST--
AMQPEnvelope var_dump
--SKIPIF--
<?php
if (!extension_loaded("amqp")) {
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
  ["contentType":"AMQPBasicProperties":private]=>
  string(10) "text/plain"
  ["contentEncoding":"AMQPBasicProperties":private]=>
  string(0) ""
  ["headers":"AMQPBasicProperties":private]=>
  array(0) {
  }
  ["deliveryMode":"AMQPBasicProperties":private]=>
  int(1)
  ["priority":"AMQPBasicProperties":private]=>
  int(0)
  ["correlationId":"AMQPBasicProperties":private]=>
  string(0) ""
  ["replyTo":"AMQPBasicProperties":private]=>
  string(0) ""
  ["expiration":"AMQPBasicProperties":private]=>
  string(0) ""
  ["messageId":"AMQPBasicProperties":private]=>
  string(0) ""
  ["timestamp":"AMQPBasicProperties":private]=>
  int(0)
  ["type":"AMQPBasicProperties":private]=>
  string(0) ""
  ["userId":"AMQPBasicProperties":private]=>
  string(0) ""
  ["appId":"AMQPBasicProperties":private]=>
  string(0) ""
  ["clusterId":"AMQPBasicProperties":private]=>
  string(0) ""
  ["body":"AMQPEnvelope":private]=>
  string(7) "message"
  ["consumerTag":"AMQPEnvelope":private]=>
  string(31) "amq.ctag-%s"
  ["deliveryTag":"AMQPEnvelope":private]=>
  int(1)
  ["isRedelivery":"AMQPEnvelope":private]=>
  bool(false)
  ["exchangeName":"AMQPEnvelope":private]=>
  string(9) "exchange1"
  ["routingKey":"AMQPEnvelope":private]=>
  string(9) "routing.1"
}
object(AMQPEnvelope)#5 (20) {
  ["contentType":"AMQPBasicProperties":private]=>
  string(10) "text/plain"
  ["contentEncoding":"AMQPBasicProperties":private]=>
  string(0) ""
  ["headers":"AMQPBasicProperties":private]=>
  array(1) {
    ["test"]=>
    string(6) "passed"
  }
  ["deliveryMode":"AMQPBasicProperties":private]=>
  int(1)
  ["priority":"AMQPBasicProperties":private]=>
  int(0)
  ["correlationId":"AMQPBasicProperties":private]=>
  string(0) ""
  ["replyTo":"AMQPBasicProperties":private]=>
  string(0) ""
  ["expiration":"AMQPBasicProperties":private]=>
  string(0) ""
  ["messageId":"AMQPBasicProperties":private]=>
  string(0) ""
  ["timestamp":"AMQPBasicProperties":private]=>
  int(0)
  ["type":"AMQPBasicProperties":private]=>
  string(0) ""
  ["userId":"AMQPBasicProperties":private]=>
  string(0) ""
  ["appId":"AMQPBasicProperties":private]=>
  string(0) ""
  ["clusterId":"AMQPBasicProperties":private]=>
  string(0) ""
  ["body":"AMQPEnvelope":private]=>
  string(7) "message"
  ["consumerTag":"AMQPEnvelope":private]=>
  string(31) "amq.ctag-%s"
  ["deliveryTag":"AMQPEnvelope":private]=>
  int(2)
  ["isRedelivery":"AMQPEnvelope":private]=>
  bool(false)
  ["exchangeName":"AMQPEnvelope":private]=>
  string(9) "exchange1"
  ["routingKey":"AMQPEnvelope":private]=>
  string(9) "routing.1"
}
