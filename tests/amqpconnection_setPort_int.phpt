--TEST--
AMQPConnection constructor
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
echo var_export($cnn->setPort(12345), true), PHP_EOL;
echo $cnn->getPort(), PHP_EOL;
?>
--EXPECT--
true
12345
