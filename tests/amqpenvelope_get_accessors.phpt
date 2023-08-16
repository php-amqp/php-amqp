--TEST--
AMQPEnvelope test get*() accessors
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
require '_test_helpers.php.inc';

$cnn = new AMQPConnection();
$cnn->connect();
$ch = new AMQPChannel($cnn);
// Declare a new exchange
$ex = new AMQPExchange($ch);
$ex->setName('exchange-'. bin2hex(random_bytes(32)));
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->declareExchange();
// Create a new queue
$q = new AMQPQueue($ch);
$q->setName('queue1' . bin2hex(random_bytes(32)));
$q->declareQueue();
// Bind it on the exchange to routing.key
$q->bind($ex->getName(), 'routing.*');
// Publish a message to the exchange with a routing key
$ex->publish('message', 'routing.1', null, array('headers' => array('foo' => 'bar')));

// Read from the queue
$msg = $q->get(null);
var_dump($msg);
dump_message($msg);

$header = $msg->getHeader('foo');
var_dump($header);
$header = 'changed';
$header = $msg->getHeader('foo');
var_dump($header);

?>
--EXPECTF--
object(AMQPEnvelope)#5 (20) {
  ["contentType":"AMQPBasicProperties":private]=>
  string(10) "text/plain"
  ["contentEncoding":"AMQPBasicProperties":private]=>
  NULL
  ["headers":"AMQPBasicProperties":private]=>
  array(1) {
    ["foo"]=>
    string(3) "bar"
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
  string(0) ""
  ["deliveryTag":"AMQPEnvelope":private]=>
  int(1)
  ["isRedelivery":"AMQPEnvelope":private]=>
  bool(false)
  ["exchangeName":"AMQPEnvelope":private]=>
  string(%d) "exchange-%s"
  ["routingKey":"AMQPEnvelope":private]=>
  string(9) "routing.1"
}
AMQPEnvelope
    getBody:
        string(7) "message"
    getContentType:
        string(10) "text/plain"
    getRoutingKey:
        string(9) "routing.1"
    getConsumerTag:
        string(0) ""
    getDeliveryTag:
        int(1)
    getDeliveryMode:
        int(1)
    getExchangeName:
        string(%d) "exchange-%s"
    isRedelivery:
        bool(false)
    getContentEncoding:
        NULL
    getType:
        NULL
    getTimeStamp:
        int(0)
    getPriority:
        int(0)
    getExpiration:
        NULL
    getUserId:
        NULL
    getAppId:
        NULL
    getMessageId:
        NULL
    getReplyTo:
        NULL
    getCorrelationId:
        NULL
    getHeaders:
        array(1) {
  ["foo"]=>
  string(3) "bar"
}
string(3) "bar"
string(3) "bar"
