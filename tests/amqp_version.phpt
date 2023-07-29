--TEST--
AMQP extension version constants
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
var_dump(AMQP_EXTENSION_VERSION);
var_dump(AMQP_EXTENSION_VERSION_MAJOR);
var_dump(AMQP_EXTENSION_VERSION_MINOR);
var_dump(AMQP_EXTENSION_VERSION_PATCH);
var_dump(AMQP_EXTENSION_VERSION_EXTRA);
var_dump(AMQP_EXTENSION_VERSION_ID);
?>
==DONE==
--EXPECTF--
string(%d) "%d.%d.%s"
int(%d)
int(%d)
int(%d)
string(%d) "%s"
int(%d)
==DONE==