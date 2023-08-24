--TEST--
AMQPTimestamp
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip";
?>
--FILE--
<?php

$timestamp = new AMQPTimestamp(100000);
var_dump($timestamp->getTimestamp(), (string) $timestamp);
var_dump($timestamp === $timestamp->toAmqpValue());
var_dump($timestamp instanceof AMQPValue);

$timestamp = new AMQPTimestamp(100000.1);
var_dump($timestamp->getTimestamp(), (string) $timestamp);

try {
    new AMQPTimestamp(AMQPTimestamp::MIN - 1);
} catch (AMQPValueException $e) {
    echo $e->getMessage() . "\n";
}

try {
    new AMQPTimestamp(INF);
} catch (AMQPValueException $e) {
    echo $e->getMessage() . "\n";
}

var_dump((new ReflectionClass("AMQPTimestamp"))->isFinal());

var_dump(number_format(AMQPTimestamp::MAX, 0, '.', '_'));
var_dump(AMQPTimestamp::MIN);
?>

==END==
--EXPECTF--
float(100000)
string(6) "100000"
bool(true)
bool(true)
float(100000)
string(6) "100000"
The timestamp parameter must be greater than 0.
The timestamp parameter must be less than 18446744073709551616.
bool(true)
string(26) "18_446_744_073_709_551_616"
float(0)

==END==