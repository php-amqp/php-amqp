--TEST--
AMQPQueue::setArgument() test
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
elseif (!getenv("PHP_AMQP_HOST")) print "skip PHP_AMQP_HOST environment variable is not set";
?>
--FILE--
<?php
$credentials = array();

$cnn = new AMQPConnection($credentials);
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->connect();

$ch = new AMQPChannel($cnn);

$heartbeat   = 10;
$q_dead_name = 'test.queue.dead.' . bin2hex(random_bytes(32));
$q_name      = 'test.queue.' . bin2hex(random_bytes(32));

$q = new AMQPQueue($ch);
$q->setName($q_name);
$q->setFlags(AMQP_AUTODELETE);
$q->declareQueue();

var_dump($q);

class MyValue implements AMQPValue {
    public function toAmqpValue() { return "foo"; }
}

$q_dead = new AMQPQueue($ch);
$q_dead->setName($q_dead_name);
$q_dead->setArgument('x-dead-letter-exchange', '');
$q_dead->setArgument('x-dead-letter-routing-key', $q_name);
$q_dead->setArgument('x-message-ttl', $heartbeat * 10 * 1000);
$q_dead->setArgument('x-null', null);
$q_dead->setArgument('x-array', [0, 3]);
$q_dead->setArgument('x-hash', ['foo' => 'bar']);
$q_dead->setArgument('x-timestamp', new AMQPTimestamp(404));
$q_dead->setArgument('x-decimal', new AMQPDecimal(1, 2));
$q_dead->setArgument('x-custom-value', new MyValue());
$q_dead->setFlags(AMQP_AUTODELETE);
$q_dead->declareQueue();

var_dump($q_dead);
$q_dead->removeArgument('x-null');
$q_dead->removeArgument('x-does-not-exist');
var_dump($q_dead);
?>
--EXPECTF--
object(AMQPQueue)#%d (%d) {
  ["connection":"AMQPQueue":private]=>
  %a
  ["channel":"AMQPQueue":private]=>
  %a
  ["name":"AMQPQueue":private]=>
  string(%d) "test.queue.%s"
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
object(AMQPQueue)#%d (%d) {
  ["connection":"AMQPQueue":private]=>
  %a
  ["channel":"AMQPQueue":private]=>
  %a
  ["name":"AMQPQueue":private]=>
  string(%d) "test.queue.dead.%s"
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
  array(%d) {
    ["x-dead-letter-exchange"]=>
    string(0) ""
    ["x-dead-letter-routing-key"]=>
    string(%d) "test.queue.%s"
    ["x-message-ttl"]=>
    int(100000)
    ["x-null"]=>
    NULL
    ["x-array"]=>
    array(2) {
      [0]=>
      int(0)
      [1]=>
      int(3)
    }
    ["x-hash"]=>
    array(1) {
      ["foo"]=>
      string(3) "bar"
    }
    ["x-timestamp"]=>
    object(AMQPTimestamp)#%d (%d) {
      ["timestamp":"AMQPTimestamp":private]=>
      float(404)
    }
    ["x-decimal"]=>
    object(AMQPDecimal)#%d (%d) {
      ["exponent":"AMQPDecimal":private]=>
      int(1)
      ["significand":"AMQPDecimal":private]=>
      int(2)
    }
    ["x-custom-value"]=>
    object(MyValue)#%d (%d) {
    }
  }
}
object(AMQPQueue)#%d (%d) {
  ["connection":"AMQPQueue":private]=>
  %a
  ["channel":"AMQPQueue":private]=>
  %a
  ["name":"AMQPQueue":private]=>
  string(%d) "test.queue.dead.%s"
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
  array(%d) {
    ["x-dead-letter-exchange"]=>
    string(0) ""
    ["x-dead-letter-routing-key"]=>
    string(%d) "test.queue.%s"
    ["x-message-ttl"]=>
    int(100000)
    ["x-array"]=>
    array(2) {
      [0]=>
      int(0)
      [1]=>
      int(3)
    }
    ["x-hash"]=>
    array(1) {
      ["foo"]=>
      string(3) "bar"
    }
    ["x-timestamp"]=>
    object(AMQPTimestamp)#%d (%d) {
      ["timestamp":"AMQPTimestamp":private]=>
      float(404)
    }
    ["x-decimal"]=>
    object(AMQPDecimal)#%d (%d) {
      ["exponent":"AMQPDecimal":private]=>
      int(1)
      ["significand":"AMQPDecimal":private]=>
      int(2)
    }
    ["x-custom-value"]=>
    object(MyValue)#%d (%d) {
    }
  }
}
