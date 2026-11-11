--TEST--
AMQPConnection - TLS - connection inherited by forked child does not break parent connection
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
elseif (!extension_loaded("pcntl")) print "skip pcntl extension is not loaded";
elseif (!getenv("PHP_AMQP_SSL_HOST")) print "skip PHP_AMQP_SSL_HOST environment variable is not set";
elseif (!file_exists(__DIR__ . "/../infra/tls/certificates/testca/cacert.pem")) print "skip CA certificate is not available";
?>
--FILE--
<?php
$cnn = new AMQPConnection(array(
    'port' => 5671,
    'host' => getenv('PHP_AMQP_SSL_HOST'),
    'cacert' => __DIR__ . "/../infra/tls/certificates/testca/cacert.pem",
));
$cnn->connect();

$ch = new AMQPChannel($cnn);

$q = new AMQPQueue($ch);
$q->setName('test_fork_tls_' . microtime(true));
$q->setFlags(AMQP_EXCLUSIVE);
$q->declareQueue();

$ex = new AMQPExchange($ch);
$ex->publish('before fork', $q->getName());

$pid = pcntl_fork();

if ($pid === 0) {
    try {
        $ex->publish('from child', $q->getName());
    } catch (AMQPException $e) {
        echo 'child: ', get_class($e), PHP_EOL;
    }

    // Normal exit runs destructors, they must not send anything (incl. TLS close_notify) over parent's socket
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
child: AMQPConnectionException
child exit code: 0
parent: got before fork
parent: got after fork
