--TEST--
AMQPConnection - setters and nullability
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$c = new AMQPConnection();

$c->setKey('key');
var_dump($c->getKey());
$c->setKey(null);
var_dump($c->getKey());
$c->setKey('');
var_dump($c->getKey());

$c->setCert('cert');
var_dump($c->getCert());
$c->setCert(null);
var_dump($c->getCert());
$c->setCert('');
var_dump($c->getCert());

$c->setCACert('cacert');
var_dump($c->getCACert());
$c->setCACert(null);
var_dump($c->getCACert());
$c->setCACert('');
var_dump($c->getCACert());

$c->setConnectionName('con');
var_dump($c->getConnectionName());
$c->setConnectionName(null);
var_dump($c->getConnectionName());
$c->setConnectionName('');
var_dump($c->getConnectionName());
?>
==DONE==
--EXPECT--
string(3) "key"
NULL
string(0) ""
string(4) "cert"
NULL
string(0) ""
string(6) "cacert"
NULL
string(0) ""
string(3) "con"
NULL
string(0) ""
==DONE==