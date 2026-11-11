--TEST--
AMQPQueue::consume() - forked child returning from the callback into the consume loop does not read deliveries of the parent
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
elseif (!extension_loaded("pcntl")) print "skip pcntl extension is not loaded";
elseif (!getenv("PHP_AMQP_HOST")) print "skip PHP_AMQP_HOST environment variable is not set";
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->setReadTimeout(5);
$cnn->connect();

$ch = new AMQPChannel($cnn);

$q = new AMQPQueue($ch);
$q->setName('test_fork_consume_' . microtime(true));
$q->setFlags(AMQP_EXCLUSIVE);
$q->declareQueue();

$ex = new AMQPExchange($ch);
foreach (['m1', 'm2', 'm3'] as $body) {
    $ex->publish($body, $q->getName());
}

$parent = getmypid();
$received = 0;

try {
    $q->consume(function (AMQPEnvelope $envelope, AMQPQueue $queue) use ($parent, &$received) {
        if (getmypid() !== $parent) {
            echo 'child: got ', $envelope->getBody(), PHP_EOL;

            return false;
        }

        echo 'parent: got ', $envelope->getBody(), PHP_EOL;
        $received++;

        if ($received === 1) {
            $pid = pcntl_fork();

            if ($pid === 0) {
                // the child returns into the consume loop of the parent's connection
                return true;
            }

            pcntl_waitpid($pid, $status);
            echo 'child exit code: ', pcntl_wexitstatus($status), PHP_EOL;
        }

        $queue->ack($envelope->getDeliveryTag());

        return $received < 3;
    });
} catch (AMQPException $e) {
    echo getmypid() === $parent ? 'parent: ' : 'child: ', get_class($e), ': ', $e->getMessage(), PHP_EOL;

    if (getmypid() !== $parent) {
        exit(0);
    }
}

if (getmypid() !== $parent) {
    echo 'child: consume returned', PHP_EOL;
    exit(0);
}

echo 'parent: received ', $received, PHP_EOL;

$q->delete();
$cnn->disconnect();
?>
--EXPECTF--
parent: got m1
child: AMQPConnectionException: Could not consume messages from queue. Connection was opened in process %d and cannot be used in forked process %d.
child exit code: 0
parent: got m2
parent: got m3
parent: received 3
