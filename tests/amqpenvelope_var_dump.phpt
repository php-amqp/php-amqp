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
  NULL
  ["headers":"AMQPBasicProperties":private]=>
  array(0) {
  }
  ["deliveryMode":"AMQPBasicProperties":private]=>
  int(1)
  ["priority":"AMQPBasicProperties":private]=>
  int(0)
  ["correlationId":"AMQPBasicProperties":private]=>
  NULL
  ["replyTo":"AMQPBasicProperties":private]=>
  NULL
  ["expiration":"AMQPBasicProperties":private]=>
  NULL
  ["messageId":"AMQPBasicProperties":private]=>
  NULL
  ["timestamp":"AMQPBasicProperties":private]=>
  int(0)
  ["type":"AMQPBasicProperties":private]=>
  NULL
  ["userId":"AMQPBasicProperties":private]=>
  NULL
  ["appId":"AMQPBasicProperties":private]=>
  NULL
  ["clusterId":"AMQPBasicProperties":private]=>
  NULL
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
  NULL
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
  NULL
  ["replyTo":"AMQPBasicProperties":private]=>
  NULL
  ["expiration":"AMQPBasicProperties":private]=>
  NULL
  ["messageId":"AMQPBasicProperties":private]=>
  NULL
  ["timestamp":"AMQPBasicProperties":private]=>
  int(0)
  ["type":"AMQPBasicProperties":private]=>
  NULL
  ["userId":"AMQPBasicProperties":private]=>
  NULL
  ["appId":"AMQPBasicProperties":private]=>
  NULL
  ["clusterId":"AMQPBasicProperties":private]=>
  NULL
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
