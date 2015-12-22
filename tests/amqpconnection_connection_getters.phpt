--TEST--
AMQPConnection - connection-specific getters
--SKIPIF--
<?php
if (!extension_loaded("amqp") || version_compare(PHP_VERSION, '5.3', '<')) {
  print "skip";
}
?>
--FILE--
<?php
$cnn = new AMQPConnection();

echo 'connected: ', var_export($cnn->isConnected(), true), PHP_EOL;
echo 'channel_max: ', var_export($cnn->getMaxChannels(), true), PHP_EOL;
echo 'frame_max: ', var_export($cnn->getMaxFrameSize(), true), PHP_EOL;
echo 'heartbeat: ', var_export($cnn->getHeartbeatInterval(), true), PHP_EOL;
echo PHP_EOL;

$cnn->connect();

echo 'connected: ', var_export($cnn->isConnected(), true), PHP_EOL;
echo 'channel_max: ', var_export($cnn->getMaxChannels(), true), PHP_EOL;
echo 'frame_max: ', var_export($cnn->getMaxFrameSize(), true), PHP_EOL;
echo 'heartbeat: ', var_export($cnn->getHeartbeatInterval(), true), PHP_EOL;
echo PHP_EOL;

$cnn->disconnect();

echo 'connected: ', var_export($cnn->isConnected(), true), PHP_EOL;
echo 'channel_max: ', var_export($cnn->getMaxChannels(), true), PHP_EOL;
echo 'frame_max: ', var_export($cnn->getMaxFrameSize(), true), PHP_EOL;
echo 'heartbeat: ', var_export($cnn->getHeartbeatInterval(), true), PHP_EOL;
echo PHP_EOL;


$cnn = new AMQPConnection(array('channel_max' => '10', 'frame_max' => 10240, 'heartbeat' => 10));

echo 'connected: ', var_export($cnn->isConnected(), true), PHP_EOL;
echo 'channel_max: ', var_export($cnn->getMaxChannels(), true), PHP_EOL;
echo 'frame_max: ', var_export($cnn->getMaxFrameSize(), true), PHP_EOL;
echo 'heartbeat: ', var_export($cnn->getHeartbeatInterval(), true), PHP_EOL;
echo PHP_EOL;

$cnn->connect();

echo 'connected: ', var_export($cnn->isConnected(), true), PHP_EOL;
echo 'channel_max: ', var_export($cnn->getMaxChannels(), true), PHP_EOL;
echo 'frame_max: ', var_export($cnn->getMaxFrameSize(), true), PHP_EOL;
echo 'heartbeat: ', var_export($cnn->getHeartbeatInterval(), true), PHP_EOL;
echo PHP_EOL;

$cnn->disconnect();

echo 'connected: ', var_export($cnn->isConnected(), true), PHP_EOL;
echo 'channel_max: ', var_export($cnn->getMaxChannels(), true), PHP_EOL;
echo 'frame_max: ', var_export($cnn->getMaxFrameSize(), true), PHP_EOL;
echo 'heartbeat: ', var_export($cnn->getHeartbeatInterval(), true), PHP_EOL;
echo PHP_EOL;
?>
--EXPECT--
connected: false
channel_max: 256
frame_max: 131072
heartbeat: 0

connected: true
channel_max: 256
frame_max: 131072
heartbeat: 0

connected: false
channel_max: 256
frame_max: 131072
heartbeat: 0

connected: false
channel_max: 10
frame_max: 10240
heartbeat: 10

connected: true
channel_max: 10
frame_max: 10240
heartbeat: 10

connected: false
channel_max: 10
frame_max: 10240
heartbeat: 10
