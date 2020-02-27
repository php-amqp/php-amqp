--TEST--
Params are passed by value in AMQPConnection::__construct()

--SKIPIF--
<?php if (!extension_loaded("amqp")) die("Skip: ext/amqp must be installed."); ?>

--FILE--
<?php

$params = [
    'host'            => 'localhost',
    'port'            => 5432,
    'login'           => 'login',
    'password'        => 'password',
    'read_timeout'    => 10,
    'write_timeout'   => 10,
    'connect_timeout' => 10,
    'rpc_timeout'     => 10,
    'connection_name' => 'custom_connection_name'
];

$conn = new \AMQPConnection($params);

echo gettype($params['host']) . PHP_EOL;
echo gettype($params['port']) . PHP_EOL;
echo gettype($params['login']) . PHP_EOL;
echo gettype($params['password']) . PHP_EOL;

echo gettype($params['read_timeout']) . PHP_EOL;
echo gettype($params['write_timeout']) . PHP_EOL;
echo gettype($params['connect_timeout']) . PHP_EOL;
echo gettype($params['rpc_timeout']) . PHP_EOL;
echo gettype($params['connection_name']) . PHP_EOL;

--EXPECT--
string
integer
string
string
integer
integer
integer
integer
string
