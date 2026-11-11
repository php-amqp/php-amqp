--TEST--
AMQPConnection - connection inherited by forked child can not be used there and does not break parent connection
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

$q = new AMQPQueue($ch);
$q->setName('test_fork_' . microtime(true));
$q->setFlags(AMQP_EXCLUSIVE);
$q->declareQueue();

$ex = new AMQPExchange($ch);
$ex->publish('before fork', $q->getName());

$pid = pcntl_fork();

if ($pid === 0) {
    echo 'child: connection connected: ', var_export($cnn->isConnected(), true), PHP_EOL;
    echo 'child: channel connected: ', var_export($ch->isConnected(), true), PHP_EOL;

    $calls = [
        'publish' => function () use ($ex, $q) { $ex->publish('from child', $q->getName()); },
        'get' => function () use ($q) { $q->get(); },
        'declare' => function () use ($q) { $q->declareQueue(); },
        'channel' => function () use ($cnn) { new AMQPChannel($cnn); },
        'qos' => function () use ($ch) { $ch->setPrefetchCount(10); },
    ];

    foreach ($calls as $name => $call) {
        try {
            $call();
            echo "child: {$name}: no exception", PHP_EOL;
        } catch (AMQPException $e) {
            echo "child: {$name}: ", get_class($e), ': ', $e->getMessage(), PHP_EOL;
        }
    }

    // Normal exit runs destructors, they must not close parent connection
    exit(0);
}

pcntl_waitpid($pid, $status);
echo 'child exit code: ', pcntl_wexitstatus($status), PHP_EOL;

echo 'parent: connection connected: ', var_export($cnn->isConnected(), true), PHP_EOL;
echo 'parent: channel connected: ', var_export($ch->isConnected(), true), PHP_EOL;

$ex->publish('after fork', $q->getName());

while ($msg = $q->get(AMQP_AUTOACK)) {
    echo 'parent: got ', $msg->getBody(), PHP_EOL;
}

$q->delete();
$cnn->disconnect();
?>
--EXPECTF--
child: connection connected: false
child: channel connected: false
child: publish: AMQPConnectionException: Could not publish to exchange. Connection was opened in process %d and cannot be used in forked process %d.
child: get: AMQPConnectionException: Could not get messages from queue. Connection was opened in process %d and cannot be used in forked process %d.
child: declare: AMQPConnectionException: Could not declare queue. Connection was opened in process %d and cannot be used in forked process %d.
child: channel: AMQPConnectionException: Could not create channel. Connection was opened in process %d and cannot be used in forked process %d.
child: qos: AMQPConnectionException: Could not set prefetch count. Connection was opened in process %d and cannot be used in forked process %d.
child exit code: 0
parent: connection connected: true
parent: channel connected: true
parent: got before fork
parent: got after fork
