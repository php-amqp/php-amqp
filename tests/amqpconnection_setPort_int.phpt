--TEST--
AMQPConnection constructor
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$port = 12345;
var_dump($cnn->setPort($port));
echo $cnn->getPort(), PHP_EOL;
echo gettype($port), PHP_EOL;
?>
--EXPECT--
NULL
12345
integer
