--TEST--
#155 RabbitMQ's Direct reply-to (related to consume multiple)
--SKIPIF--
<?php if (!extension_loaded("amqp")) {
    print "skip";
} ?>
--FILE--
<?php
$conn = new AMQPConnection();
$conn->connect();

$channel  = new AMQPChannel($conn);
$exchange = new AMQPExchange($channel);


$q_reply_to = new AMQPQueue($channel);
$q_reply_to->setName('amq.rabbitmq.reply-to');
$q_reply_to->consume(null, AMQP_AUTOACK);


// this will be kind out long-living queue to
$q_request = new AMQPQueue($channel);
$q_request->setName('reply-to-requests');
$q_request->setFlags(AMQP_DURABLE);
$q_request->declareQueue();
$q_request->purge();

$q_request_name = $q_request->getName();

echo 'Publishing request...' . PHP_EOL;

$exchange->publish('request', $q_request_name, AMQP_NOPARAM, array('reply_to' => 'amq.rabbitmq.reply-to'));

$request_message = $q_request->get(AMQP_AUTOACK);

$reply_to = $request_message->getReplyTo();

echo 'Reply-to queue: ', $reply_to, PHP_EOL;

echo 'Prepare response queue...' . PHP_EOL;

$channel_2 = new AMQPChannel($conn);

$q_reply = new AMQPQueue($channel_2);
$q_reply->setName($reply_to);
$q_reply->setFlags(AMQP_PASSIVE);
$q_reply->declareQueue();

echo 'Publishing response...' . PHP_EOL;

$exchange->publish('response', $reply_to, AMQP_NOPARAM);


echo 'Waiting for reply...' . PHP_EOL;
$q_reply_to->consume(function (AMQPEnvelope $message, AMQPQueue $queue) {
    echo $message->getBody() . ': ' . $message->getRoutingKey() . PHP_EOL;

    echo 'Received on ', $queue->getName(), ' queue', PHP_EOL;

    return false;
}, AMQP_JUST_CONSUME);

echo 'done', PHP_EOL;



?>
--EXPECTF--
Publishing request...
Reply-to queue: amq.rabbitmq.reply-to.%s.%s==
Prepare response queue...
Publishing response...
Waiting for reply...
response: amq.rabbitmq.reply-to.%s.%s==
Received on amq.rabbitmq.reply-to queue
done