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
$cnn->disconnect();
var_dump($ch);

?>
--EXPECT--
object(AMQPChannel)#2 (4) {
  ["channel_id"]=>
  int(1)
  ["prefetch_count"]=>
  int(3)
  ["prefetch_size"]=>
  int(0)
  ["is_connected"]=>
  bool(true)
}
object(AMQPChannel)#2 (4) {
  ["channel_id"]=>
  int(1)
  ["prefetch_count"]=>
  int(3)
  ["prefetch_size"]=>
  int(0)
  ["is_connected"]=>
  bool(false)
}
