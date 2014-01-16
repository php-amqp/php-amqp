--TEST--
AMQPQueue getChannel() test
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();

$ch = new AMQPChannel($cnn);
$ch2 = new AMQPChannel($cnn);

$q = new AMQPQueue($ch);

echo $ch === $q->getChannel() ? 'same' : 'not same', PHP_EOL;
echo $ch2 === $q->getChannel() ? 'same' : 'not same', PHP_EOL;

$old_prefetch = $ch->getPrefetchCount();
$new_prefetch = 999;

$q->getChannel()->setPrefetchCount($new_prefetch);

echo $ch->getPrefetchCount() == $new_prefetch ? 'by ref' : 'copy', PHP_EOL;

?>
--EXPECT--
same
not same
by ref
