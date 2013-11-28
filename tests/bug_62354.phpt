--TEST--
Constructing AMQPQueue with AMQPConnection segfaults
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
class Amqptest {};
$o = new Amqptest();
$o->conn = new AMQPConnection();
$funcs = array(
  'getHost', 'getLogin', 'getPassword', 'getPort', 'getVHost', 'isConnected'
);
foreach ($funcs as $func) {
  printf("%s => %s\n", $func, var_export($o->conn->$func(), true));
};
?>
--EXPECT--
getHost => 'localhost'
getLogin => 'guest'
getPassword => 'guest'
getPort => 5672
getVHost => '/'
isConnected => false