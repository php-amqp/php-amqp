--TEST--
AMQPChannel::confirmSelect()
--SKIPIF--
<?php
if (!extension_loaded("amqp") || version_compare(PHP_VERSION, '5.3', '<')) {
    print "skip";
}
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();

$ch = new AMQPChannel($cnn);

$ch->confirmSelect();
echo 'confirm.select: OK', PHP_EOL;

try {
    $ch->startTransaction();
} catch (Exception $e) {
    echo get_class($e), "({$e->getCode()}): " . $e->getMessage(), PHP_EOL;
}

$ch = new AMQPChannel($cnn);
$ch->startTransaction();
echo 'transaction.start: OK', PHP_EOL;

try {
    $ch->confirmSelect();
} catch (Exception $e) {
    echo get_class($e), "({$e->getCode()}): " . $e->getMessage(), PHP_EOL;
}


?>
--EXPECT--
confirm.select: OK
AMQPChannelException(406): Server channel error: 406, message: PRECONDITION_FAILED - cannot switch from confirm to tx mode
transaction.start: OK
AMQPChannelException(406): Server channel error: 406, message: PRECONDITION_FAILED - cannot switch from tx to confirm mode
