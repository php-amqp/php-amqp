--TEST--
AMQPConnection too many channels on a connection
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();

$channels = array();

for ($i = 0; $i < PHP_AMQP_MAX_CHANNELS; $i++) {
	$channel = $channels[] = new AMQPChannel($cnn);
    //echo '#', $channel->getChannelId(), ', used ', $cnn->getUsedChannels(), ' of ', $cnn->getMaxChannels(), PHP_EOL;
}

echo "Good\n";

try {
	new AMQPChannel($cnn);
	echo "Bad\n";
} catch(Exception $e) {
    echo get_class($e), "({$e->getCode()}): ", $e->getMessage(), PHP_EOL;
}

?>
--EXPECT--
Good
AMQPChannelException(0): Could not create channel. Connection has no open channel slots remaining.