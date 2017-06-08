--TEST--
AMQPEnvelope construct
--SKIPIF--
<?php
if (!extension_loaded("amqp") || version_compare(PHP_VERSION, '5.3', '<')) {
  print "skip";
}
--FILE--
<?php
var_dump(new AMQPEnvelope());
?>
--EXPECT--
object(AMQPEnvelope)#1 (20) {
  ["content_type":"AMQPBasicProperties":private]=>
  string(0) ""
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
  NULL
  ["consumer_tag":"AMQPEnvelope":private]=>
  NULL
  ["delivery_tag":"AMQPEnvelope":private]=>
  NULL
  ["is_redelivery":"AMQPEnvelope":private]=>
  NULL
  ["exchange_name":"AMQPEnvelope":private]=>
  NULL
  ["routing_key":"AMQPEnvelope":private]=>
  NULL
}
