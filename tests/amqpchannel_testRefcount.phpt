--TEST--
AMQPChannel Ref Count test
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

unset($cnn);

debug_zval_dump($ch);

?>
--EXPECT--
object(AMQPChannel)#2 (5) refcount(2){
  ["channel_id"]=>
  int(1)
  ["prefetch_count"]=>
  int(3)
  ["prefetch_size"]=>
  int(0)
  ["is_connected"]=>
  bool(true)
  ["connection"]=>
  object(AMQPConnection)#1 (15) refcount(1){
    ["login"]=>
    string(5) "guest" refcount(1)
    ["password"]=>
    string(5) "guest" refcount(1)
    ["host"]=>
    string(9) "localhost" refcount(1)
    ["vhost"]=>
    string(1) "/" refcount(1)
    ["port"]=>
    int(5672)
    ["read_timeout"]=>
    float(0)
    ["write_timeout"]=>
    float(0)
    ["connect_timeout"]=>
    float(0)
    ["is_connected"]=>
    bool(true)
    ["is_persistent"]=>
    bool(false)
    ["connection_resource"]=>
    resource(4) of type (AMQP Connection Resource) refcount(2)
    ["used_channels"]=>
    int(1)
    ["max_channel_id"]=>
    int(256)
    ["max_frame_size"]=>
    int(131072)
    ["heartbeat_interval"]=>
    int(0)
  }
}