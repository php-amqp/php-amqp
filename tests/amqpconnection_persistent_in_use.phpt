--TEST--
AMQPConnection persitent connection resource can't be used by multiple connection
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
echo get_class($cnn), PHP_EOL;
$cnn->pconnect();
echo $cnn->isConnected() ? 'true' : 'false', PHP_EOL;

echo PHP_EOL;

$cnn2 = new AMQPConnection();
echo get_class($cnn), PHP_EOL;

try {
    $cnn2->pconnect();
    echo 'reused', PHP_EOL;
} catch (AMQPException $e) {
    echo get_class($e), "({$e->getCode()}): ", $e->getMessage(), PHP_EOL;
}
echo $cnn->isConnected() ? 'true' : 'false', PHP_EOL;

?>
--EXPECT--
AMQPConnection
true

AMQPConnection
AMQPConnectionException(0): There are already established persistent connection to the same resource.
true