--TEST--
AMQPQueue::get headers with float values
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
            'positive' => 2.3,
            'negative' => -1022.123456789,
            'scientific' => 10E2,
            'scientific_big' => 10E23,
        )
    )
);

// Read from the queue
$msg = $q->get(AMQP_AUTOACK);

var_dump($msg->getHeaders());
echo $msg->getHeader('foo') . "\n";
var_dump($msg->hasHeader('positive'), $msg->getHeader('positive'));
var_dump($msg->hasHeader('negative'), $msg->getHeader('negative'));
var_dump($msg->hasHeader('scientific'), $msg->getHeader('scientific'));
var_dump($msg->hasHeader('scientific_big'), $msg->getHeader('scientific_big'));

$ex->delete();
$q->delete();
?>
--EXPECT--
array(5) {
  ["foo"]=>
  string(3) "bar"
  ["positive"]=>
  float(2.3)
  ["negative"]=>
  float(-1022.123456789)
  ["scientific"]=>
  float(1000)
  ["scientific_big"]=>
  float(1.0E+24)
}
bar
bool(true)
float(2.3)
bool(true)
float(-1022.123456789)
bool(true)
float(1000)
bool(true)
float(1.0E+24)
