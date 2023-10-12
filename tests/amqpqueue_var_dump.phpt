--TEST--
AMQPQueue var_dump
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
elseif (!getenv("PHP_AMQP_HOST")) print "skip PHP_AMQP_HOST environment variable is not set";
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->connect();
$ch = new AMQPChannel($cnn);
// Declare a new exchange
$ex = new AMQPExchange($ch);
$ex->setName('exchange1');
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->declareExchange();
// Create a new queue
$q = new AMQPQueue($ch);
$q->setName('queue_var_dump');
$q->declareQueue();
var_dump($q);
?>
--EXPECTF--
object(AMQPQueue)#4 (9) {
  ["connection":"AMQPQueue":private]=>
  %a
  ["channel":"AMQPQueue":private]=>
  %a
  ["name":"AMQPQueue":private]=>
  string(14) "queue_var_dump"
  ["consumerTag":"AMQPQueue":private]=>
  NULL
  ["passive":"AMQPQueue":private]=>
  bool(false)
  ["durable":"AMQPQueue":private]=>
  bool(false)
  ["exclusive":"AMQPQueue":private]=>
  bool(false)
  ["autoDelete":"AMQPQueue":private]=>
  bool(true)
  ["arguments":"AMQPQueue":private]=>
  array(0) {
  }
}
