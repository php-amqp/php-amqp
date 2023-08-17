--TEST--
AMQPQueue attributes
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

$q = new AMQPQueue($ch);

var_dump($q->setArguments($arr = array('existent' => 'value', 'false' => false)));

echo 'Initial args: ', count($arr), ', queue args: ', count($q->getArguments()), PHP_EOL;
var_dump($q->setArgument('foo', 'bar'));
echo 'Initial args: ', count($arr), ', queue args: ', count($q->getArguments()), PHP_EOL;

foreach (array('existent', 'false', 'nonexistent') as $key) {
    echo "$key: ";
    var_export($q->hasArgument($key));
    echo ', ';
    try {
        var_export($q->getArgument($key));
    } catch (AMQPQueueException $e) {
        echo 'Ex: ', $e->getMessage();
    }
    echo PHP_EOL;
}

?>
--EXPECT--
NULL
Initial args: 2, queue args: 2
NULL
Initial args: 2, queue args: 3
existent: true, 'value'
false: true, false
nonexistent: false, Ex: The argument "nonexistent" does not exist
