--TEST--
AMQPExchange publish with timestamp header
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

$headers = ['headerName' => new AMQPTimestamp(1488578462)];

$ex->publish('message', 'routing.key', AMQP_NOPARAM, array('headers' => $headers));

$message =$q->get(AMQP_AUTOACK);
var_dump($message->getHeaders());
var_dump($headers);
echo $message->getHeaders() == $headers ? 'same' : 'differs';
?>


==DONE==
--EXPECTF--
array(1) {
  ["headerName"]=>
  object(AMQPTimestamp)#%d (1) {
    ["timestamp":"AMQPTimestamp":private]=>
    float(1488578462)
  }
}
array(1) {
  ["headerName"]=>
  object(AMQPTimestamp)#%d (1) {
    ["timestamp":"AMQPTimestamp":private]=>
    float(1488578462)
  }
}
same

==DONE==
