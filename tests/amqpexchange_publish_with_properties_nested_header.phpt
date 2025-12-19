--TEST--
AMQPExchange publish with properties - nested header values
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

$ex = new AMQPExchange($ch);
$ex->setName("exchange-" . bin2hex(random_bytes(32)));
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->declareExchange();

$q = new AMQPQueue($ch);
$q->setName('queue-' . bin2hex(random_bytes(32)));
$q->declareQueue();
$q->bind($ex->getName());

$headers = array(
    'nested' => array(
        'string'     => 'passed',
        999          => 'numeric works',
        'sub-nested' => array(
            'should' => 'works',
            42       => 'too'
        ),
    ),
);

$ex->publish('message', 'routing.key', AMQP_NOPARAM, array('headers' => $headers));

$message = $q->get(AMQP_AUTOACK);
var_dump($message->getHeaders());
var_dump($headers);
echo $message->getHeaders() === $headers ? 'same' : 'differs';
echo PHP_EOL, PHP_EOL;


$originalHeaders = array(
    'x-death' => array(
        array (
            'count' => 1,
            'reason' => 'rejected',
            'queue' => 'my_queue',
            'time' => 1410527691,
            'exchange' => 'my_exchange',
            'routing-keys' => array ('my_routing_key')
        )
    )
);

$ex->publish('message', 'routing.key', AMQP_NOPARAM, array('headers' => $originalHeaders));

$message = $q->get(AMQP_AUTOACK);
$messageHeaders = $message->getHeaders();
ksort($messageHeaders);
var_dump($originalHeaders);
var_dump($messageHeaders);
echo $messageHeaders['x-death'][0]['time']->getTimestamp() === (float) $originalHeaders['x-death'][0]['time'] ? "timestamp matches\n" : "timestamp differs\n";
unset($messageHeaders['x-death'][0]['time'], $originalHeaders['x-death'][0]['time']);
echo $messageHeaders === $originalHeaders ? "headers (except timestamp) identical\n" : "headers (except timestamp) also differ\n";
echo PHP_EOL, PHP_EOL;

?>
--EXPECT--
array(1) {
  ["nested"]=>
  array(3) {
    ["string"]=>
    string(6) "passed"
    [999]=>
    string(13) "numeric works"
    ["sub-nested"]=>
    array(2) {
      ["should"]=>
      string(5) "works"
      [42]=>
      string(3) "too"
    }
  }
}
array(1) {
  ["nested"]=>
  array(3) {
    ["string"]=>
    string(6) "passed"
    [999]=>
    string(13) "numeric works"
    ["sub-nested"]=>
    array(2) {
      ["should"]=>
      string(5) "works"
      [42]=>
      string(3) "too"
    }
  }
}
same

array(1) {
  ["x-death"]=>
  array(1) {
    [0]=>
    array(6) {
      ["count"]=>
      int(1)
      ["reason"]=>
      string(8) "rejected"
      ["queue"]=>
      string(8) "my_queue"
      ["time"]=>
      int(1410527691)
      ["exchange"]=>
      string(11) "my_exchange"
      ["routing-keys"]=>
      array(1) {
        [0]=>
        string(14) "my_routing_key"
      }
    }
  }
}
array(1) {
  ["x-death"]=>
  array(1) {
    [0]=>
    array(6) {
      ["count"]=>
      int(1)
      ["reason"]=>
      string(8) "rejected"
      ["queue"]=>
      string(8) "my_queue"
      ["time"]=>
      object(AMQPTimestamp)#7 (1) {
        ["timestamp":"AMQPTimestamp":private]=>
        float(1410527691)
      }
      ["exchange"]=>
      string(11) "my_exchange"
      ["routing-keys"]=>
      array(1) {
        [0]=>
        string(14) "my_routing_key"
      }
    }
  }
}
timestamp matches
headers (except timestamp) identical
