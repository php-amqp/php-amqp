--TEST--
AMQPChannel::getChannelId
--SKIPIF--
<?php
if (!extension_loaded("amqp")) {
    print "skip";
}
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();
$ch = new AMQPChannel($cnn);

var_dump($ch->getChannelId());

$cnn->disconnect();
var_dump($ch->getChannelId());

?>
--EXPECT--
int(1)
int(1)