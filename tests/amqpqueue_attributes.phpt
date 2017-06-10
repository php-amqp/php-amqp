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

$q->setArguments($arr = array('existent' => 'value', 'false' => false));

echo 'Initial args: ', count($arr), ', queue args: ', count($q->getArguments()), PHP_EOL;
$q->setArgument('foo', 'bar');
echo 'Initial args: ', count($arr), ', queue args: ', count($q->getArguments()), PHP_EOL;

foreach (array('existent', 'false', 'nonexistent') as $key) {
    echo "$key: ", var_export($q->hasArgument($key), true), ', ', var_export($q->getArgument($key)), PHP_EOL;
}

?>
--EXPECT--
Initial args: 2, queue args: 2
Initial args: 2, queue args: 3
existent: true, 'value'
false: true, false
nonexistent: false, false
