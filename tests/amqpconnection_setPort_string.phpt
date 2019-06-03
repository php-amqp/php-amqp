--TEST--
AMQPConnection setPort with string
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$port = '12345';
echo $cnn->setPort($port), PHP_EOL;
var_dump($cnn->getPort());
var_dump($port);
?>
--EXPECT--
1
int(12345)
string(5) "12345"
