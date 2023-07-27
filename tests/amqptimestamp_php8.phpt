--TEST--
AMQPTimestamp
--SKIPIF--
<?php
if (!extension_loaded("amqp") || version_compare(PHP_VERSION, '8.0', '<')) {
  print "skip";
}
--FILE--
<?php

$timestamp = new AMQPTimestamp(100000);
var_dump($timestamp->getTimestamp(), (string) $timestamp);

$timestamp = new AMQPTimestamp(100000.1);
var_dump($timestamp->getTimestamp(), (string) $timestamp);

try {
	new AMQPTimestamp();
} catch(ArgumentCountError $e) {
	echo $e->getMessage() . "\n";
}
try {
	new AMQPTimestamp("string");
} catch(TypeError $e) {
	echo $e->getMessage() . "\n";
}

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
float(100000)
string(6) "100000"
float(100000)
string(6) "100000"
AMQPTimestamp::__construct() expects exactly 1 argument, 0 given
AMQPTimestamp::__construct(): Argument #1 ($timestamp) must be of type float, string given
The timestamp parameter must be greater than 0.
The timestamp parameter must be less than 18446744073709551616.
bool(true)
float(1.8446744073709552E+19)
float(0)

==END==
