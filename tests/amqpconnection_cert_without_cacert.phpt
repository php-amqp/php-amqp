--TEST--
AMQPConnection - TLS client cert/key without cacert is rejected instead of falling back to plaintext
--EXTENSIONS--
amqp
--SKIPIF--
<?php
if (!getenv("PHP_AMQP_HOST")) print "skip PHP_AMQP_HOST environment variable is not set";
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->setCert('/nonexistent/client.pem');
$cnn->setKey('/nonexistent/client.key');

try {
    $cnn->connect();
    echo "no exception\n";
} catch (AMQPConnectionException $e) {
    echo $e::class, ': ', $e->getMessage(), "\n";
}
?>
--EXPECT--
AMQPConnectionException: Socket error: a TLS client certificate or key was configured without a CA certificate (cacert); refusing to fall back to an unencrypted connection.
