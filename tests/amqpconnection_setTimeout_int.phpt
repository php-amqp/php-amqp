--TEST--
AMQPConnection setTimeout int
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
@$cnn->setTimeout(3);
var_dump(@$cnn->getTimeout());
?>
--EXPECT--
float(3)
