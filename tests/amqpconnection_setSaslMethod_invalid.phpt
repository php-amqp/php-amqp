--TEST--
AMQPConnection setSaslMethod invalid
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip";
if (!getenv("PHP_AMQP_HOST")) print "skip";
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
try {
	$cnn->setSaslMethod(-1);
} catch (Exception $e) {
	echo get_class($e);
	echo PHP_EOL;
    echo $e->getMessage();
}
?>
--EXPECT--
AMQPConnectionException
Invalid SASL method given. Method must be AMQP_SASL_METHOD_PLAIN or AMQP_SASL_METHOD_EXTERNAL.
