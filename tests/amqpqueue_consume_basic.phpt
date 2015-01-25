--TEST--
AMQPQueue::consume basic
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();

$ch = new AMQPChannel($cnn);

// Declare a new exchange
$ex = new AMQPExchange($ch);
$ex->setName('exchange-' . microtime(true));
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->declareExchange();

// Create a new queue
$q = new AMQPQueue($ch);
$q->setName('queue-' . microtime(true));
$q->declareQueue();

// Bind it on the exchange to routing.key
$q->bind($ex->getName(), 'routing.*');

// Publish a message to the exchange with a routing key
$ex->publish('message1', 'routing.1', AMQP_NOPARAM, array('content_type' => 'plain/test', 'headers' => array('foo' => 'bar')));
$ex->publish('message2', 'routing.2', AMQP_NOPARAM, array('delivery_mode' => AMQP_DURABLE));
$ex->publish('message3', 'routing.3', AMQP_DURABLE); // this is wrong way to make messages persistent

$count = 0;

function dump_message($msg) {
    if (!$msg) {
        var_dump($msg);
        return;
    }

    echo get_class($msg), PHP_EOL;
    echo "    getBody:", PHP_EOL, "        ";
    var_dump($msg->getBody());
    echo "    getContentType:", PHP_EOL, "        ";
    var_dump($msg->getContentType());
    echo "    getRoutingKey:", PHP_EOL, "        ";
    var_dump($msg->getRoutingKey());
    echo "    getDeliveryTag:", PHP_EOL, "        ";
    var_dump($msg->getDeliveryTag());
    echo "    getDeliveryMode:", PHP_EOL, "        ";
    var_dump($msg->getDeliveryMode());
    echo "    getExchangeName:", PHP_EOL, "        ";
    var_dump($msg->getExchangeName());
    echo "    isRedelivery:", PHP_EOL, "        ";
    var_dump($msg->isRedelivery());
    echo "    getContentEncoding:", PHP_EOL, "        ";
    var_dump($msg->getContentEncoding());
    echo "    getType:", PHP_EOL, "        ";
    var_dump($msg->getType());
    echo "    getTimeStamp:", PHP_EOL, "        ";
    var_dump($msg->getTimeStamp());
    echo "    getPriority:", PHP_EOL, "        ";
    var_dump($msg->getPriority());
    echo "    getExpiration:", PHP_EOL, "        ";
    var_dump($msg->getExpiration());
    echo "    getUserId:", PHP_EOL, "        ";
    var_dump($msg->getUserId());
    echo "    getAppId:", PHP_EOL, "        ";
    var_dump($msg->getAppId());
    echo "    getMessageId:", PHP_EOL, "        ";
    var_dump($msg->getMessageId());
    echo "    getReplyTo:", PHP_EOL, "        ";
    var_dump($msg->getReplyTo());
    echo "    getCorrelationId:", PHP_EOL, "        ";
    var_dump($msg->getCorrelationId());
    echo "    getHeaders:", PHP_EOL, "        ";
    var_dump($msg->getHeaders());
}

function consumeThings($message, $queue) {
	global $count;

    echo "call #$count", PHP_EOL;
    // Read from the queue
    dump_message($message);
    echo PHP_EOL;
	$count++;

	if ($count >= 2) {
		return false;
	}

	return true;
}

// Read from the queue
$q->consume("consumeThings", AMQP_AUTOACK);

$q->delete();
$ex->delete();

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
    getDeliveryTag:
        int(1)
    getDeliveryMode:
        int(0)
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

call #1
AMQPEnvelope
    getBody:
        string(8) "message2"
    getContentType:
        string(10) "text/plain"
    getRoutingKey:
        string(9) "routing.2"
    getDeliveryTag:
        int(2)
    getDeliveryMode:
        int(2)
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
        array(0) {
}