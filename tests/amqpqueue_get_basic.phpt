--TEST--
AMQPQueue::get basic
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip";
if (!getenv("PHP_AMQP_HOST")) print "skip";
?>
--FILE--
<?php
require '_test_helpers.php.inc';

$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->connect();

$ch = new AMQPChannel($cnn);

// Declare a new exchange
$ex = new AMQPExchange($ch);
$ex->setName('exchange-'. bin2hex(random_bytes(32)));
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->declareExchange();

// Create a new queue
$q = new AMQPQueue($ch);
$q->setName('queue-' . bin2hex(random_bytes(32)));
$q->declareQueue();

// Bind it on the exchange to routing.key
$q->bind($ex->getName(), 'routing.*');
// Publish a message to the exchange with a routing key
$ex->publish('message1', 'routing.1', AMQP_NOPARAM, array('content_type' => 'plain/test', 'headers' => array('foo' => 'bar')));
$ex->publish('message2', 'routing.2', AMQP_DURABLE);
$ex->publish('message3', 'routing.3');

for ($i = 0; $i < 4; $i++) {
    echo "call #$i", PHP_EOL;
	// Read from the queue
	$msg = $q->get(AMQP_AUTOACK);
    dump_message($msg);
    echo PHP_EOL;
}

?>
--EXPECTF--
call #0
AMQPEnvelope
    getBody:
        string(8) "message1"
    getContentType:
        string(10) "plain/test"
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

call #1
AMQPEnvelope
    getBody:
        string(8) "message2"
    getContentType:
        string(10) "text/plain"
    getRoutingKey:
        string(9) "routing.2"
    getConsumerTag:
        string(0) ""
    getDeliveryTag:
        int(2)
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
        array(0) {
}

call #2
AMQPEnvelope
    getBody:
        string(8) "message3"
    getContentType:
        string(10) "text/plain"
    getRoutingKey:
        string(9) "routing.3"
    getConsumerTag:
        string(0) ""
    getDeliveryTag:
        int(3)
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
        array(0) {
}

call #3
NULL
