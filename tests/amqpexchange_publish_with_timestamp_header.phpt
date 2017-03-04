--TEST--
AMQPExchange publish with properties - nested header values
--SKIPIF--
<?php if (!extension_loaded("amqp")) {
    print "skip";
} ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();

$ch = new AMQPChannel($cnn);

$ex = new AMQPExchange($ch);
$ex->setName("exchange-" . microtime(true));
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->declareExchange();

$q = new AMQPQueue($ch);
$q->setName('queue-' . microtime(true));
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
    string(10) "1488578462"
  }
}
array(1) {
  ["headerName"]=>
  object(AMQPTimestamp)#%d (1) {
    ["timestamp":"AMQPTimestamp":private]=>
    string(10) "1488578462"
  }
}
same

==DONE==