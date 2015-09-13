--TEST--
AMQPQueue::get headers
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();

$ch = new AMQPChannel($cnn);

// Declare a new exchange
$ex = new AMQPExchange($ch);
$ex->setName('exchange' . microtime(true));
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->declareExchange();

// Create a new queue
$q = new AMQPQueue($ch);
$q->setName('queue1' . microtime(true));
$q->declareQueue();

// Bind it on the exchange to routing.key
$q->bind($ex->getName(), 'routing.*');

// Publish a message to the exchange with a routing key
$ex->publish('body', 'routing.1', AMQP_NOPARAM, array("headers" => array(
	"first" => "one",
	"second" => 2
)));

// Read from the queue
$msg = $q->get(AMQP_AUTOACK);

echo $msg->getBody() . "\n";
var_dump($msg->getHeaders());
echo $msg->getContentType() . "\n";

var_dump($msg->hasHeader("first"));
echo $msg->getHeader("first") . "\n";
echo $msg->getHeader("second") . "\n";

echo 'Has nonexistent header: ', var_export($msg->hasHeader("nonexistent"), true), PHP_EOL;
echo 'Get nonexistent header: ', var_export($msg->getHeader("nonexistent"), true), PHP_EOL;

$ex->delete();
$q->delete();
?>
--EXPECT--
body
array(2) {
  ["first"]=>
  string(3) "one"
  ["second"]=>
  int(2)
}
text/plain
bool(true)
one
2
Has nonexistent header: false
Get nonexistent header: false
