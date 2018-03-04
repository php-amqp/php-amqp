--TEST--
AMQPExchange::publish() - publish unroutable with mandatory flag and handle them with AMQPQueue::consume() method
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
    echo 'Message returned', PHP_EOL;
    var_dump(func_get_args());
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
AMQPQueueException(0): Consumer timeout exceed
true
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
  object(AMQPBasicProperties)#9 (14) {
    ["content_type":"AMQPBasicProperties":private]=>
    string(10) "text/plain"
    ["content_encoding":"AMQPBasicProperties":private]=>
    string(0) ""
    ["headers":"AMQPBasicProperties":private]=>
    array(0) {
    }
    ["delivery_mode":"AMQPBasicProperties":private]=>
    int(1)
    ["priority":"AMQPBasicProperties":private]=>
    int(0)
    ["correlation_id":"AMQPBasicProperties":private]=>
    string(0) ""
    ["reply_to":"AMQPBasicProperties":private]=>
    string(0) ""
    ["expiration":"AMQPBasicProperties":private]=>
    string(0) ""
    ["message_id":"AMQPBasicProperties":private]=>
    string(0) ""
    ["timestamp":"AMQPBasicProperties":private]=>
    int(0)
    ["type":"AMQPBasicProperties":private]=>
    string(0) ""
    ["user_id":"AMQPBasicProperties":private]=>
    string(0) ""
    ["app_id":"AMQPBasicProperties":private]=>
    string(0) ""
    ["cluster_id":"AMQPBasicProperties":private]=>
    string(0) ""
  }
  [5]=>
  string(9) "message 2"
}
AMQPQueueException(0): Consumer timeout exceed
