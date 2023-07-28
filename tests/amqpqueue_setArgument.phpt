--TEST--
AMQPQueue::setArgument() test
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$credentials = array();

$cnn = new AMQPConnection($credentials);
$cnn->connect();

$ch = new AMQPChannel($cnn);

$heartbeat   = 10;
$q_dead_name = 'test.queue.dead.' . microtime(true);
$q_name      = 'test.queue.' . microtime(true);

$q = new AMQPQueue($ch);
$q->setName($q_name);
$q->setFlags(AMQP_AUTODELETE);
$q->declareQueue();

var_dump($q);


$q_dead = new AMQPQueue($ch);
$q_dead->setName($q_dead_name);
$q_dead->setArgument('x-dead-letter-exchange', '');
$q_dead->setArgument('x-dead-letter-routing-key', $q_name);
$q_dead->setArgument('x-message-ttl', $heartbeat * 10 * 1000);
$q_dead->setFlags(AMQP_AUTODELETE);
$q_dead->declareQueue();

var_dump($q_dead);
$q_dead->setArgument('x-dead-letter-routing-key', null); // removes this key
var_dump($q_dead);
?>
--EXPECTF--
object(AMQPQueue)#3 (9) {
  ["connection":"AMQPQueue":private]=>
  %a
  ["channel":"AMQPQueue":private]=>
  %a
  ["name":"AMQPQueue":private]=>
  string(%d) "test.queue.%f"
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
object(AMQPQueue)#4 (9) {
  ["connection":"AMQPQueue":private]=>
  %a
  ["channel":"AMQPQueue":private]=>
  %a
  ["name":"AMQPQueue":private]=>
  string(%d) "test.queue.dead.%f"
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
  array(3) {
    ["x-dead-letter-exchange"]=>
    string(0) ""
    ["x-dead-letter-routing-key"]=>
    string(%d) "test.queue.%f"
    ["x-message-ttl"]=>
    int(100000)
  }
}
object(AMQPQueue)#4 (9) {
  ["connection":"AMQPQueue":private]=>
  %a
  ["channel":"AMQPQueue":private]=>
  %a
  ["name":"AMQPQueue":private]=>
  string(%d) "test.queue.dead.%f"
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
  array(2) {
    ["x-dead-letter-exchange"]=>
    string(0) ""
    ["x-message-ttl"]=>
    int(100000)
  }
}
