--TEST--
AMQPExchange attributes
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

$ex = new AMQPExchange($ch);

$ex->setArguments(array('existent' => 'value', 'false' => false));

foreach (array('existent', 'false', 'nonexistent') as $key) {
    echo "$key: ", var_export($ex->hasArgument($key), true), ', ', var_export($ex->getArgument($key)), PHP_EOL;
}

?>
--EXPECT--
existent: true, 'value'
false: true, false
nonexistent: false, false
