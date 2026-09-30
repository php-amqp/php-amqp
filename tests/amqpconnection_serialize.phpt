--TEST--
AMQPConnection and AMQPChannel cannot be serialized
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
?>
--FILE--
<?php
try {
    serialize(new AMQPConnection(['password' => 'secret']));
} catch (Exception $e) {
    echo get_class($e), ": ", $e->getMessage(), PHP_EOL;
}

// Before PHP 8.1, only the C: format reaches the handler that throws
$format = PHP_VERSION_ID < 80100 ? 'C' : 'O';

foreach (['AMQPConnection', 'AMQPChannel'] as $class) {
    try {
        unserialize($format . ':' . strlen($class) . ':"' . $class . '":0:{}');
    } catch (Exception $e) {
        echo get_class($e), ": ", $e->getMessage(), PHP_EOL;
    }
}
?>
--EXPECT--
Exception: Serialization of 'AMQPConnection' is not allowed
Exception: Unserialization of 'AMQPConnection' is not allowed
Exception: Unserialization of 'AMQPChannel' is not allowed
