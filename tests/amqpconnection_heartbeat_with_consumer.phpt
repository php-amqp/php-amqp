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
$t = microtime(true) - $t;

echo 'Consuming took: ', (float) round($t, 4), 'sec', PHP_EOL;

$t_min = (float)round($heartbeat * 9.5, 4);
$t_max = (float)round($heartbeat * 10.5, 4);

if ($t > $t_min && $t < $t_max) {
  echo "Timing OK ($t_min < $t < $t_max)", PHP_EOL;
} else {
  echo "Timing ERROR ($t_min < $t < $t_max)", PHP_EOL;
}

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
  long(5672) refcount(1)
  ["read_timeout"]=>
  double(40) refcount(1)
  ["write_timeout"]=>
  double(0) refcount(1)
  ["connect_timeout"]=>
  double(0) refcount(1)
  ["is_connected"]=>
  bool(true) refcount(1)
  ["is_persistent"]=>
  bool(false) refcount(1)
  ["connection_resource"]=>
  resource(4) of type (AMQP Connection Resource) refcount(1)
  ["used_channels"]=>
  long(0) refcount(1)
  ["max_channel_id"]=>
  long(256) refcount(1)
  ["max_frame_size"]=>
  long(131072) refcount(1)
  ["heartbeat_interval"]=>
  long(2) refcount(1)
}
Consumed: test message 1 (should be dead lettered)
Consuming took: %fsec
Timing OK (%f < %f < %f)
Done