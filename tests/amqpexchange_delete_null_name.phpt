--TEST--
AMQPExchange::delete with explicit null as exchange name
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
elseif (!getenv("PHP_AMQP_HOST")) print "skip PHP_AMQP_HOST environment variable is not set";
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->connect();
$chan = new AMQPChannel($cnn);

$ex = new AMQPExchange($chan);
$ex->setName('test.queue.' . bin2hex(random_bytes(32)));
$ex->setType(AMQP_EX_TYPE_DIRECT);
$ex->declareExchange();

$ex->delete(null, null);

// Deleting with explicit null deleted the current exchange, so we should be able to redeclare
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->declare();
$ex->delete();
?>
==DONE==
--EXPECT--
==DONE==
