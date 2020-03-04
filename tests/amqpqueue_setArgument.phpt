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
  object(AMQPConnection)#1 (18) {
    ["login":"AMQPConnection":private]=>
    string(5) "guest"
    ["password":"AMQPConnection":private]=>
    string(5) "guest"
    ["host":"AMQPConnection":private]=>
    string(9) "localhost"
    ["vhost":"AMQPConnection":private]=>
    string(1) "/"
    ["port":"AMQPConnection":private]=>
    int(5672)
    ["read_timeout":"AMQPConnection":private]=>
    float(0)
    ["write_timeout":"AMQPConnection":private]=>
    float(0)
    ["connect_timeout":"AMQPConnection":private]=>
    float(0)
    ["rpc_timeout":"AMQPConnection":private]=>
    float(0)
    ["channel_max":"AMQPConnection":private]=>
    int(256)
    ["frame_max":"AMQPConnection":private]=>
    int(131072)
    ["heartbeat":"AMQPConnection":private]=>
    int(0)
    ["cacert":"AMQPConnection":private]=>
    string(0) ""
    ["key":"AMQPConnection":private]=>
    string(0) ""
    ["cert":"AMQPConnection":private]=>
    string(0) ""
    ["verify":"AMQPConnection":private]=>
    bool(true)
    ["sasl_method":"AMQPConnection":private]=>
    int(0)
    ["connection_name":"AMQPConnection":private]=>
    NULL
  }
  ["channel":"AMQPQueue":private]=>
  object(AMQPChannel)#2 (6) {
    ["connection":"AMQPChannel":private]=>
    object(AMQPConnection)#1 (18) {
      ["login":"AMQPConnection":private]=>
      string(5) "guest"
      ["password":"AMQPConnection":private]=>
      string(5) "guest"
      ["host":"AMQPConnection":private]=>
      string(9) "localhost"
      ["vhost":"AMQPConnection":private]=>
      string(1) "/"
      ["port":"AMQPConnection":private]=>
      int(5672)
      ["read_timeout":"AMQPConnection":private]=>
      float(0)
      ["write_timeout":"AMQPConnection":private]=>
      float(0)
      ["connect_timeout":"AMQPConnection":private]=>
      float(0)
      ["rpc_timeout":"AMQPConnection":private]=>
      float(0)
      ["channel_max":"AMQPConnection":private]=>
      int(256)
      ["frame_max":"AMQPConnection":private]=>
      int(131072)
      ["heartbeat":"AMQPConnection":private]=>
      int(0)
      ["cacert":"AMQPConnection":private]=>
      string(0) ""
      ["key":"AMQPConnection":private]=>
      string(0) ""
      ["cert":"AMQPConnection":private]=>
      string(0) ""
      ["verify":"AMQPConnection":private]=>
      bool(true)
      ["sasl_method":"AMQPConnection":private]=>
      int(0)
      ["connection_name":"AMQPConnection":private]=>
      NULL
    }
    ["prefetch_count":"AMQPChannel":private]=>
    int(3)
    ["prefetch_size":"AMQPChannel":private]=>
    int(0)
    ["global_prefetch_count":"AMQPChannel":private]=>
    int(0)
    ["global_prefetch_size":"AMQPChannel":private]=>
    int(0)
    ["consumers":"AMQPChannel":private]=>
    array(0) {
    }
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
  object(AMQPConnection)#1 (18) {
    ["login":"AMQPConnection":private]=>
    string(5) "guest"
    ["password":"AMQPConnection":private]=>
    string(5) "guest"
    ["host":"AMQPConnection":private]=>
    string(9) "localhost"
    ["vhost":"AMQPConnection":private]=>
    string(1) "/"
    ["port":"AMQPConnection":private]=>
    int(5672)
    ["read_timeout":"AMQPConnection":private]=>
    float(0)
    ["write_timeout":"AMQPConnection":private]=>
    float(0)
    ["connect_timeout":"AMQPConnection":private]=>
    float(0)
    ["rpc_timeout":"AMQPConnection":private]=>
    float(0)
    ["channel_max":"AMQPConnection":private]=>
    int(256)
    ["frame_max":"AMQPConnection":private]=>
    int(131072)
    ["heartbeat":"AMQPConnection":private]=>
    int(0)
    ["cacert":"AMQPConnection":private]=>
    string(0) ""
    ["key":"AMQPConnection":private]=>
    string(0) ""
    ["cert":"AMQPConnection":private]=>
    string(0) ""
    ["verify":"AMQPConnection":private]=>
    bool(true)
    ["sasl_method":"AMQPConnection":private]=>
    int(0)
    ["connection_name":"AMQPConnection":private]=>
    NULL
  }
  ["channel":"AMQPQueue":private]=>
  object(AMQPChannel)#2 (6) {
    ["connection":"AMQPChannel":private]=>
    object(AMQPConnection)#1 (18) {
      ["login":"AMQPConnection":private]=>
      string(5) "guest"
      ["password":"AMQPConnection":private]=>
      string(5) "guest"
      ["host":"AMQPConnection":private]=>
      string(9) "localhost"
      ["vhost":"AMQPConnection":private]=>
      string(1) "/"
      ["port":"AMQPConnection":private]=>
      int(5672)
      ["read_timeout":"AMQPConnection":private]=>
      float(0)
      ["write_timeout":"AMQPConnection":private]=>
      float(0)
      ["connect_timeout":"AMQPConnection":private]=>
      float(0)
      ["rpc_timeout":"AMQPConnection":private]=>
      float(0)
      ["channel_max":"AMQPConnection":private]=>
      int(256)
      ["frame_max":"AMQPConnection":private]=>
      int(131072)
      ["heartbeat":"AMQPConnection":private]=>
      int(0)
      ["cacert":"AMQPConnection":private]=>
      string(0) ""
      ["key":"AMQPConnection":private]=>
      string(0) ""
      ["cert":"AMQPConnection":private]=>
      string(0) ""
      ["verify":"AMQPConnection":private]=>
      bool(true)
      ["sasl_method":"AMQPConnection":private]=>
      int(0)
      ["connection_name":"AMQPConnection":private]=>
      NULL
    }
    ["prefetch_count":"AMQPChannel":private]=>
    int(3)
    ["prefetch_size":"AMQPChannel":private]=>
    int(0)
    ["global_prefetch_count":"AMQPChannel":private]=>
    int(0)
    ["global_prefetch_size":"AMQPChannel":private]=>
    int(0)
    ["consumers":"AMQPChannel":private]=>
    array(0) {
    }
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
    int(100000)
  }
}
object(AMQPQueue)#4 (9) {
  ["connection":"AMQPQueue":private]=>
  object(AMQPConnection)#1 (18) {
    ["login":"AMQPConnection":private]=>
    string(5) "guest"
    ["password":"AMQPConnection":private]=>
    string(5) "guest"
    ["host":"AMQPConnection":private]=>
    string(9) "localhost"
    ["vhost":"AMQPConnection":private]=>
    string(1) "/"
    ["port":"AMQPConnection":private]=>
    int(5672)
    ["read_timeout":"AMQPConnection":private]=>
    float(0)
    ["write_timeout":"AMQPConnection":private]=>
    float(0)
    ["connect_timeout":"AMQPConnection":private]=>
    float(0)
    ["rpc_timeout":"AMQPConnection":private]=>
    float(0)
    ["channel_max":"AMQPConnection":private]=>
    int(256)
    ["frame_max":"AMQPConnection":private]=>
    int(131072)
    ["heartbeat":"AMQPConnection":private]=>
    int(0)
    ["cacert":"AMQPConnection":private]=>
    string(0) ""
    ["key":"AMQPConnection":private]=>
    string(0) ""
    ["cert":"AMQPConnection":private]=>
    string(0) ""
    ["verify":"AMQPConnection":private]=>
    bool(true)
    ["sasl_method":"AMQPConnection":private]=>
    int(0)
    ["connection_name":"AMQPConnection":private]=>
    NULL
  }
  ["channel":"AMQPQueue":private]=>
  object(AMQPChannel)#2 (6) {
    ["connection":"AMQPChannel":private]=>
    object(AMQPConnection)#1 (18) {
      ["login":"AMQPConnection":private]=>
      string(5) "guest"
      ["password":"AMQPConnection":private]=>
      string(5) "guest"
      ["host":"AMQPConnection":private]=>
      string(9) "localhost"
      ["vhost":"AMQPConnection":private]=>
      string(1) "/"
      ["port":"AMQPConnection":private]=>
      int(5672)
      ["read_timeout":"AMQPConnection":private]=>
      float(0)
      ["write_timeout":"AMQPConnection":private]=>
      float(0)
      ["connect_timeout":"AMQPConnection":private]=>
      float(0)
      ["rpc_timeout":"AMQPConnection":private]=>
      float(0)
      ["channel_max":"AMQPConnection":private]=>
      int(256)
      ["frame_max":"AMQPConnection":private]=>
      int(131072)
      ["heartbeat":"AMQPConnection":private]=>
      int(0)
      ["cacert":"AMQPConnection":private]=>
      string(0) ""
      ["key":"AMQPConnection":private]=>
      string(0) ""
      ["cert":"AMQPConnection":private]=>
      string(0) ""
      ["verify":"AMQPConnection":private]=>
      bool(true)
      ["sasl_method":"AMQPConnection":private]=>
      int(0)
      ["connection_name":"AMQPConnection":private]=>
      NULL
    }
    ["prefetch_count":"AMQPChannel":private]=>
    int(3)
    ["prefetch_size":"AMQPChannel":private]=>
    int(0)
    ["global_prefetch_count":"AMQPChannel":private]=>
    int(0)
    ["global_prefetch_size":"AMQPChannel":private]=>
    int(0)
    ["consumers":"AMQPChannel":private]=>
    array(0) {
    }
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
    int(100000)
  }
}
