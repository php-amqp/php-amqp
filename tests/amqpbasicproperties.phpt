--TEST--
AMQPBasicProperties
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
?>
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
}
AMQPBasicProperties
    getContentType():
        NULL
    getContentEncoding():
        NULL
    getHeaders():
        array(0) {
}
    getDeliveryMode():
        int(1)
    getPriority():
        int(0)
    getCorrelationId():
        NULL
    getReplyTo():
        NULL
    getExpiration():
        NULL
    getMessageId():
        NULL
    getTimestamp():
        NULL
    getType():
        NULL
    getUserId():
        NULL
    getAppId():
        NULL
    getClusterId():
        NULL

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
