--TEST--
AMQPBasicProperties
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
require '_test_helpers.php.inc';

$props = new AMQPBasicProperties();
var_dump($props);
dump_methods($props);

echo PHP_EOL;

$props = new AMQPBasicProperties(
    "content_type",
    "content_encoding",
    array('test' => 'headers'),
    42,
    24,
    "correlation_id",
    "reply_to",
    "expiration",
    "message_id",
    99999,
    "type",
    "user_id",
    "app_id",
    "cluster_id"
);
var_dump($props);
dump_methods($props);


?>
--EXPECT--
object(AMQPBasicProperties)#1 (14) {
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
}
AMQPBasicProperties
    getContentType():
        string(0) ""
    getContentEncoding():
        string(0) ""
    getHeaders():
        array(0) {
}
    getDeliveryMode():
        int(1)
    getPriority():
        int(0)
    getCorrelationId():
        string(0) ""
    getReplyTo():
        string(0) ""
    getExpiration():
        string(0) ""
    getMessageId():
        string(0) ""
    getTimestamp():
        int(0)
    getType():
        string(0) ""
    getUserId():
        string(0) ""
    getAppId():
        string(0) ""
    getClusterId():
        string(0) ""

object(AMQPBasicProperties)#2 (14) {
  ["contentType":"AMQPBasicProperties":private]=>
  string(12) "content_type"
  ["contentEncoding":"AMQPBasicProperties":private]=>
  string(16) "content_encoding"
  ["headers":"AMQPBasicProperties":private]=>
  array(1) {
    ["test"]=>
    string(7) "headers"
  }
  ["deliveryMode":"AMQPBasicProperties":private]=>
  int(42)
  ["priority":"AMQPBasicProperties":private]=>
  int(24)
  ["correlationId":"AMQPBasicProperties":private]=>
  string(14) "correlation_id"
  ["replyTo":"AMQPBasicProperties":private]=>
  string(8) "reply_to"
  ["expiration":"AMQPBasicProperties":private]=>
  string(10) "expiration"
  ["messageId":"AMQPBasicProperties":private]=>
  string(10) "message_id"
  ["timestamp":"AMQPBasicProperties":private]=>
  int(99999)
  ["type":"AMQPBasicProperties":private]=>
  string(4) "type"
  ["userId":"AMQPBasicProperties":private]=>
  string(7) "user_id"
  ["appId":"AMQPBasicProperties":private]=>
  string(6) "app_id"
  ["clusterId":"AMQPBasicProperties":private]=>
  string(10) "cluster_id"
}
AMQPBasicProperties
    getContentType():
        string(12) "content_type"
    getContentEncoding():
        string(16) "content_encoding"
    getHeaders():
        array(1) {
  ["test"]=>
  string(7) "headers"
}
    getDeliveryMode():
        int(42)
    getPriority():
        int(24)
    getCorrelationId():
        string(14) "correlation_id"
    getReplyTo():
        string(8) "reply_to"
    getExpiration():
        string(10) "expiration"
    getMessageId():
        string(10) "message_id"
    getTimestamp():
        int(99999)
    getType():
        string(4) "type"
    getUserId():
        string(7) "user_id"
    getAppId():
        string(6) "app_id"
    getClusterId():
        string(10) "cluster_id"
