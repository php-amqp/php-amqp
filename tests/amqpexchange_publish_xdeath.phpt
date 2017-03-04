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

function assert_xdeath(AMQPEnvelope $envelope, $exchangeName, $queueName) {
    if (!$envelope->hasHeader('x-death')) {
        return 'header-missing';
    }

    $header = $envelope->getHeader('x-death');

    if (count($header) !== 1) {
        return 'unexpected-number-of-headers-' . count($header);
    }

    if (!isset($header[0]['reason']) || $header[0]['reason'] !== 'rejected') {
        return 'unexpected-reason';
    }

    if (!isset($header[0]['time']) || !$header[0]['time'] instanceof AMQPTimestamp) {
        return 'unexpected-time';
    }

    if (!isset($header[0]['exchange']) || $header[0]['exchange'] !== $exchangeName) {
        return 'unexpected-exchange';
    }

    if (!isset($header[0]['queue']) || $header[0]['queue'] !== $queueName) {
        return 'unexpected-queue';
    }

    if (!isset($header[0]['routing-keys']) || $header[0]['routing-keys'] !== ['']) {
        return 'unexpected-routing-keys';
    }

    if (!isset($header[0]['count'])) {
        return 'count-missing';
    }

    return $header[0]['count'];
}

$envelope = $q->get();
var_dump(assert_xdeath($envelope, $ex->getName(), $q->getName()));
$q->nack($envelope->getDeliveryTag());

usleep(20000);

$failed = $dq->get();
var_dump(assert_xdeath($failed, $ex->getName(), $q->getName()));
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
var_dump(assert_xdeath($envelope, $ex->getName(), $q->getName()));
$q->nack($envelope->getDeliveryTag());


usleep(20000);

$failedTwice = $dq->get();
var_dump(assert_xdeath($failedTwice, $ex->getName(), $q->getName()));
$dq->ack($failedTwice->getDeliveryTag());

?>

==DONE==
--EXPECTF--
string(14) "header-missing"
int(1)
int(1)
int(2)

==DONE==