--TEST--
AMQPConnection constructor
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
echo $cnn->setPort(12345);
?>
--EXPECT--
1