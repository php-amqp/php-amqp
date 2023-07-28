--TEST--
AMQPQueue - orphaned envelope
--SKIPIF--
<?php if (!extension_loaded("amqp")) {
    print "skip";
} ?>
--FILE--
<?php
$connection = new AMQPConnection();
$connection->connect();

$channel1 = new AMQPChannel($connection);

$ex1 = new AMQPExchange($channel1);
$ex1->setName('ex1-' . microtime(true));
$ex1->setType(AMQP_EX_TYPE_FANOUT);
$ex1->declareExchange();

$q1 = new AMQPQueue($channel1);
$q1->setName('q1-' . microtime(true));
$q1->declareQueue();
$q1->bind($ex1->getName());

$ex1->publish("test passed");
$ex1->publish("test orphaned");

$q1->consume(function (AMQPEnvelope $message, AMQPQueue $queue) {
    $queue->ack($message->getDeliveryTag());
    return false;
});

$q1->cancel();

$q1 = null;

$q2 = new AMQPQueue($channel1);
$q2->setName('q1-' . microtime(true));
$q2->declareQueue();
$q2->bind($ex1->getName());


try {
    $q2->consume(function (AMQPEnvelope $message, AMQPQueue $queue) {
        $queue->ack($message->getDeliveryTag());
        return false;
    });

} catch (AMQPEnvelopeException $e) {
    echo  get_class($e), ': ', $e->getMessage(), ':', PHP_EOL, PHP_EOL;
    var_dump($e->envelope);
}

?>
--EXPECTF--
AMQPEnvelopeException: Orphaned envelope:

object(AMQPEnvelope)#6 (20) {
  ["contentType":"AMQPBasicProperties":private]=>
  string(10) "text/plain"
  ["contentEncoding":"AMQPBasicProperties":private]=>
  string(0) ""
  ["headers":"AMQPBasicProperties":private]=>
  array(0) {
  }
  ["deliveryMode":"AMQPBasicProperties":private]=>
  int(1)
  ["priority":"AMQPBasicProperties":private]=>
  int(0)
  ["correlationId":"AMQPBasicProperties":private]=>
  string(0) ""
  ["replyTo":"AMQPBasicProperties":private]=>
  string(0) ""
  ["expiration":"AMQPBasicProperties":private]=>
  string(0) ""
  ["messageId":"AMQPBasicProperties":private]=>
  string(0) ""
  ["timestamp":"AMQPBasicProperties":private]=>
  int(0)
  ["type":"AMQPBasicProperties":private]=>
  string(0) ""
  ["userId":"AMQPBasicProperties":private]=>
  string(0) ""
  ["appId":"AMQPBasicProperties":private]=>
  string(0) ""
  ["clusterId":"AMQPBasicProperties":private]=>
  string(0) ""
  ["body":"AMQPEnvelope":private]=>
  string(13) "test orphaned"
  ["consumerTag":"AMQPEnvelope":private]=>
  string(31) "amq.ctag-%s"
  ["deliveryTag":"AMQPEnvelope":private]=>
  int(2)
  ["isRedelivery":"AMQPEnvelope":private]=>
  bool(false)
  ["exchangeName":"AMQPEnvelope":private]=>
  string(%d) "ex1-%f"
  ["routingKey":"AMQPEnvelope":private]=>
  string(0) ""
}
