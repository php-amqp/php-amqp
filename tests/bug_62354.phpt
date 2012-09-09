--TEST--
Constructing AMQPQueue with AMQPConnection segfaults
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
class Amqptest {};
$o = new Amqptest();
$o->conn = new AMQPConnection();
print_r($o);
?>
--EXPECTF--
Amqptest Object
(
    [conn] => AMQPConnection Object
        (
            [login] => guest
            [password] => guest
            [host] => localhost
            [vhost] => /
            [port] => 5672
        )

)