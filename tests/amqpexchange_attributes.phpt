--TEST--
AMQPExchange attributes
--SKIPIF--
<?php
if (!extension_loaded("amqp")) {
    print "skip";
}
?>
--FILE--
<?php
$cnn = new AMQPConnection();

$cnn->connect();
$ch = new AMQPChannel($cnn);

$ex = new AMQPExchange($ch);

$ex->setArguments($arr = array('existent' => 'value', 'false' => false, 'null' => null));

echo 'Initial args: ', count($arr), ', exchange args: ', count($ex->getArguments()), PHP_EOL;
$ex->setArgument('foo', 'bar');
echo 'Initial args: ', count($arr), ', exchange args: ', count($ex->getArguments()), PHP_EOL;

foreach (array('existent', 'false', 'null', 'nonexistent') as $key) {
    echo "$key: ";
    var_export($ex->hasArgument($key));
    echo ', ';
    try {
        var_export($ex->getArgument($key));
    } catch (AMQPExchangeException $e) {
        echo "Ex: " . $e->getMessage();
    }
    echo PHP_EOL;
}

?>
--EXPECT--
Initial args: 3, exchange args: 3
Initial args: 3, exchange args: 4
existent: true, 'value'
false: true, false
null: true, NULL
nonexistent: false, Ex: The argument "nonexistent" does not exist
