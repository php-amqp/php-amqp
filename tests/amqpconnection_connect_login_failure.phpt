--TEST--
AMQPConnection connect login failure
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
elseif (!getenv("PHP_AMQP_HOST")) print "skip PHP_AMQP_HOST environment variable is not set";
?>
--FILE--
<?php
//ini_set('amqp.connect_timeout', 60);
//ini_set('amqp.read_timeout', 60);
//ini_set('amqp.write_timeout', 60);

$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
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
?>
--EXPECTF--
disconnected
AMQPConnectionException(403): %s
disconnected