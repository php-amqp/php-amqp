--TEST--
AMQPQueue::consume multiple (no doubles)
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$time = microtime(true);

$cnn = new AMQPConnection();
$cnn->connect();

$ch = new AMQPChannel($cnn);

// Create a new queue
$q = new AMQPQueue($ch);
$q->setName('queue-' . microtime(true));
$q->declareQueue();

var_dump($q->getConsumerTag());
$q->consume();
var_dump($tag1=$q->getConsumerTag());
$q->consume(null, AMQP_JUST_CONSUME);
var_dump($tag2=$q->getConsumerTag());

echo ($tag1 == $tag2 ? 'equals' : 'differs'), PHP_EOL;


?>
--EXPECTF--
NULL
string(%d) "amq.ctag-%s"
string(%d) "amq.ctag-%s"
equals