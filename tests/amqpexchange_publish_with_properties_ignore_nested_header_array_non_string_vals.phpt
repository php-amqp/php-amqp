--TEST--
AMQPExchange publish with properties - ignore nested header array non-string values
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
        'nested' => array(
            'string'        => 'passed',
            'numeric fails' => 999,
        ),
    ),
);

echo $ex->publish('message', 'routing.key', AMQP_NOPARAM, $attrs) ? 'true' : 'false';

$ex->delete();

?>
--EXPECTF--
Warning: AMQPExchange::publish(): Ignoring non-string header nested array member type 1 for field 'nested' in %s on line %d
true