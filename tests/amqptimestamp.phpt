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

new AMQPTimestamp();

new AMQPTimestamp("string");

try {
    new AMQPTimestamp(-1);
} catch (AMQPValueException $e) {
    echo $e .  "\n";
}

?>

==END==
--EXPECTF--
string(6) "100000"
string(6) "100000"

Warning: AMQPTimestamp::__construct() expects exactly 1 parameter, 0 given in %s on line %d

Warning: AMQPTimestamp::__construct() expects parameter 1 to be float, string given in %s on line %d
AMQPValueException: The timestamp parameter must be greater than 0. in %s.php:%d
Stack trace:
#0 %s.php(%d): AMQPTimestamp->__construct(-1)
#1 {main}

==END==