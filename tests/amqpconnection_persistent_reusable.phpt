--TEST--
AMQPConnection persistent connection are reusable
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
elseif (!getenv("PHP_AMQP_HOST")) print "skip PHP_AMQP_HOST environment variable is not set";
?>
--FILE--
<?php
$manual = false; // set to true when you want to check via RabbitMQ Management panel that connection really persists.

$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->pconnect();
echo get_class($cnn), PHP_EOL;
echo $cnn->isConnected() ? 'true' : 'false', PHP_EOL;

if ($manual) {
    $ch = new AMQPChannel($cnn);
    sleep(10);
    $ch = null;
}

$cnn = null;
echo PHP_EOL;

if ($manual) {
    sleep(10);
}

$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->pconnect();
echo get_class($cnn), PHP_EOL;
echo $cnn->isConnected() ? 'true' : 'false', PHP_EOL;

if ($manual) {
    $ch = new AMQPChannel($cnn);
    sleep(10);
    $ch = null;
}

$cnn->pdisconnect();

?>
--EXPECT--
AMQPConnection
true

AMQPConnection
true
