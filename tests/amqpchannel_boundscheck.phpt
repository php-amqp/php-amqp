--TEST--
AMQPChannel Bounds Check
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();
$ch = new AMQPChannel($cnn);
try {
    $ch->setPrefetchCount(-1);
} catch(InvalidArgumentException $e) {
    echo $e->getMessage(), PHP_EOL;
}
try {
    $ch->setPrefetchCount(65536);
} catch(InvalidArgumentException $e) {
 echo $e->getMessage(), PHP_EOL;
}
try {
    $ch->setPrefetchSize(-1);
} catch(InvalidArgumentException $e) {
    echo $e->getMessage(), PHP_EOL;
}
try {
    $ch->setPrefetchSize(4294967296);
} catch(InvalidArgumentException $e) {
    echo $e->getMessage(), PHP_EOL;
}
try {
    $ch->qos(-1, 0);
} catch(InvalidArgumentException $e) {
    echo $e->getMessage(), PHP_EOL;
}
try {
    $ch->qos(4294967296, 0);
} catch(InvalidArgumentException $e) {
    echo $e->getMessage(), PHP_EOL;
}
try {
    $ch->qos(0, -1);
} catch(InvalidArgumentException $e) {
    echo $e->getMessage(), PHP_EOL;
}
try {
    $ch->qos(0, 65536);
} catch(InvalidArgumentException $e) {
    echo $e->getMessage(), PHP_EOL;
}
--EXPECT--
prefetchCount=-1 out of bounds 0-65535
prefetchCount=65536 out of bounds 0-65535
prefetchSize=-1 out of bounds 0-4294967295
prefetchSize=4294967296 out of bounds 0-4294967295
prefetchSize=-1 out of bounds 0-4294967295
prefetchSize=4294967296 out of bounds 0-4294967295
prefetchCount=-1 out of bounds 0-65535
prefetchCount=65536 out of bounds 0-65535
