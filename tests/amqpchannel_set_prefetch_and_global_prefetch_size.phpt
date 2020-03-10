--TEST--
AMQPChannel - Setting both consumer and channel wide prefetch sizes.
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

$ch->setPrefetchSize(11);

var_dump($ch->getGlobalPrefetchCount());
var_dump($ch->getGlobalPrefetchSize());
var_dump($ch->getPrefetchCount());
var_dump($ch->getPrefetchSize());

// Shouldn't affect the prefetch count
$ch->setGlobalPrefetchSize(22);

var_dump($ch->getGlobalPrefetchCount());
var_dump($ch->getGlobalPrefetchSize());
var_dump($ch->getPrefetchCount());
var_dump($ch->getPrefetchSize());

// Shouldn't affect the global prefetch count
$ch->setPrefetchSize(33);

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
int(0)
int(0)
int(11)
int(0)
int(22)
int(0)
int(11)
int(0)
int(22)
int(0)
int(33)
