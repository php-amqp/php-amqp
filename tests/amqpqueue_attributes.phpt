--TEST--
AMQPQueue attributes
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();

$cnn->connect();
$ch = new AMQPChannel($cnn);

$q = new AMQPQueue($ch);

$q->setArguments(array('existent' => 'value', 'false' => false));

foreach (array('existent', 'false', 'nonexistent') as $key) {
    echo "$key: ", var_export($q->hasArgument($key), true), ', ', var_export($q->getArgument($key)), PHP_EOL;
}

?>
--EXPECT--
existent: true, 'value'
false: true, false
nonexistent: false, false
