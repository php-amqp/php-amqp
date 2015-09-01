--TEST--
AMQPConnection heartbeats support (with active consumer)
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$heartbeat = 2;
$credentials = array('heartbeat' => $heartbeat, 'read_timeout' => $heartbeat * 20);
$cnn = new AMQPConnection($credentials);
$cnn->connect();

debug_zval_dump($cnn);

$ch = new AMQPChannel($cnn);

$q_dead_name = 'test.queue.dead.' . microtime(true);
$q_name      = 'test.queue.' . microtime(true);

$e = new AMQPExchange($ch);

$q = new AMQPQueue($ch);
$q->setName($q_name);
$q->declareQueue();


$q_dead = new AMQPQueue($ch);
$q_dead->setName($q_dead_name);
$q_dead->setArgument('x-dead-letter-exchange', '');
$q_dead->setArgument('x-dead-letter-routing-key', $q_name);
$q_dead->setArgument('x-message-ttl', $heartbeat * 10 * 1000);
$q_dead->declareQueue();

$e->publish('test message 1 (should be dead lettered)', $q_dead_name);

$t = microtime(true);
$q->consume(function (AMQPEnvelope $envelope) {
  echo 'Consumed: ', $envelope->getBody(), PHP_EOL;
  return false;
});

echo 'Consuming took: ', (float) round(microtime(true) - $t, 4), 'sec', PHP_EOL;

$ch2 = new AMQPChannel($cnn);

sleep($heartbeat/2);

$ch3 = new AMQPChannel($cnn);

echo 'Done', PHP_EOL

?>
--EXPECTF--
object(AMQPConnection)#1 (15) refcount(2){
  ["login"]=>
  string(5) "guest" refcount(1)
  ["password"]=>
  string(5) "guest" refcount(1)
  ["host"]=>
  string(9) "localhost" refcount(1)
  ["vhost"]=>
  string(1) "/" refcount(1)
  ["port"]=>
  int(5672)
  ["read_timeout"]=>
  float(40)
  ["write_timeout"]=>
  float(0)
  ["connect_timeout"]=>
  float(0)
  ["is_connected"]=>
  bool(true)
  ["is_persistent"]=>
  bool(false)
  ["connection_resource"]=>
  resource(4) of type (AMQP Connection Resource) refcount(3)
  ["used_channels"]=>
  int(0)
  ["max_channel_id"]=>
  int(256)
  ["max_frame_size"]=>
  int(131072)
  ["heartbeat_interval"]=>
  int(2)
}
Consumed: test message 1 (should be dead lettered)
Consuming took: 20%fsec
Done