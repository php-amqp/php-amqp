--TEST--
AMQPExchange publish with properties
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
require '_test_helpers.php.inc';

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


//var_dump($attrs, $attrs_control);
echo 'Message attributes are ', $attrs === $attrs_control ? 'the same' : 'not the same', PHP_EOL;

$msg = $q->get(AMQP_AUTOACK);


dump_message($msg);

$ex->delete();
$q->delete();


?>
--EXPECTF--
true
Message attributes are the same
AMQPEnvelope
    getBody:
        string(7) "message"
    getContentType:
        string(1) "1"
    getRoutingKey:
        string(11) "routing.key"
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
        string(1) "2"
    getType:
        string(1) "7"
    getTimeStamp:
        int(123)
    getPriority:
        int(2)
    getExpiration:
        string(9) "100000000"
    getUserId:
        string(0) ""
    getAppId:
        string(1) "5"
    getMessageId:
        string(1) "3"
    getReplyTo:
        string(1) "8"
    getCorrelationId:
        string(1) "9"
    getHeaders:
        array(0) {
}
