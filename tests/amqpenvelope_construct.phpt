--TEST--
AMQPEnvelope construct
--SKIPIF--
<?php
if (!extension_loaded("amqp")) {
  print "skip";
}
--FILE--
<?php
var_dump(new AMQPEnvelope());
?>
--EXPECT--
object(AMQPEnvelope)#1 (20) {
  ["contentType":"AMQPBasicProperties":private]=>
  NULL
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
  NULL
  ["type":"AMQPBasicProperties":private]=>
  NULL
  ["userId":"AMQPBasicProperties":private]=>
  NULL
  ["appId":"AMQPBasicProperties":private]=>
  NULL
  ["clusterId":"AMQPBasicProperties":private]=>
  NULL
  ["body":"AMQPEnvelope":private]=>
  string(0) ""
  ["consumerTag":"AMQPEnvelope":private]=>
  NULL
  ["deliveryTag":"AMQPEnvelope":private]=>
  NULL
  ["isRedelivery":"AMQPEnvelope":private]=>
  bool(false)
  ["exchangeName":"AMQPEnvelope":private]=>
  NULL
  ["routingKey":"AMQPEnvelope":private]=>
  string(0) ""
}
