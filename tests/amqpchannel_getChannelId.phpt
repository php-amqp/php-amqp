--TEST--
AMQPChannel::getChannelId
--SKIPIF--
<?php
if (!extension_loaded("amqp") || version_compare(PHP_VERSION, '5.3', '<')) {
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