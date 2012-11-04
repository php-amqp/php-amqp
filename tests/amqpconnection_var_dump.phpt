--TEST--
AMQPConnection var_dump
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
var_dump($cnn);
?>
--EXPECT--
object(AMQPConnection)#1 (6) {
  ["login"]=>
  string(5) "guest"
  ["password"]=>
  string(5) "guest"
  ["host"]=>
  string(9) "localhost"
  ["vhost"]=>
  string(1) "/"
  ["port"]=>
  int(5672)
  ["timeout"]=>
  float(0)
}
