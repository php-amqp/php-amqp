--TEST--
AMQPExchange::publish() - publish in conform mode and handle conforms with AMQPQueue::consume() method
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php

function exception_error_handler($errno, $errstr, $errfile, $errline)
{
    echo $errstr, PHP_EOL;
}

set_error_handler('exception_error_handler');


$cnn = new AMQPConnection();
$cnn->setReadTimeout(2);

$cnn->connect();
$ch = new AMQPChannel($cnn);
$ch->confirmSelect();

$ex = new AMQPExchange($ch);
$ex->setName("exchange-" . microtime(true));
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->setFlags(AMQP_AUTODELETE);
$ex->declareExchange();

echo $ex->publish('message 1', 'routing.key', AMQP_MANDATORY) ? 'true' : 'false', PHP_EOL;

// Create a new queue
$q = new AMQPQueue($ch);
$q->setName('queue-' . microtime(true));
$q->setFlags(AMQP_AUTODELETE);
$q->declareQueue();

$msg = $q->get();
var_dump($msg);

try {
    $q->consume(function() use ($q) {
        echo 'Message returned', PHP_EOL;
        var_dump(func_get_args());
        return false;
    });
} catch(Exception $e) {
    echo get_class($e), "({$e->getCode()}): ", $e->getMessage(). PHP_EOL;
}

echo $ex->publish('message 2', 'routing.key', AMQP_MANDATORY) ? 'true' : 'false', PHP_EOL;

/* callback(int $reply_code, string $reply_text, string $exchange, string $routing_key, AMQPBasicProperties $properties, string $body); */
$ch->setReturnCallback(function ($reply_code, $reply_text, $exchange, $routing_key, AMQPBasicProperties $properties, $body) {
    echo 'Message returned: ', $reply_text, ', message body:', $body, PHP_EOL;
});

$cnt = 1;
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
    $q->consume(function() use ($q) {
        echo 'Received message', PHP_EOL;
        var_dump(func_get_args());
        return false;
    });
} catch(Exception $e) {
    echo get_class($e), "({$e->getCode()}): ", $e->getMessage(). PHP_EOL;
}


$q->delete();
$ex->delete();
?>
--EXPECTF--
true
bool(false)
Unhandled basic.return method from server received. Use AMQPChannel::setReturnCallback() to process it.
Unhandled basic.ack method from server received. Use AMQPChannel::setConfirmCallback() to process it.
AMQPQueueException(0): Consumer timeout exceed
true
Message returned: NO_ROUTE, message body:message 2
Message acked
array(2) {
  [0]=>
  int(2)
  [1]=>
  bool(false)
}
AMQPQueueException(0): Consumer timeout exceed
