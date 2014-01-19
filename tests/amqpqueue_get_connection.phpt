--TEST--
AMQPQueue getConnection test
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

$q = new AMQPQueue($ch);

$cnn2 = new AMQPConnection();

echo $cnn === $q->getConnection() ? 'same' : 'not same', PHP_EOL;
echo $cnn2 === $q->getConnection() ? 'same' : 'not same', PHP_EOL;

$old_host = $cnn->getHost();
$new_host = 'test';

$q->getConnection()->setHost($new_host);

echo $cnn->getHost() == $new_host ? 'by ref' : 'copy', PHP_EOL;

?>
--EXPECT--
same
not same
by ref
