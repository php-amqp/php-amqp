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
  object(AMQPConnection)#1 (11) {
    ["login":"AMQPConnection":private]=>
    string(5) "guest"
    ["password":"AMQPConnection":private]=>
    string(5) "guest"
    ["host":"AMQPConnection":private]=>
    string(9) "localhost"
    ["vhost":"AMQPConnection":private]=>
    string(1) "/"
    ["port":"AMQPConnection":private]=>
    %s(5672)
    ["read_timeout":"AMQPConnection":private]=>
    %s(0)
    ["write_timeout":"AMQPConnection":private]=>
    %s(0)
    ["connect_timeout":"AMQPConnection":private]=>
    %s(0)
    ["channel_max":"AMQPConnection":private]=>
    %s(256)
    ["frame_max":"AMQPConnection":private]=>
    %s(131072)
    ["heartbeat":"AMQPConnection":private]=>
    %s(0)
  }
  ["channel":"AMQPQueue":private]=>
  object(AMQPChannel)#2 (3) {
    ["connection":"AMQPChannel":private]=>
    object(AMQPConnection)#1 (11) {
      ["login":"AMQPConnection":private]=>
      string(5) "guest"
      ["password":"AMQPConnection":private]=>
      string(5) "guest"
      ["host":"AMQPConnection":private]=>
      string(9) "localhost"
      ["vhost":"AMQPConnection":private]=>
      string(1) "/"
      ["port":"AMQPConnection":private]=>
      %s(5672)
      ["read_timeout":"AMQPConnection":private]=>
      %s(0)
      ["write_timeout":"AMQPConnection":private]=>
      %s(0)
      ["connect_timeout":"AMQPConnection":private]=>
      %s(0)
      ["channel_max":"AMQPConnection":private]=>
      %s(256)
      ["frame_max":"AMQPConnection":private]=>
      %s(131072)
      ["heartbeat":"AMQPConnection":private]=>
      %s(0)
    }
    ["prefetch_count":"AMQPChannel":private]=>
    %s(3)
    ["prefetch_size":"AMQPChannel":private]=>
    %s(0)
  }
  ["name":"AMQPQueue":private]=>
  string(%d) "test.queue.%f"
  ["consumer_tag":"AMQPQueue":private]=>
  NULL
  ["passive":"AMQPQueue":private]=>
  bool(false)
  ["durable":"AMQPQueue":private]=>
  bool(false)
  ["exclusive":"AMQPQueue":private]=>
  bool(false)
  ["auto_delete":"AMQPQueue":private]=>
  bool(true)
  ["arguments":"AMQPQueue":private]=>
  array(0) {
  }
}
object(AMQPQueue)#4 (9) {
  ["connection":"AMQPQueue":private]=>
  object(AMQPConnection)#1 (11) {
    ["login":"AMQPConnection":private]=>
    string(5) "guest"
    ["password":"AMQPConnection":private]=>
    string(5) "guest"
    ["host":"AMQPConnection":private]=>
    string(9) "localhost"
    ["vhost":"AMQPConnection":private]=>
    string(1) "/"
    ["port":"AMQPConnection":private]=>
    %s(5672)
    ["read_timeout":"AMQPConnection":private]=>
    %s(0)
    ["write_timeout":"AMQPConnection":private]=>
    %s(0)
    ["connect_timeout":"AMQPConnection":private]=>
    %s(0)
    ["channel_max":"AMQPConnection":private]=>
    %s(256)
    ["frame_max":"AMQPConnection":private]=>
    %s(131072)
    ["heartbeat":"AMQPConnection":private]=>
    %s(0)
  }
  ["channel":"AMQPQueue":private]=>
  object(AMQPChannel)#2 (3) {
    ["connection":"AMQPChannel":private]=>
    object(AMQPConnection)#1 (11) {
      ["login":"AMQPConnection":private]=>
      string(5) "guest"
      ["password":"AMQPConnection":private]=>
      string(5) "guest"
      ["host":"AMQPConnection":private]=>
      string(9) "localhost"
      ["vhost":"AMQPConnection":private]=>
      string(1) "/"
      ["port":"AMQPConnection":private]=>
      %s(5672)
      ["read_timeout":"AMQPConnection":private]=>
      %s(0)
      ["write_timeout":"AMQPConnection":private]=>
      %s(0)
      ["connect_timeout":"AMQPConnection":private]=>
      %s(0)
      ["channel_max":"AMQPConnection":private]=>
      %s(256)
      ["frame_max":"AMQPConnection":private]=>
      %s(131072)
      ["heartbeat":"AMQPConnection":private]=>
      %s(0)
    }
    ["prefetch_count":"AMQPChannel":private]=>
    %s(3)
    ["prefetch_size":"AMQPChannel":private]=>
    %s(0)
  }
  ["name":"AMQPQueue":private]=>
  string(%d) "test.queue.dead.%f"
  ["consumer_tag":"AMQPQueue":private]=>
  NULL
  ["passive":"AMQPQueue":private]=>
  bool(false)
  ["durable":"AMQPQueue":private]=>
  bool(false)
  ["exclusive":"AMQPQueue":private]=>
  bool(false)
  ["auto_delete":"AMQPQueue":private]=>
  bool(true)
  ["arguments":"AMQPQueue":private]=>
  array(3) {
    ["x-dead-letter-exchange"]=>
    string(0) ""
    ["x-dead-letter-routing-key"]=>
    string(%d) "test.queue.%f"
    ["x-message-ttl"]=>
    %s(100000)
  }
}
object(AMQPQueue)#4 (9) {
  ["connection":"AMQPQueue":private]=>
  object(AMQPConnection)#1 (11) {
    ["login":"AMQPConnection":private]=>
    string(5) "guest"
    ["password":"AMQPConnection":private]=>
    string(5) "guest"
    ["host":"AMQPConnection":private]=>
    string(9) "localhost"
    ["vhost":"AMQPConnection":private]=>
    string(1) "/"
    ["port":"AMQPConnection":private]=>
    %s(5672)
    ["read_timeout":"AMQPConnection":private]=>
    %s(0)
    ["write_timeout":"AMQPConnection":private]=>
    %s(0)
    ["connect_timeout":"AMQPConnection":private]=>
    %s(0)
    ["channel_max":"AMQPConnection":private]=>
    %s(256)
    ["frame_max":"AMQPConnection":private]=>
    %s(131072)
    ["heartbeat":"AMQPConnection":private]=>
    %s(0)
  }
  ["channel":"AMQPQueue":private]=>
  object(AMQPChannel)#2 (3) {
    ["connection":"AMQPChannel":private]=>
    object(AMQPConnection)#1 (11) {
      ["login":"AMQPConnection":private]=>
      string(5) "guest"
      ["password":"AMQPConnection":private]=>
      string(5) "guest"
      ["host":"AMQPConnection":private]=>
      string(9) "localhost"
      ["vhost":"AMQPConnection":private]=>
      string(1) "/"
      ["port":"AMQPConnection":private]=>
      %s(5672)
      ["read_timeout":"AMQPConnection":private]=>
      %s(0)
      ["write_timeout":"AMQPConnection":private]=>
      %s(0)
      ["connect_timeout":"AMQPConnection":private]=>
      %s(0)
      ["channel_max":"AMQPConnection":private]=>
      %s(256)
      ["frame_max":"AMQPConnection":private]=>
      %s(131072)
      ["heartbeat":"AMQPConnection":private]=>
      %s(0)
    }
    ["prefetch_count":"AMQPChannel":private]=>
    %s(3)
    ["prefetch_size":"AMQPChannel":private]=>
    %s(0)
  }
  ["name":"AMQPQueue":private]=>
  string(%d) "test.queue.dead.%f"
  ["consumer_tag":"AMQPQueue":private]=>
  NULL
  ["passive":"AMQPQueue":private]=>
  bool(false)
  ["durable":"AMQPQueue":private]=>
  bool(false)
  ["exclusive":"AMQPQueue":private]=>
  bool(false)
  ["auto_delete":"AMQPQueue":private]=>
  bool(true)
  ["arguments":"AMQPQueue":private]=>
  array(2) {
    ["x-dead-letter-exchange"]=>
    string(0) ""
    ["x-message-ttl"]=>
    %s(100000)
  }
}
