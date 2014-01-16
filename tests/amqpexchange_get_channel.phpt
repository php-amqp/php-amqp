--TEST--
AMQPExchange getChannel() test
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();

$ch = new AMQPChannel($cnn);
$ch2 = new AMQPChannel($cnn);

$ex = new AMQPExchange($ch);


echo $ch === $ex->getChannel() ? 'same' : 'not same', PHP_EOL;
echo $ch2 === $ex->getChannel() ? 'same' : 'not same', PHP_EOL;

$old_prefetch = $ch->getPrefetchCount();
$new_prefetch = 999;

$ex->getChannel()->setPrefetchCount($new_prefetch);

echo $ch->getPrefetchCount() == $new_prefetch ? 'by ref' : 'copy', PHP_EOL;

?>
--EXPECT--
same
not same
by ref
