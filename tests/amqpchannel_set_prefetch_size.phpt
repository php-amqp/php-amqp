--TEST--
AMQPChannel::setPrefetchSize
--SKIPIF--
<?php
if (!extension_loaded("amqp")) {
    print "skip AMQP extension is not loaded";
} elseif (!getenv('PHP_AMQP_HOST')) {
    print "skip PHP_AMQP_HOST environment variable is not set";
} else {
    try {
        $cnn = new AMQPConnection();
        $cnn->setHost(getenv('PHP_AMQP_HOST'));
        $cnn->connect();
        $ch = new AMQPChannel($cnn);
        $ch->setPrefetchSize(123);
    } catch (AMQPConnectionException $e) {
        if ($e->getCode() === 540 && strpos($e->getMessage(), "NOT_IMPLEMENTED") !== false) {
            print "skip prefetch size is not supported by the AMQP server";
        }
    }
}
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->connect();
$ch = new AMQPChannel($cnn);

var_dump($ch->getGlobalPrefetchCount());
var_dump($ch->getGlobalPrefetchSize());
var_dump($ch->getPrefetchCount());
var_dump($ch->getPrefetchSize());

$ch->setPrefetchSize(123);

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
int(123)
