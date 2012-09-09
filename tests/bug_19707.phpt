--TEST--
AMQPQueue::get() doesn't return the message
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();

$ch = new AMQPChannel($cnn);

$ex = new AMQPExchange($ch);
$ex->setName("exchange_testing_19707");
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->declare();

$q = new AMQPQueue($ch);
$q->setName('queue' . time());
$q->setFlags(AMQP_DURABLE);
$q->declare();

$q->bind($ex->getName(), 'routing.key');

$ex->publish('message', 'routing.key');

$msg = $q->get();

echo "message received from get:";
print_r($msg);

$q->delete();
$ex->delete();
?>
--EXPECTF--
message received from get:AMQPEnvelope Object
(
    [body] => message
    [content_type] => text/plain
    [routing_key] => routing.key
    [delivery_tag] => 1
    [delivery_mode] => 0
    [exchange_name] => exchange_testing_19707
    [is_redelivery] => 0
    [content_encoding] => 
    [type] => 
    [timestamp] => 0
    [priority] => 0
    [expiration] => 
    [user_id] => 
    [app_id] => 
    [message_id] => 
    [reply_to] => 
    [correlation_id] => 
    [headers] => Array
        (
        )
)
