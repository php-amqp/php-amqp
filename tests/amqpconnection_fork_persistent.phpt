--TEST--
AMQPConnection - persistent connection inherited by forked child is not reused there
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
$cnn->pconnect();

$ch = new AMQPChannel($cnn);

$q = new AMQPQueue($ch);
$q->setName('test_fork_' . microtime(true));
$q->setFlags(AMQP_EXCLUSIVE);
$q->declareQueue();

$ex = new AMQPExchange($ch);
$ex->publish('before fork', $q->getName());

$pid = pcntl_fork();

if ($pid === 0) {
    try {
        $ex->publish('from child inherited channel', $q->getName());
    } catch (AMQPException $e) {
        echo 'child: inherited channel: ', get_class($e), PHP_EOL;
    }

    // Same credentials hash: must not pick up parent's persistent connection
    $own = new AMQPConnection();
    $own->setHost(getenv('PHP_AMQP_HOST'));
    $own->pconnect();
    echo 'child: own connected: ', var_export($own->isConnected(), true), PHP_EOL;
    echo 'child: inherited connected: ', var_export($cnn->isConnected(), true), PHP_EOL;
    (new AMQPExchange(new AMQPChannel($own)))->publish('from child', $q->getName());
    $own->pdisconnect();

    exit(0);
}

pcntl_waitpid($pid, $status);
echo 'child exit code: ', pcntl_wexitstatus($status), PHP_EOL;

$ex->publish('after fork', $q->getName());

while ($msg = $q->get(AMQP_AUTOACK)) {
    echo 'parent: got ', $msg->getBody(), PHP_EOL;
}

$q->delete();
$cnn->pdisconnect();
?>
--EXPECT--
child: inherited channel: AMQPConnectionException
child: own connected: true
child: inherited connected: false
child exit code: 0
parent: got before fork
parent: got from child
parent: got after fork
