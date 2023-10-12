--TEST--
AMQPExchange getConnection test
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
elseif (!getenv("PHP_AMQP_HOST")) print "skip PHP_AMQP_HOST environment variable is not set";
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));

$cnn->connect();
$ch = new AMQPChannel($cnn);

$ex = new AMQPExchange($ch);

$cnn2 = new AMQPConnection();

echo $cnn === $ex->getConnection() ? 'same' : 'not same', PHP_EOL;
echo $cnn2 === $ex->getConnection() ? 'same' : 'not same', PHP_EOL;

$old_host = $cnn->getHost();
$new_host = 'test';

$ex->getConnection()->setHost($new_host);

echo $cnn->getHost() == $new_host ? 'by ref' : 'copy', PHP_EOL;

?>
--EXPECT--
same
not same
by ref
