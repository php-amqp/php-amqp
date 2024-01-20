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
    null,
    null,
    array('test' => 'headers'),
    42,
    24,
    null,
    null,
    null,
    null,
    null,
    null,
    null,
    null,
    null
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
  NULL
  ["contentEncoding":"AMQPBasicProperties":private]=>
  NULL
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
        array(1) {
  ["test"]=>
  string(7) "headers"
}
    getDeliveryMode():
        int(42)
    getPriority():
        int(24)
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
