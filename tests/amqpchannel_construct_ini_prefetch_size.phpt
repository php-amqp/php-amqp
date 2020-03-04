--TEST--
AMQPChannel - constructor with amqp.prefetch_size ini value set
--SKIPIF--
<?php
if (!extension_loaded("amqp")) {
    print "skip";
} else {
    try {
        $cnn = new AMQPConnection();
        $cnn->connect();
        $ch = new AMQPChannel($cnn);
        $ch->setPrefetchSize(123);
    } catch (AMQPConnectionException $e) {
        if ($e->getCode() === 540 && strpos($e->getMessage(), "NOT_IMPLEMENTED") !== false) {
            print "skip";
        }
    }
}
?>
--INI--
amqp.prefetch_size=123
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
int(3)
int(123)
