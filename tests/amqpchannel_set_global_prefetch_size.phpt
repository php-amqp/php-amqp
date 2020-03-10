--TEST--
AMQPChannel::setGlobalPrefetchSize
--SKIPIF--
<?php
if (!extension_loaded("amqp")) {
    print "skip";
} else {
    try {
        $cnn = new AMQPConnection();
        $cnn->connect();
        $ch = new AMQPChannel($cnn);
        $ch->setGlobalPrefetchSize(123);
    } catch (AMQPConnectionException $e) {
        if ($e->getCode() === 540 && strpos($e->getMessage(), "NOT_IMPLEMENTED") !== false) {
            print "skip";
        }
    }
}
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();
$ch = new AMQPChannel($cnn);

var_dump($ch->getGlobalPrefetchCount());
var_dump($ch->getGlobalPrefetchSize());
var_dump($ch->getPrefetchCount());
var_dump($ch->getPrefetchSize());

$ch->setGlobalPrefetchSize(123);

var_dump($ch->getGlobalPrefetchCount());
var_dump($ch->getGlobalPrefetchSize());
var_dump($ch->getPrefetchCount());
var_dump($ch->getPrefetchSize());

?>
--EXPECT--
int(0)
int(0)
int(3)
int(0)
int(0)
int(123)
int(3)
int(0)
