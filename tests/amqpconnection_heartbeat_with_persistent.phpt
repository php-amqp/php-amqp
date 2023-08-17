--TEST--
AMQPConnection - heartbeats support with persistent connections
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip";
if (!getenv("PHP_AMQP_HOST")) print "skip";
?>
--FILE--
<?php
$heartbeat = 2;
$credentials = array('heartbeat' => $heartbeat);

$cnn = new AMQPConnection($credentials);
$cnn->setHost(getenv('PHP_AMQP_HOST'));

echo 'heartbeat: ', var_export($cnn->getHeartbeatInterval(), true), PHP_EOL;
echo 'connected: ', var_export($cnn->isConnected(), true), PHP_EOL;
echo 'persistent: ', var_export($cnn->isPersistent(), true), PHP_EOL;
echo PHP_EOL;

$cnn->pconnect();

echo 'heartbeat: ', var_export($cnn->getHeartbeatInterval(), true), PHP_EOL;
echo 'connected: ', var_export($cnn->isConnected(), true), PHP_EOL;
echo 'persistent: ', var_export($cnn->isPersistent(), true), PHP_EOL;
echo PHP_EOL;

sleep($heartbeat*5);

try {
    $ch = new AMQPChannel($cnn);
    echo 'channel created', PHP_EOL;
} catch (AMQPException $e) {
    echo get_class($e), "({$e->getCode()}): ", $e->getMessage(), PHP_EOL;
}

echo PHP_EOL;
echo 'heartbeat: ', var_export($cnn->getHeartbeatInterval(), true), PHP_EOL;
echo 'connected: ', var_export($cnn->isConnected(), true), PHP_EOL;
echo 'persistent: ', var_export($cnn->isPersistent(), true), PHP_EOL;
echo PHP_EOL;

$cnn = new AMQPConnection($credentials);
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->pconnect();

echo 'heartbeat: ', var_export($cnn->getHeartbeatInterval(), true), PHP_EOL;
echo 'connected: ', var_export($cnn->isConnected(), true), PHP_EOL;
echo 'persistent: ', var_export($cnn->isPersistent(), true), PHP_EOL;
echo PHP_EOL;


$ch = new AMQPChannel($cnn);
echo 'channel created', PHP_EOL;
echo PHP_EOL;

echo 'heartbeat: ', var_export($cnn->getHeartbeatInterval(), true), PHP_EOL;
echo 'connected: ', var_export($cnn->isConnected(), true), PHP_EOL;
echo 'persistent: ', var_export($cnn->isPersistent(), true), PHP_EOL;
echo PHP_EOL;

$cnn->pdisconnect();

echo 'heartbeat: ', var_export($cnn->getHeartbeatInterval(), true), PHP_EOL;
echo 'connected: ', var_export($cnn->isConnected(), true), PHP_EOL;
echo 'persistent: ', var_export($cnn->isPersistent(), true), PHP_EOL;
echo PHP_EOL;

// NOTE: in real-world environment (incl. travis ci) "a socket error occurred" happens, but in virtual environment "connection closed unexpectedly" happens
?>
--EXPECTF--
heartbeat: 2
connected: false
persistent: false

heartbeat: 2
connected: true
persistent: true

AMQPException(0): Library error: %s

heartbeat: 2
connected: false
persistent: false

heartbeat: 2
connected: true
persistent: true

channel created

heartbeat: 2
connected: true
persistent: true

heartbeat: 2
connected: false
persistent: false
