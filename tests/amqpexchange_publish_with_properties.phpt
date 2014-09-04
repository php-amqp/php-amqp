--TEST--
AMQPExchange publish with properties
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();

$ch = new AMQPChannel($cnn);

$ex = new AMQPExchange($ch);
$ex->setName("exchange-" . microtime(true));
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->declareExchange();

$q = new AMQPQueue($ch);
$q->declareQueue();

$q->bind($ex->getName());

$attrs = array(
    'content_type'     => 1, // should be string
    'content_encoding' => 2, // should be string
    'message_id'       => 3, // should be string
    //'user_id'          => 4, // should be string // NOTE: fail due to Validated User-ID https://www.rabbitmq.com/validated-user-id.html, @see tests/amqpexchange_publish_with_properties_user_id_failure.phpt test
    'app_id'           => 5, // should be string
    'delivery_mode'    => '1-non-persistent', // should be long
    'priority'         => '2high', // should be long
    'timestamp'        => '123now', // should be long
    'expiration'       => 100000000, // should be string // NOTE: in fact it is milliseconds for how long to stay in queue, see https://www.rabbitmq.com/ttl.html#per-message-ttl for details
    'type'             => 7, // should be string
    'reply_to'         => 8, // should be string
    'correlation_id'   => 9, // should be string
    //'headers'          => 'not array', // should be array // NOTE: covered in tests/amqpexchange_publish_with_properties_ignore_num_header.phpt
);

$attrs_control = array(
    'content_type'     => 1, // should be string
    'content_encoding' => 2, // should be string
    'message_id'       => 3, // should be string
    //'user_id'          => 4, // should be string // NOTE: fail due to Validated User-ID https://www.rabbitmq.com/validated-user-id.html, @see tests/amqpexchange_publish_with_properties_user_id_failure.phpt test
    'app_id'           => 5, // should be string
    'delivery_mode'    => '1-non-persistent', // should be long
    'priority'         => '2high', // should be long
    'timestamp'        => '123now', // should be long
    'expiration'       => 100000000, // should be string // NOTE: in fact it is milliseconds for how long to stay in queue, see https://www.rabbitmq.com/ttl.html#per-message-ttl for details
    'type'             => 7, // should be string
    'reply_to'         => 8, // should be string
    'correlation_id'   => 9, // should be string
    //'headers'          => 'not array', // should be array // NOTE: covered in tests/amqpexchange_publish_with_properties_ignore_num_header.phpt
);

echo $ex->publish('message', 'routing.key', AMQP_NOPARAM, $attrs) ? 'true' : 'false', PHP_EOL;


echo 'Message attributes are ', $attrs == $attrs_control ? 'the same' : 'not the same', PHP_EOL;

$msg = $q->get(AMQP_AUTOACK);

function dump_message(AMQPEnvelope $msg) {
    echo get_class($msg), PHP_EOL;
    var_dump($msg->getBody());
    var_dump($msg->getContentType());
    var_dump($msg->getRoutingKey());
    var_dump($msg->getDeliveryTag());
    var_dump($msg->getDeliveryMode());
    var_dump($msg->getExchangeName());
    var_dump($msg->isRedelivery());
    var_dump($msg->getContentEncoding());
    var_dump($msg->getType());
    var_dump($msg->getTimeStamp());
    var_dump($msg->getPriority());
    var_dump($msg->getExpiration());
    var_dump($msg->getUserId());
    var_dump($msg->getAppId());
    var_dump($msg->getMessageId());
    var_dump($msg->getReplyTo());
    var_dump($msg->getCorrelationId());
    var_dump($msg->getHeaders());
}

dump_message($msg);

$ex->delete();
$q->delete();


?>
--EXPECTF--
true
Message attributes are the same
AMQPEnvelope
string(7) "message"
string(1) "1"
string(11) "routing.key"
int(1)
int(1)
string(%d) "exchange-%f"
bool(false)
string(1) "2"
string(1) "7"
int(123)
int(2)
string(9) "100000000"
string(0) ""
string(1) "5"
string(1) "3"
string(1) "8"
string(1) "9"
array(0) {
}