--TEST--
AMQPQueue::get headers
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip";
if (!getenv("PHP_AMQP_HOST")) print "skip";
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->connect();

$ch = new AMQPChannel($cnn);

// create an error exchange and bind a queue to it:
$errorXchange = new AMQPExchange($ch);
$errorXchange->setName('errorXchange' . bin2hex(random_bytes(32)));
$errorXchange->setType(AMQP_EX_TYPE_TOPIC);
$errorXchange->declareExchange();

$errorQ = new AMQPQueue($ch);
$errorQ->setName('errorQueue' . bin2hex(random_bytes(32)));
$errorQ->declareQueue();
$errorQ->bind($errorXchange->getName(), '#');


// Declare a new exchange and queue using this dead-letter-exchange:
$ex = new AMQPExchange($ch);
$ex->setName('exchange-' . bin2hex(random_bytes(32)));
$ex->setType(AMQP_EX_TYPE_TOPIC);
$ex->declareExchange();
$q = new AMQPQueue($ch);
$q->setName('queue-' . bin2hex(random_bytes(32)));
$q->setArgument('x-dead-letter-exchange', $errorXchange->getName());
$q->declareQueue();
$q->bind($ex->getName(), '#');


// publish a message:
$ex->publish(
    'body', 'routing.1', AMQP_NOPARAM, array(
        'headers' => array(
            'foo' => 'fval',
            'bar' => array(array('aa', 'bb', array('bar_nested' => 'nested'))),
            'baz' => array('a', 'bc', 'def', 123, 'g'),
            'bla' => array('one' => 2),
        )
    )
);

// Read from the queue and reject, so it gets dead-lettered:
$msg = $q->get(AMQP_NOPARAM);
$q->nack($msg->getDeliveryTag());

sleep(1);
// Now read from the error queue:
$msg = $errorQ->get(AMQP_AUTOACK);

$header = $msg->getHeader('x-death');

echo isset($header[0]['count']) ? 'count found' : 'count not found', PHP_EOL;

unset($header[0]['count']);

var_dump($header);
var_dump($msg->getHeader('foo'));
var_dump($msg->getHeader('bar'));
var_dump($msg->getHeader('baz'));
var_dump($msg->getHeader('bla'));

$ex->delete();
$q->delete();
$errorXchange->delete();
$errorQ->delete();
?>
==DONE==
--EXPECTF--
count %s
array(1) {
  [0]=>
  array(5) {
    ["reason"]=>
    string(8) "rejected"
    ["queue"]=>
    string(%d) "queue-%s"
    ["time"]=>
    object(AMQPTimestamp)#%d (1) {
      ["timestamp":"AMQPTimestamp":private]=>
      float(%d)
    }
    ["exchange"]=>
    string(%d) "exchange-%s"
    ["routing-keys"]=>
    array(1) {
      [0]=>
      string(9) "routing.1"
    }
  }
}
string(4) "fval"
array(1) {
  [0]=>
  array(3) {
    [0]=>
    string(2) "aa"
    [1]=>
    string(2) "bb"
    [2]=>
    array(1) {
      ["bar_nested"]=>
      string(6) "nested"
    }
  }
}
array(5) {
  [0]=>
  string(1) "a"
  [1]=>
  string(2) "bc"
  [2]=>
  string(3) "def"
  [3]=>
  int(123)
  [4]=>
  string(1) "g"
}
array(1) {
  ["one"]=>
  int(2)
}
==DONE==