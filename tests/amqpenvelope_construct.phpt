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
  string(0) ""
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
  string(0) ""
  ["consumerTag":"AMQPEnvelope":private]=>
  NULL
  ["deliveryTag":"AMQPEnvelope":private]=>
  NULL
  ["isRedelivery":"AMQPEnvelope":private]=>
  NULL
  ["exchangeName":"AMQPEnvelope":private]=>
  NULL
  ["routingKey":"AMQPEnvelope":private]=>
  string(0) ""
}
