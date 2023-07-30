--TEST--
AMQPConnection connect login failure
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
//ini_set('amqp.connect_timeout', 60);
//ini_set('amqp.read_timeout', 60);
//ini_set('amqp.write_timeout', 60);

$cnn = new AMQPConnection();
$cnn->setLogin('nonexistent-login-'. bin2hex(random_bytes(32)));
$cnn->setPassword('nonexistent-password-'. bin2hex(random_bytes(32)));

//var_dump($cnn);

echo ($cnn->isConnected() ? 'connected' : 'disconnected'), PHP_EOL;
//
try {
    $cnn->connect();
    echo 'Connected', PHP_EOL;
} catch (AMQPException $e) {
    echo get_class($e), "({$e->getCode()}): ", $e->getMessage(), PHP_EOL;
}
//
echo ($cnn->isConnected() ? 'connected' : 'disconnected'), PHP_EOL;

// NOTE: in real-world environment (incl. travis ci) "a socket error occurred" happens, but in vagrant environment "connection closed unexpectedly" happens. WTF?
?>
--EXPECTF--
disconnected
AMQPConnectionException(%d): %s error: %s - Potential login failure.
disconnected