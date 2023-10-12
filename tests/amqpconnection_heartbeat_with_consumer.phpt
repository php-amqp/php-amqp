--TEST--
AMQPConnection heartbeats support (with active consumer)
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
elseif (!getenv("PHP_AMQP_HOST")) print "skip PHP_AMQP_HOST environment variable is not set";
elseif (getenv("SKIP_SLOW_TESTS")) print "skip slow test and SKIP_SLOW_TESTS is set";
?>
--FILE--
<?php
$heartbeat = 2;
$credentials = array('heartbeat' => $heartbeat, 'read_timeout' => $heartbeat * 20);
$cnn = new AMQPConnection($credentials);
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->connect();

var_dump($cnn);

$ch = new AMQPChannel($cnn);

$q_dead_name = 'test.queue.dead.' . bin2hex(random_bytes(32));
$q_name      = 'test.queue.' . bin2hex(random_bytes(32));

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
object(AMQPConnection)#1 (18) {
  ["login":"AMQPConnection":private]=>
  string(5) "guest"
  ["password":"AMQPConnection":private]=>
  string(5) "guest"
  ["host":"AMQPConnection":private]=>
  string(%d) "%s"
  ["vhost":"AMQPConnection":private]=>
  string(1) "/"
  ["port":"AMQPConnection":private]=>
  int(5672)
  ["readTimeout":"AMQPConnection":private]=>
  float(40)
  ["writeTimeout":"AMQPConnection":private]=>
  float(0)
  ["connectTimeout":"AMQPConnection":private]=>
  float(0)
  ["rpcTimeout":"AMQPConnection":private]=>
  float(0)
  ["frameMax":"AMQPConnection":private]=>
  int(131072)
  ["channelMax":"AMQPConnection":private]=>
  int(256)
  ["heartbeat":"AMQPConnection":private]=>
  int(2)
  ["cacert":"AMQPConnection":private]=>
  NULL
  ["key":"AMQPConnection":private]=>
  NULL
  ["cert":"AMQPConnection":private]=>
  NULL
  ["verify":"AMQPConnection":private]=>
  bool(true)
  ["saslMethod":"AMQPConnection":private]=>
  int(0)
  ["connectionName":"AMQPConnection":private]=>
  NULL
}
Consumed: test message 1 (should be dead lettered)
Consuming took: %fsec
Timing OK (%f < %f < %f)
Done
