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
object(AMQPEnvelope)#1 (18) {
  ["body":"AMQPEnvelope":private]=>
  string(0) ""
  ["delivery_tag":"AMQPEnvelope":private]=>
  int(0)
  ["is_redelivery":"AMQPEnvelope":private]=>
  bool(false)
  ["exchange_name":"AMQPEnvelope":private]=>
  string(0) ""
  ["routing_key":"AMQPEnvelope":private]=>
  string(0) ""
  ["content_type":"AMQPEnvelope":private]=>
  string(0) ""
  ["content_encoding":"AMQPEnvelope":private]=>
  string(0) ""
  ["headers":"AMQPEnvelope":private]=>
  array(0) {
  }
  ["delivery_mode":"AMQPEnvelope":private]=>
  int(1)
  ["priority":"AMQPEnvelope":private]=>
  int(0)
  ["correlation_id":"AMQPEnvelope":private]=>
  string(0) ""
  ["reply_to":"AMQPEnvelope":private]=>
  string(0) ""
  ["expiration":"AMQPEnvelope":private]=>
  string(0) ""
  ["message_id":"AMQPEnvelope":private]=>
  string(0) ""
  ["timestamp":"AMQPEnvelope":private]=>
  int(0)
  ["type":"AMQPEnvelope":private]=>
  string(0) ""
  ["user_id":"AMQPEnvelope":private]=>
  string(0) ""
  ["app_id":"AMQPEnvelope":private]=>
  string(0) ""
}