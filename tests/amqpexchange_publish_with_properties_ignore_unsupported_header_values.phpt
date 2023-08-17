--TEST--
AMQPExchange publish with properties - ignore unsupported header values (NULL, object, resources)
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip";
if (!getenv("PHP_AMQP_HOST")) print "skip";
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->connect();

$ch = new AMQPChannel($cnn);

$ex = new AMQPExchange($ch);
$ex->setName("exchange-" . bin2hex(random_bytes(32)));
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->declareExchange();

$attrs = array(
    'headers' => array(
        'null'     => null,
        'object'   => new stdClass(),
        'resource' => fopen(__FILE__, 'r'),
    ),
);

var_dump($ex->publish('message', 'routing.key', AMQP_NOPARAM, $attrs));

$ex->delete();


?>
--EXPECTF--
Warning: AMQPExchange::publish(): Ignoring field 'object' due to unsupported value type (object) in %s on line %d

Warning: AMQPExchange::publish(): Ignoring field 'resource' due to unsupported value type (resource) in %s on line %d
NULL
