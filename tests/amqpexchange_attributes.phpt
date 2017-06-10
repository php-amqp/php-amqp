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

$ex->setArguments($arr = array('existent' => 'value', 'false' => false));

echo 'Initial args: ', count($arr), ', exchange args: ', count($ex->getArguments()), PHP_EOL;
$ex->setArgument('foo', 'bar');
echo 'Initial args: ', count($arr), ', exchange args: ', count($ex->getArguments()), PHP_EOL;

foreach (array('existent', 'false', 'nonexistent') as $key) {
    echo "$key: ", var_export($ex->hasArgument($key), true), ', ', var_export($ex->getArgument($key)), PHP_EOL;
}

?>
--EXPECT--
Initial args: 2, exchange args: 2
Initial args: 2, exchange args: 3
existent: true, 'value'
false: true, false
nonexistent: false, false
