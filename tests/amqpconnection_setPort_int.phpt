--TEST--
AMQPConnection constructor
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip";
if (!getenv("PHP_AMQP_HOST")) print "skip";
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$port = 12345;
var_dump($cnn->setPort($port));
echo $cnn->getPort(), PHP_EOL;
echo gettype($port), PHP_EOL;
?>
--EXPECT--
NULL
12345
integer
