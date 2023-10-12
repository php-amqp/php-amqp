--TEST--
AMQPExchange::publish() - publish with confirms
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
elseif (!getenv("PHP_AMQP_HOST")) print "skip PHP_AMQP_HOST environment variable is not set";
elseif (getenv("SKIP_SLOW_TESTS")) print "skip slow test and SKIP_SLOW_TESTS is set";
?>
<?php //print "skip - WIP"; ?>
--FILE--
<?php

function exception_error_handler($errno, $errstr, $errfile, $errline)
{
    echo $errstr, PHP_EOL;
}

set_error_handler('exception_error_handler');


$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
//$cnn->setReadTimeout(2);
$cnn->connect();


$ch = new AMQPChannel($cnn);
$ch->confirmSelect();

try {
    $ch->waitForConfirm(1);
} catch(Exception $e) {
    echo get_class($e), "({$e->getCode()}): ", $e->getMessage(). PHP_EOL;
}


$ex1 = new AMQPExchange($ch);
$ex1->setName("exchange-" . bin2hex(random_bytes(32)));
$ex1->setType(AMQP_EX_TYPE_FANOUT);
$ex1->setFlags(AMQP_AUTODELETE);
$ex1->declareExchange();


var_dump($ex1->publish('message 1', 'routing.key'));
var_dump($ex1->publish('message 1', 'routing.key', AMQP_MANDATORY));

try {
    $ch->waitForConfirm();
} catch(Exception $e) {
    echo get_class($e), "({$e->getCode()}): ", $e->getMessage(). PHP_EOL;
}

try {
    $ch->waitForConfirm();
} catch(Exception $e) {
    echo get_class($e), "({$e->getCode()}): ", $e->getMessage(). PHP_EOL;
}

try {
    $ch->waitForConfirm(1);
} catch(Exception $e) {
    echo get_class($e), "({$e->getCode()}): ", $e->getMessage(). PHP_EOL;
}


var_dump($ex1->publish('message 1', 'routing.key'));
var_dump($ex1->publish('message 1', 'routing.key', AMQP_MANDATORY));

// ack_callback(int $delivery_tag, bool $multiple) : bool;
// nack_callback(int $delivery_tag, bool $multiple, bool $requeue) : bool;
// return_callback(int $reply_code, string $reply_text, string $exchange, string $routing_key, AMQPBasicProperties $properties, string $body);

$ch->setReturnCallback(function ($reply_code, $reply_text, $exchange, $routing_key, AMQPBasicProperties $properties, $body) {
    echo 'Message returned: ', $reply_text, ', message body:', $body, PHP_EOL;
});

$cnt = 2;
$ch->setConfirmCallback(function ($delivery_tag, $multiple) use(&$cnt) {
    echo 'Message acked', PHP_EOL;
    var_dump(func_get_args());
    return --$cnt > 0;
}, function ($delivery_tag, $multiple, $requeue) {
    echo 'Message nacked', PHP_EOL;
    var_dump(func_get_args());
    return false;
});

try {
    $ch->waitForConfirm();
} catch(Exception $e) {
    echo get_class($e), "({$e->getCode()}): ", $e->getMessage(). PHP_EOL;
}

$ex1->delete();

$ex2 = new AMQPExchange($ch);
$ex2->setName("exchange-nonexistent-" . bin2hex(random_bytes(32)));
var_dump($ex2->publish('message 2', 'routing.key'));

try {
    $ch->waitForConfirm(1);
} catch(Exception $e) {
    echo get_class($e), "({$e->getCode()}): ", $e->getMessage(). PHP_EOL;
}

?>
--EXPECTF--
AMQPQueueException(0): Wait timeout exceed
NULL
NULL
Unhandled basic.ack method from server received. Use AMQPChannel::setConfirmCallback() to process it.
Unhandled basic.return method from server received. Use AMQPChannel::setReturnCallback() to process it.
Unhandled basic.ack method from server received. Use AMQPChannel::setConfirmCallback() to process it.
NULL
NULL
Message acked
array(2) {
  [0]=>
  int(3)
  [1]=>
  bool(false)
}
Message returned: NO_ROUTE, message body:message 1
Message acked
array(2) {
  [0]=>
  int(4)
  [1]=>
  bool(false)
}
NULL
AMQPChannelException(404): Server channel error: 404, message: NOT_FOUND - no exchange 'exchange-nonexistent-%s' in vhost '/'
