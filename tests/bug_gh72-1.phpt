--TEST--
#72 Publishing to an exchange with an empty name is valid and should not throw an exception (1)
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$connection = new AMQPConnection();
$connection->connect();
$channel = new AMQPChannel($connection);

$exchange = new AMQPExchange($channel);
$exchange->setName('');

$exchange->publish('msg', 'key');
?>
==DONE==
--EXPECT--
==DONE==
