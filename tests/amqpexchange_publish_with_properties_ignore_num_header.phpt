--TEST--
AMQPExchange publish with properties - ignore numeric keys in headers
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


var_dump($ex->publish('message', 'routing.key', AMQP_NOPARAM, array('headers' => 'ignored')));
var_dump($ex->publish('message', 'routing.key', AMQP_NOPARAM, array('headers' => array(2 => 'ignore_me'))));

$ex->delete();


?>
--EXPECTF--
Warning: AMQPExchange::publish(): Ignoring non-string header field '0' in %s on line %d
NULL

Warning: AMQPExchange::publish(): Ignoring non-string header field '2' in %s on line %d
NULL
