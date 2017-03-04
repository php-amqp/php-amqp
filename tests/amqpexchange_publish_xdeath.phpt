--TEST--
AMQPExchange publish with properties - nested header values
--SKIPIF--
<?php if (!extension_loaded("amqp")) {
    print "skip";
} ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();

$ch = new AMQPChannel($cnn);

$suffix = sha1(microtime(true));

$dlx = new AMQPExchange($ch);
$dlx->setName('dlx-' . $suffix);
$dlx->setType(AMQP_EX_TYPE_TOPIC);
$dlx->setFlags(AMQP_DURABLE);
$dlx->declareExchange();

$dq = new AMQPQueue($ch);
$dq->setName('dlx-' . $suffix);
$dq->declareQueue();
$dq->setFlags(AMQP_DURABLE);
$dq->bind($dlx->getName(), '#');

$ex = new AMQPExchange($ch);
$ex->setName("exchange-" . $suffix);
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->setFlags(AMQP_DURABLE);
$ex->declareExchange();

$q = new AMQPQueue($ch);
$q->setName('dlx-test-queue-' . $suffix);
$q->setFlags(AMQP_DURABLE);
$q->setArgument('x-dead-letter-exchange', $dlx->getName());
$q->declareQueue();
$q->bind($ex->getName());

$ex->publish('message');

$envelope = $q->get();
$q->nack($envelope->getDeliveryTag());

usleep(20000);

$failed = $dq->get();
var_dump($failed->getHeader('x-death'));
$dq->ack($failed->getDeliveryTag());

$ex->publish(
    $failed->getBody(),
    $failed->getRoutingKey(),
    AMQP_NOPARAM,
    [
        'content_type' => $failed->getContentType(),
        'content_encoding' => $failed->getContentEncoding(),
        'message_id' => $failed->getMessageId(),
        'user_id' => $failed->getUserId(),
        'app_id' => $failed->getAppId(),
        'delivery_mode' => $failed->getDeliveryMode(),
        'priority' => $failed->getPriority(),
        'timestamp' => $failed->getTimeStamp(),
        'expiration' => $failed->getExpiration(),
        'type' => $failed->getType(),
        'reply_to' => $failed->getReplyTo(),
        'headers' => $failed->getHeaders(),
        'correlation_id' => $failed->getCorrelationId(),
    ]
);

usleep(20000);


$envelope = $q->get();
var_dump($envelope->getHeader('x-death'));
$q->nack($envelope->getDeliveryTag());


usleep(20000);

$failedTwice = $dq->get();
var_dump($failedTwice->getHeader('x-death'));
$dq->ack($failedTwice->getDeliveryTag());

?>

==DONE==
--EXPECTF--
array(1) {
  [0]=>
  array(6) {
    ["count"]=>
    int(1)
    ["reason"]=>
    string(8) "rejected"
    ["queue"]=>
    string(%d) "dlx-test-queue-%s"
    ["time"]=>
    object(AMQPTimestamp)#%d (1) {
      ["timestamp":"AMQPTimestamp":private]=>
      string(%d) "%d"
    }
    ["exchange"]=>
    string(%d) "exchange-%s"
    ["routing-keys"]=>
    array(1) {
      [0]=>
      string(0) ""
    }
  }
}
array(1) {
  [0]=>
  array(6) {
    ["count"]=>
    int(1)
    ["reason"]=>
    string(8) "rejected"
    ["queue"]=>
    string(%d) "dlx-test-queue-%s"
    ["time"]=>
    object(AMQPTimestamp)#%d (1) {
      ["timestamp":"AMQPTimestamp":private]=>
      string(%d) "%d"
    }
    ["exchange"]=>
    string(%d) "exchange-%s"
    ["routing-keys"]=>
    array(1) {
      [0]=>
      string(0) ""
    }
  }
}
array(1) {
  [0]=>
  array(6) {
    ["count"]=>
    int(2)
    ["exchange"]=>
    string(%d) "exchange-%s"
    ["queue"]=>
    string(%d) "dlx-test-queue-%s"
    ["reason"]=>
    string(8) "rejected"
    ["routing-keys"]=>
    array(1) {
      [0]=>
      string(0) ""
    }
    ["time"]=>
    object(AMQPTimestamp)#%d (1) {
      ["timestamp":"AMQPTimestamp":private]=>
      string(%d) "%d"
    }
  }
}

==DONE==