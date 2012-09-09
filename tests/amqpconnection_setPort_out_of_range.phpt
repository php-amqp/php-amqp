--TEST--
AMQPConnection setPort with int out of range
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$cnn = new AMQPConnection();
try {
    $cnn->setPort(1234567890);
} catch (Exception $e) {
    echo $e->getMessage();
}
?>
--EXPECT--
Invalid port given. Value must be between 1 and 65535.