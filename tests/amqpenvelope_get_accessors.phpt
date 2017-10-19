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
$ex->setName('exchange-'.microtime(true));
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->declareExchange();
// Create a new queue
$q = new AMQPQueue($ch);
$q->setName('queue1' . microtime(true));
$q->declareQueue();
// Bind it on the exchange to routing.key
$q->bind($ex->getName(), 'routing.*');
// Publish a message to the exchange with a routing key
$ex->publish('message', 'routing.1', AMQP_NOPARAM, array('headers' => array('foo' => 'bar')));

// Read from the queue
$msg = $q->get();
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
  ["content_type":"AMQPBasicProperties":private]=>
  string(10) "text/plain"
  ["content_encoding":"AMQPBasicProperties":private]=>
  string(0) ""
  ["headers":"AMQPBasicProperties":private]=>
  array(1) {
    ["foo"]=>
    string(3) "bar"
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
  string(7) "message"
  ["consumer_tag":"AMQPEnvelope":private]=>
  string(0) ""
  ["delivery_tag":"AMQPEnvelope":private]=>
  int(1)
  ["is_redelivery":"AMQPEnvelope":private]=>
  bool(false)
  ["exchange_name":"AMQPEnvelope":private]=>
  string(%d) "exchange-%f"
  ["routing_key":"AMQPEnvelope":private]=>
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
        string(%d) "exchange-%f"
    isRedelivery:
        bool(false)
    getContentEncoding:
        string(0) ""
    getType:
        string(0) ""
    getTimeStamp:
        int(0)
    getPriority:
        int(0)
    getExpiration:
        string(0) ""
    getUserId:
        string(0) ""
    getAppId:
        string(0) ""
    getMessageId:
        string(0) ""
    getReplyTo:
        string(0) ""
    getCorrelationId:
        string(0) ""
    getHeaders:
        array(1) {
  ["foo"]=>
  string(3) "bar"
}
string(3) "bar"
string(3) "bar"
