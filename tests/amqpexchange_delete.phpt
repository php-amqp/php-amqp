--TEST--
AMQPExchange Delete
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();

$ch1 = new AMQPChannel($cnn);
$ch2 = new AMQPChannel($cnn);
$ch3 = new AMQPChannel($cnn);

$ex1 = new AMQPExchange($ch1);
$ex2 = new AMQPExchange($ch2);

$ex1->setName("exchange1-" . time());
$ex2->setName("exchange2-" . time());

$ex1->setType(AMQP_EX_TYPE_FANOUT);
$ex2->setType(AMQP_EX_TYPE_FANOUT);

try {
    $ex1->delete("ex1-11");
} catch (Exception $e) {
    // should not stuck the process
}


try {
    $ex2->delete("ex2-22");
} catch (Exception $e) {
    // should not stuck the process
}


$ex3 = new AMQPExchange($ch3);
$ex3->setName("exchange3-" . time());
$ex3->setType(AMQP_EX_TYPE_FANOUT);
$ex3->declareExchange();

echo ($ex3->delete()) ? "Removed exchange\n" : "Cannot remove exchange\n";

?>
--EXPECT--
Removed exchange
