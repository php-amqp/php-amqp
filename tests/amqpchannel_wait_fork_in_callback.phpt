--TEST--
AMQPChannel::waitForConfirm() and waitForBasicReturn() - forked child returning from the callback into the wait loop does not read frames of the parent
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
$cnn->connect();

$ch = new AMQPChannel($cnn);
$ch->confirmSelect();

$q = new AMQPQueue($ch);
$q->setName('test_fork_wait_' . microtime(true));
$q->setFlags(AMQP_EXCLUSIVE);
$q->declareQueue();

$ex = new AMQPExchange($ch);

$parent = getmypid();

// fork in the callback; the child returns true (keep waiting), the parent returns false (stop)
$forkInCallback = function (string $name) use ($parent) {
    if (getmypid() !== $parent) {
        echo "child: {$name} callback", PHP_EOL;

        return false;
    }

    echo "parent: {$name} callback", PHP_EOL;

    $pid = pcntl_fork();

    if ($pid === 0) {
        return true;
    }

    pcntl_waitpid($pid, $status);
    echo 'child exit code: ', pcntl_wexitstatus($status), PHP_EOL;

    return false;
};

$wait = function (AMQPChannel $channel, string $method) use ($parent) {
    try {
        $channel->{$method}(2);
        echo getmypid() === $parent ? 'parent: ' : 'child: ', "{$method} returned", PHP_EOL;
    } catch (AMQPException $e) {
        echo getmypid() === $parent ? 'parent: ' : 'child: ', get_class($e), ': ', $e->getMessage(), PHP_EOL;
    }

    if (getmypid() !== $parent) {
        exit(0);
    }
};

$ch->setConfirmCallback(function () use ($forkInCallback) {
    return $forkInCallback('ack');
});
$ex->publish('confirmed', $q->getName());
$wait($ch, 'waitForConfirm');

// basic.return on a channel without confirms, so no basic.ack is left unread
$ch2 = new AMQPChannel($cnn);
$ch2->setReturnCallback(function () use ($forkInCallback) {
    return $forkInCallback('return');
});
$ex2 = new AMQPExchange($ch2);
$ex2->publish('unroutable', 'no_such_queue_' . microtime(true), AMQP_MANDATORY);
$wait($ch2, 'waitForBasicReturn');

// the parent connection is still usable
$ch->setConfirmCallback(function () {
    echo 'parent: ack callback', PHP_EOL;

    return false;
});
$ex->publish('after fork', $q->getName());
$wait($ch, 'waitForConfirm');

echo 'parent: messages in queue: ', $q->declareQueue(), PHP_EOL;

$q->delete();
$cnn->disconnect();
?>
--EXPECTF--
parent: ack callback
child: AMQPConnectionException: Could not wait for confirm methods. Connection was opened in process %d and cannot be used in forked process %d.
child exit code: 0
parent: waitForConfirm returned
parent: return callback
child: AMQPConnectionException: Could not wait for basic.return method. Connection was opened in process %d and cannot be used in forked process %d.
child exit code: 0
parent: waitForBasicReturn returned
parent: ack callback
parent: waitForConfirm returned
parent: messages in queue: 2
