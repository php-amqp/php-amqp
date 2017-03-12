--TEST--
AMQPDecimal
--SKIPIF--
<?php
if (!extension_loaded("amqp") || version_compare(PHP_VERSION, '5.3', '<')) {
  print "skip";
}
--FILE--
<?php

$decimal = new AMQPDecimal(1, 2);
var_dump($decimal->getExponent(), $decimal->getSignificand());

$decimal = new AMQPDecimal(1.1, 2.2);
var_dump($decimal->getExponent(), $decimal->getSignificand());


try {
    new AMQPDecimal(-1, 1);
} catch (AMQPValueException $e) {
    echo $e->getMessage() . "\n";
}

try {
    new AMQPDecimal(1, -1);
} catch (AMQPValueException $e) {
    echo $e->getMessage() . "\n";
}


try {
    new AMQPDecimal(AMQPDecimal::EXPONENT_MAX+1, 1);
} catch (AMQPValueException $e) {
    echo $e->getMessage() . "\n";
}

try {
    new AMQPDecimal(1, AMQPDecimal::SIGNIFICAND_MAX+1);
} catch (AMQPValueException $e) {
    echo $e->getMessage() . "\n";
}


var_dump((new ReflectionClass("AMQPDecimal"))->isFinal());

var_dump(AMQPDecimal::EXPONENT_MIN);
var_dump(AMQPDecimal::EXPONENT_MAX);
var_dump(AMQPDecimal::SIGNIFICAND_MIN);
var_dump(AMQPDecimal::SIGNIFICAND_MAX);
?>

==END==
--EXPECT--
int(1)
int(2)
int(1)
int(2)
Decimal exponent value must be unsigned.
Decimal significand value must be unsigned.
Decimal exponent value must be less than 255.
Decimal significand value must be less than 4294967295.
bool(true)
int(0)
int(255)
int(0)
int(4294967295)

==END==
