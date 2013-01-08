--TEST--
AMQPChannel var_dump
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
var_dump($ch);
?>
--EXPECT--
object(AMQPChannel)#2 (3) {
  ["channel_id"]=>
  int(1)
  ["prefetch_count"]=>
  int(3)
  ["prefetch_size"]=>
  int(0)
}

