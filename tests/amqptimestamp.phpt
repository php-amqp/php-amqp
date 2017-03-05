--TEST--
AMQPTimestamp
--SKIPIF--
<?php
if (!extension_loaded("amqp") || version_compare(PHP_VERSION, '5.3', '<')) {
  print "skip";
}
--FILE--
<?php

$timestamp = new AMQPTimestamp(100000);
var_dump($timestamp->getTimestamp(), (string) $timestamp);

$timestamp = new AMQPTimestamp(100000.1);
var_dump($timestamp->getTimestamp(), (string) $timestamp);

new AMQPTimestamp();

new AMQPTimestamp("string");

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

var_dump(AMQPTimestamp::MAX);
var_dump(AMQPTimestamp::MIN);
?>

==END==
--EXPECTF--
string(6) "100000"
string(6) "100000"
string(6) "100000"
string(6) "100000"

Warning: AMQPTimestamp::__construct() expects exactly 1 parameter, 0 given in %s on line %d

Warning: AMQPTimestamp::__construct() expects parameter 1 to be %s, string given in %s on line %d
The timestamp parameter must be greater than 0.
The timestamp parameter must be less than 18446744073709551616.
bool(true)
string(20) "18446744073709551616"
string(1) "0"

==END==