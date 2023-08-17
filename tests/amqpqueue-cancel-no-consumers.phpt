--TEST--
AMQPQueue - orphaned envelope
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip";
if (!getenv("PHP_AMQP_HOST")) print "skip";
--FILE--
<?php
///** @var \Enqueue\AmqpExt\AmqpContext context */
//$context = $this->createContext();

$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->connect();

$extChannel = new AMQPChannel($cnn);
$extChannel->qos(0, 3);

$microtime = microtime('true');
$queue_name = 'queue-test-.' . $microtime;
$exchange_name = 'exchnage-test-.' . $microtime;


$queue_1 = new \AMQPQueue($extChannel);
$queue_1->setName($queue_name);
$queue_1->declareQueue();
var_dump($queue_1->purge());

$exchange = new \AMQPExchange($extChannel);
$exchange->setType(AMQP_EX_TYPE_DIRECT);
$exchange->setName($exchange_name);
$exchange->declareExchange();

$exchange->publish( 'test message', $queue_name, AMQP_NOPARAM, []);

$queue_2 = new \AMQPQueue($extChannel);
$queue_2->setName($queue_name);
$queue_2->consume(null, AMQP_NOPARAM, '');

$consumer_tag = $queue_2->getConsumerTag();

$queue_1->cancel($consumer_tag);

echo "Canceled", PHP_EOL;
--EXPECTF--
int(0)
Canceled
