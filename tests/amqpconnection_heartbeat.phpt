--TEST--
AMQPConnection - heartbeats support
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
elseif (!getenv("PHP_AMQP_HOST")) print "skip PHP_AMQP_HOST environment variable is not set";
elseif (getenv("SKIP_SLOW_TESTS")) print "skip slow test and SKIP_SLOW_TESTS is set";
?>
--FILE--
<?php
$heartbeat = 2;
$credentials = array('heartbeat' => $heartbeat);
$cnn = new AMQPConnection($credentials);
$cnn->setHost(getenv('PHP_AMQP_HOST'));
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
--EXPECTREGEX--
heartbeat: 2
connected: true
persistent: false
AMQPConnectionException\(0\): (a socket error occurred|connection closed unexpectedly)
heartbeat: 2
connected: false
persistent: false
