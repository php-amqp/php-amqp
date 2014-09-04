--TEST--
Segfault when publishing to non existent exchange
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$c = new AMQPConnection();
$c->connect();

$ch = new AMQPChannel($c);

$ex = new AMQPExchange($ch);
$ex->setName("exchange-" . microtime(true));
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->declareExchange();
try {
    $ex->publish("data", "bar");
    echo "Success\n";
} catch (Exception $e) {
    echo "Success\n";
}
$ex->delete();
?>
--EXPECT--
Success
