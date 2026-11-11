--TEST--
AMQPConnection - forked child can open its own connection, explicitly or by connect() on inherited object
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
    $own = new AMQPConnection();
    $own->setHost(getenv('PHP_AMQP_HOST'));
    $own->connect();
    (new AMQPExchange(new AMQPChannel($own)))->publish('from child own connection', $q->getName());
    $own->disconnect();

    // connect() on inherited object drops inherited connection locally and opens a new one
    $cnn->connect();
    echo 'child: connected: ', var_export($cnn->isConnected(), true), PHP_EOL;
    (new AMQPExchange(new AMQPChannel($cnn)))->publish('from child reconnected', $q->getName());

    try {
        $ex->publish('from child old channel', $q->getName());
    } catch (AMQPException $e) {
        echo 'child: old channel: ', get_class($e), ': ', $e->getMessage(), PHP_EOL;
    }

    $cnn->disconnect();
    exit(0);
}

pcntl_waitpid($pid, $status);
echo 'child exit code: ', pcntl_wexitstatus($status), PHP_EOL;

$ex->publish('after fork', $q->getName());

while ($msg = $q->get(AMQP_AUTOACK)) {
    echo 'parent: got ', $msg->getBody(), PHP_EOL;
}

$q->delete();
$cnn->disconnect();
?>
--EXPECT--
child: connected: true
child: old channel: AMQPChannelException: Could not publish to exchange. No channel available.
child exit code: 0
parent: got before fork
parent: got from child own connection
parent: got from child reconnected
parent: got after fork
