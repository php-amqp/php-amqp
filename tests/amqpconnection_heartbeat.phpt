--TEST--
AMQPConnection - heartbeats support
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$heartbeat = 2;
$credentials = array('heartbeat' => $heartbeat);
$cnn = new AMQPConnection($credentials);
$cnn->connect();

echo 'heartbeat: ', var_export($cnn->getHeartbeatInterval(), true), PHP_EOL;
echo 'connected: ', var_export($cnn->isConnected(), true), PHP_EOL;
echo 'persistent: ', var_export($cnn->isPersistent(), true), PHP_EOL;

sleep($heartbeat*5);

try {
    $ch = new AMQPChannel($cnn);
    echo 'channel created', PHP_EOL;
} catch (AMQPException $e) {
  echo get_class($e), "({$e->getCode()}): ", $e->getMessage(), PHP_EOL;
}

echo 'heartbeat: ', var_export($cnn->getHeartbeatInterval(), true), PHP_EOL;
echo 'connected: ', var_export($cnn->isConnected(), true), PHP_EOL;
echo 'persistent: ', var_export($cnn->isPersistent(), true), PHP_EOL;

?>
--EXPECTF--
heartbeat: 2
connected: true
persistent: false
AMQPException(0): Library error: a socket error occurred
heartbeat: 2
connected: false
persistent: false
