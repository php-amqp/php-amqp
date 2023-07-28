--TEST--
AMQPExchange::publish() - publish unroutable mandatory
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
<?php //print "skip - WIP"; ?>
--FILE--
<?php

function exception_error_handler($errno, $errstr, $errfile, $errline)
{
    echo $errstr, PHP_EOL;
}

set_error_handler('exception_error_handler');


$cnn = new AMQPConnection();
//$cnn->setReadTimeout(2);

$cnn->connect();


$ch = new AMQPChannel($cnn);


try {
    $ch->waitForBasicReturn(1);
} catch(Exception $e) {
    echo get_class($e), "({$e->getCode()}): ", $e->getMessage(). PHP_EOL;
}


$ex = new AMQPExchange($ch);
$ex->setName("exchange-" . microtime(true));
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->setFlags(AMQP_AUTODELETE);
$ex->declareExchange();

var_dump($ex->publish('message 1', 'routing.key', AMQP_MANDATORY));
var_dump($ex->publish('message 2', 'routing.key', AMQP_MANDATORY));

// Create a new queue
$q = new AMQPQueue($ch);
$q->setName('queue-' . microtime(true));
$q->setFlags(AMQP_AUTODELETE);
$q->declareQueue();

$msg = $q->get();
var_dump($msg);

try {
    $ch->waitForBasicReturn();
} catch(Exception $e) {
    echo get_class($e), "({$e->getCode()}): ", $e->getMessage(). PHP_EOL;
}

/* callback(int $reply_code, string $reply_text, string $exchange, string $routing_key, AMQPBasicProperties $properties, string $body); */
$ch->setReturnCallback(function ($reply_code, $reply_text, $exchange, $routing_key, AMQPBasicProperties $properties, $body) {
    echo 'Message returned', PHP_EOL;
    var_dump(func_get_args());
    return false;
});

try {
    $ch->waitForBasicReturn();
} catch(Exception $e) {
    echo get_class($e), "({$e->getCode()}): ", $e->getMessage(). PHP_EOL;
}


$q->delete();


$ex->delete();
?>
--EXPECTF--
AMQPQueueException(0): Wait timeout exceed
NULL
NULL
NULL
Unhandled basic.return method from server received. Use AMQPChannel::setReturnCallback() to process it.
Message returned
array(6) {
  [0]=>
  int(312)
  [1]=>
  string(8) "NO_ROUTE"
  [2]=>
  string(%d) "exchange-%f"
  [3]=>
  string(11) "routing.key"
  [4]=>
  object(AMQPBasicProperties)#7 (14) {
    ["contentType":"AMQPBasicProperties":private]=>
    string(10) "text/plain"
    ["contentEncoding":"AMQPBasicProperties":private]=>
    string(0) ""
    ["headers":"AMQPBasicProperties":private]=>
    array(0) {
    }
    ["deliveryMode":"AMQPBasicProperties":private]=>
    int(1)
    ["priority":"AMQPBasicProperties":private]=>
    int(0)
    ["correlationId":"AMQPBasicProperties":private]=>
    string(0) ""
    ["replyTo":"AMQPBasicProperties":private]=>
    string(0) ""
    ["expiration":"AMQPBasicProperties":private]=>
    string(0) ""
    ["messageId":"AMQPBasicProperties":private]=>
    string(0) ""
    ["timestamp":"AMQPBasicProperties":private]=>
    int(0)
    ["type":"AMQPBasicProperties":private]=>
    string(0) ""
    ["userId":"AMQPBasicProperties":private]=>
    string(0) ""
    ["appId":"AMQPBasicProperties":private]=>
    string(0) ""
    ["clusterId":"AMQPBasicProperties":private]=>
    string(0) ""
  }
  [5]=>
  string(9) "message 2"
}
