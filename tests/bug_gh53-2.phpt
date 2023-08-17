--TEST--
Upgrade to RabbitMQ 3.1.0-1: AMQPConnectionException: connection closed unexpectedly (2)
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip";
if (!getenv("PHP_AMQP_HOST")) print "skip";
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->connect();

$channel = new AMQPChannel($cnn);

$exchange = new AMQPExchange($channel);
$exchange->setName('exchange' . bin2hex(random_bytes(32)));
$exchange->setType(AMQP_EX_TYPE_TOPIC);
$exchange->declareExchange();

$queue = new AMQPQueue($channel);
$queue->setName('queue1' . bin2hex(random_bytes(32)));
$queue->declareQueue();
$queue->bind($exchange->getName(), '#');

$exchange->publish('body1', 'routing.1');
$exchange->publish('body2', 'routing.1');

$msg = $queue->get(AMQP_AUTOACK);
var_dump($msg->getBody());

$msg = $queue->get(AMQP_AUTOACK);
var_dump($msg->getBody());

?>
==DONE==
--EXPECT--
string(5) "body1"
string(5) "body2"
==DONE==
