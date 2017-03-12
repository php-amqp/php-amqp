--TEST--
AMQPExchange publish with decimal header
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

$headers = ['headerName' => new AMQPDecimal(123, 456)];

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
  object(AMQPDecimal)#7 (2) {
    ["exponent":"AMQPDecimal":private]=>
    int(123)
    ["significand":"AMQPDecimal":private]=>
    int(456)
  }
}
array(1) {
  ["headerName"]=>
  object(AMQPDecimal)#5 (2) {
    ["exponent":"AMQPDecimal":private]=>
    int(123)
    ["significand":"AMQPDecimal":private]=>
    int(456)
  }
}
same

==DONE==
