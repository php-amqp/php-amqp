--TEST--
AMQPChannel - constructor with amqp.prefetch_count ini value set
--SKIPIF--
<?php
if (!extension_loaded("amqp")) {
    print "skip";
}
?>
--INI--
amqp.prefetch_count=123
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();
$ch = new AMQPChannel($cnn);

var_dump($ch->getGlobalPrefetchCount());
var_dump($ch->getGlobalPrefetchSize());
var_dump($ch->getPrefetchCount());
var_dump($ch->getPrefetchSize());
?>
--EXPECT--
int(0)
int(0)
int(123)
int(0)
