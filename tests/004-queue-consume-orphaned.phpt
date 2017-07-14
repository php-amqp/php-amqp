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
  ["content_type":"AMQPBasicProperties":private]=>
  string(10) "text/plain"
  ["content_encoding":"AMQPBasicProperties":private]=>
  string(0) ""
  ["headers":"AMQPBasicProperties":private]=>
  array(0) {
  }
  ["delivery_mode":"AMQPBasicProperties":private]=>
  int(1)
  ["priority":"AMQPBasicProperties":private]=>
  int(0)
  ["correlation_id":"AMQPBasicProperties":private]=>
  string(0) ""
  ["reply_to":"AMQPBasicProperties":private]=>
  string(0) ""
  ["expiration":"AMQPBasicProperties":private]=>
  string(0) ""
  ["message_id":"AMQPBasicProperties":private]=>
  string(0) ""
  ["timestamp":"AMQPBasicProperties":private]=>
  int(0)
  ["type":"AMQPBasicProperties":private]=>
  string(0) ""
  ["user_id":"AMQPBasicProperties":private]=>
  string(0) ""
  ["app_id":"AMQPBasicProperties":private]=>
  string(0) ""
  ["cluster_id":"AMQPBasicProperties":private]=>
  string(0) ""
  ["body":"AMQPEnvelope":private]=>
  string(13) "test orphaned"
  ["consumer_tag":"AMQPEnvelope":private]=>
  string(31) "amq.ctag-%s"
  ["delivery_tag":"AMQPEnvelope":private]=>
  int(2)
  ["is_redelivery":"AMQPEnvelope":private]=>
  bool(false)
  ["exchange_name":"AMQPEnvelope":private]=>
  string(%d) "ex1-%f"
  ["routing_key":"AMQPEnvelope":private]=>
  string(0) ""
}
