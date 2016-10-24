--TEST--
AMQPConnection - heartbeats support with persistent connections
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$heartbeat = 2;
$credentials = array('heartbeat' => $heartbeat);

$cnn = new AMQPConnection($credentials);

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

?>
--EXPECTF--
heartbeat: 2
connected: false
persistent: false

heartbeat: 2
connected: true
persistent: true

AMQPException(0): Library error: a socket error occurred

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
