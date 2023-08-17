--TEST--
AMQPExchange
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip";
if (!getenv("PHP_AMQP_HOST")) print "skip";
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->connect();

$name = "exchange-" . bin2hex(random_bytes(32));

$ex = new AMQPExchange(new AMQPChannel($cnn));
$ex->setName($name);
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->declareExchange();

$ex2 = new AMQPExchange(new AMQPChannel($cnn));
$ex2->setName($name);
$ex2->setType(AMQP_EX_TYPE_DIRECT);

try {
    $ex2->declareExchange();
} catch (AMQPExchangeException $e) {
    echo get_class($e) . "\n";
    try {
        $ex2->delete();
    } catch (AMQPChannelException $e) {
        echo get_class($e) . "\n";
    }
}
?>
=DONE=
--EXPECT--
AMQPExchangeException
AMQPChannelException
=DONE=