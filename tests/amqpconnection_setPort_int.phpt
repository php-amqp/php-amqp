--TEST--
AMQPConnection constructor
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$port = 12345;
echo var_export($cnn->setPort($port), true), PHP_EOL;
echo $cnn->getPort(), PHP_EOL;
echo gettype($port), PHP_EOL;
?>
--EXPECT--
true
12345
integer
