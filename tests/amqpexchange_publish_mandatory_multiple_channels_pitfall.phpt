--TEST--
AMQPExchange::publish() - publish unroutable mandatory on multiple channels pitfall
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip";
if (!getenv("PHP_AMQP_HOST")) print "skip";
?>
--FILE--
<?php

function exception_error_handler($errno, $errstr, $errfile, $errline)
{
    echo $errstr, PHP_EOL;
}

set_error_handler('exception_error_handler');


$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->connect();


$ch1 = new AMQPChannel($cnn);

try {
    $ch1->waitForBasicReturn(1);
} catch (Exception $e) {
    echo get_class($e), "({$e->getCode()}): ", $e->getMessage() . PHP_EOL;
}

$ch2 = new AMQPChannel($cnn);

try {
    $ch2->waitForBasicReturn(1);
} catch (Exception $e) {
    echo get_class($e), "({$e->getCode()}): ", $e->getMessage() . PHP_EOL;
}


$exchange_name = "exchange-" . bin2hex(random_bytes(32));

$ex1 = new AMQPExchange($ch1);
$ex1->setName($exchange_name);
$ex1->setType(AMQP_EX_TYPE_FANOUT);
$ex1->setFlags(AMQP_AUTODELETE);
$ex1->declareExchange();


$ex2 = new AMQPExchange($ch2);
$ex2->setName($exchange_name);

var_dump($ex2->publish('message 1-2', 'routing.key', AMQP_MANDATORY));
var_dump($ex2->publish('message 2-2', 'routing.key', AMQP_MANDATORY));

// Create a new queue
$q = new AMQPQueue($ch1);
$q->setName('queue-' . bin2hex(random_bytes(32)));
$q->setFlags(AMQP_AUTODELETE);
$q->declareQueue();

$msg = $q->get();
var_dump($msg);


try {
    $ch1->waitForBasicReturn();
} catch (Exception $e) {
    echo get_class($e), "({$e->getCode()}): ", $e->getMessage(), PHP_EOL;
}

// This error happens because on a channel 1 we are expecting only messages withing channel 1, but inside current
// connection we already have pending message on channel 2

echo 'Connection active: ', ($cnn->isConnected() ? 'yes' : 'no');

?>
--EXPECTF--
AMQPQueueException(0): Wait timeout exceed
AMQPQueueException(0): Wait timeout exceed
NULL
NULL
NULL
AMQPException(0): unexpected method received
Connection active: no
