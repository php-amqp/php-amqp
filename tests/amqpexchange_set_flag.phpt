--TEST--
AMQPExchange set flag string
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();

$ch = new AMQPChannel($cnn);

$ex = new AMQPExchange($ch);

$ex->setFlags("2");
echo $ex->getFlags();

$ex->setFlags(NULL);
echo $ex->getFlags();

$ex->setFlags(2);
echo $ex->getFlags();

?>
--EXPECT--
202