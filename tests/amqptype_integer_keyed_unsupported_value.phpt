--TEST--
Serializing an integer-keyed array with an unsupported value warns instead of crashing
--EXTENSIONS--
amqp
--SKIPIF--
<?php
if (!getenv("PHP_AMQP_HOST")) print "skip PHP_AMQP_HOST environment variable is not set";
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->connect();

$channel = new AMQPChannel($cnn);

$queue = new AMQPQueue($channel);
$queue->setName('args-' . bin2hex(random_bytes(16)));
$queue->setFlags(AMQP_AUTODELETE);
$queue->setArguments(['x-nested' => [fopen('php://memory', 'r')]]);
$queue->declareQueue();

echo "ok\n";
?>
--EXPECTF--
Warning: AMQPQueue::declareQueue(): Ignoring field '0' due to unsupported value type (resource) in %s on line %d
ok
