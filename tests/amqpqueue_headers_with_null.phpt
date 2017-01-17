--TEST--
AMQPQueue::get headers
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();

$ch = new AMQPChannel($cnn);

$ex = new AMQPExchange($ch);
$ex->setName('exchange' . microtime(true));
$ex->setType(AMQP_EX_TYPE_TOPIC);
$ex->declareExchange();
$q = new AMQPQueue($ch);
$q->setName('queue1' . microtime(true));
$q->declareQueue();
$q->bind($ex->getName(), '#');


// publish a message:
$ex->publish(
    'body', 'routing.1', AMQP_NOPARAM, array(
        'headers' => array(
            'foo' => 'bar',
            'null' => NULL
        )
    )
);

// Read from the queue
$msg = $q->get(AMQP_AUTOACK);

var_dump($msg->getHeaders());
echo $msg->getHeader('foo') . "\n";
var_dump($msg->getHeader('null'));

$ex->delete();
$q->delete();
?>
--EXPECT--
array(2) {
  ["foo"]=>
  string(3) "bar"
  ["null"]=>
  NULL
}
bar
NULL
