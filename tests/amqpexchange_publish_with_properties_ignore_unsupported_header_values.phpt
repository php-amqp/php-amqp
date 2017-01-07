--TEST--
AMQPExchange publish with properties - ignore unsupported header values (NULL, object, resources)
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->connect();

$ch = new AMQPChannel($cnn);

$ex = new AMQPExchange($ch);
$ex->setName("exchange-" . microtime(true));
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->declareExchange();

$attrs = array(
    'headers' => array(
        'null'     => null,
        'object'   => new stdClass(),
        'resource' => fopen(__FILE__, 'r'),
    ),
);

echo $ex->publish('message', 'routing.key', AMQP_NOPARAM, $attrs) ? 'true' : 'false';

$ex->delete();


?>
--EXPECTF--
Warning: AMQPExchange::publish(): Ignoring field 'object' due to unsupported value type (object) in %s on line %d

Warning: AMQPExchange::publish(): Ignoring field 'resource' due to unsupported value type (resource) in %s on line %d
true
