--TEST--
AMQPExchange::publish() - headers table is not built (nor leaked) when the channel is unavailable
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

$exchange = new AMQPExchange($channel);
$exchange->setType(AMQP_EX_TYPE_DIRECT);
$exchange->setName('');

$cnn->disconnect();

try {
    $exchange->publish('body', 'rk', AMQP_NOPARAM, ['headers' => ['x-foo' => str_repeat('A', 4096)]]);
    echo "no exception\n";
} catch (AMQPChannelException $e) {
    echo $e::class, ': ', $e->getMessage(), "\n";
}
?>
--EXPECT--
AMQPChannelException: Could not publish to exchange. No channel available.
