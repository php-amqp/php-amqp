--TEST--
AMQPQueue::consume with AMQP_NB_CONSUME (non-blocking drain)
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
elseif (!getenv("PHP_AMQP_HOST")) print "skip PHP_AMQP_HOST environment variable is not set";
elseif (!defined("AMQP_NB_CONSUME")) print "skip AMQP_NB_CONSUME constant is not available";
?>
--FILE--
<?php
$id = bin2hex(random_bytes(16));

$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->connect();

$ch = new AMQPChannel($cnn);

$ex = new AMQPExchange($ch);
$ex->setName('exchange-nb-' . $id);
$ex->setType(AMQP_EX_TYPE_DIRECT);
$ex->declareExchange();

$q = new AMQPQueue($ch);
$q->setName('queue-nb-' . $id);
$q->declareQueue();
$q->bind($ex->getName(), 'k');

// Subscribe once; no callback so this just sends basic.consume and returns.
$q->consume(null);

// 1) Empty queue: must return immediately without throwing, callback must not fire.
$got = 'sentinel';
$q->consume(function (AMQPEnvelope $envelope) use (&$got) {
    $got = $envelope->getBody();
    return false;
}, AMQP_JUST_CONSUME | AMQP_NB_CONSUME);
echo "empty: ", $got, PHP_EOL;

// 2) After publishing, the non-blocking drain delivers the message.
$ex->publish('hello', 'k');

// Give the broker a moment to push the frame to us before we drain.
$deadline = microtime(true) + 2.0;
do {
    $got = 'sentinel';
    $q->consume(function (AMQPEnvelope $envelope) use (&$got) {
        $got = $envelope->getBody();
        return false;
    }, AMQP_JUST_CONSUME | AMQP_NB_CONSUME | AMQP_AUTOACK);
    if ($got !== 'sentinel') {
        break;
    }
    usleep(50000);
} while (microtime(true) < $deadline);
echo "drained: ", $got, PHP_EOL;

// 3) Without AMQP_NB_CONSUME, the existing timeout-exception behaviour is preserved.
$cnn2 = new AMQPConnection(['read_timeout' => 0.5]);
$cnn2->setHost(getenv('PHP_AMQP_HOST'));
$cnn2->connect();
$ch2 = new AMQPChannel($cnn2);
$q2 = new AMQPQueue($ch2);
$q2->setFlags(AMQP_EXCLUSIVE);
$q2->declareQueue();
try {
    $q2->consume(function () { return false; });
    echo "no-exception", PHP_EOL;
} catch (AMQPQueueException $e) {
    echo "blocking-throws: ", $e->getMessage(), PHP_EOL;
}

$q->delete();
$ex->delete();
$q2->delete();
$cnn->disconnect();
$cnn2->disconnect();
?>
--EXPECT--
empty: sentinel
drained: hello
blocking-throws: Consumer timeout exceed
