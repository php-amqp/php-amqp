--TEST--
AMQPConnection parameter validation
--SKIPIF--
<?php
if (!extension_loaded("amqp")) {
    print "skip";
}
?>
--FILE--
<?php
$parameters = [
    ['login', 'setLogin', 'getLogin', [str_repeat('X', 1025), 'user']],
    ['password', 'setPassword', 'getPassword', [str_repeat('X', 1025), 'pass']],
    ['host', 'setHost', 'getHost', [str_repeat('X', 513), 'host']],
    ['vhost', 'setVhost', 'getVhost', [str_repeat('X', 513), 'vhost']],
    ['port', 'setPort', 'getPort', [-1, 65536, 1234]],
    ['timeout', 'setTimeout', 'getTimeout', [-1], 10],
    ['read_timeout', 'setReadTimeout', 'getReadTimeout', [-1], 20],
    ['write_timeout', 'setWriteTimeout', 'getWriteTimeout', [-1], 30],
    ['connect_timeout', null, 'getConnectTimeout', [-1], 40],
    ['rpc_timeout', 'setRpcTimeout', 'getRpcTimeout', [-1], 50],
    ['frame_max', null, 'getMaxFrameSize', [-1, PHP_INT_MAX + 1], 128],
    ['channel_max', null, 'getMaxChannels', [-1, 257], 128],
    ['heartbeat', null, 'getHeartbeatInterval', [-1, PHP_INT_MAX + 1], 250],
];

foreach ($parameters as $args) {
    list($prop, $setter, $getter, $values) = $args;
    foreach ($values as $value) {
        try {
            $con1 = new AMQPConnection([$prop => $value]);
            echo $getter . " after constructor: ";
            echo $con1->{$getter}();
            echo PHP_EOL;
        } catch (\Throwable $t) {
            echo get_class($t);
            echo ": ";
            echo $t->getMessage();
            echo PHP_EOL;
        }
        if ($setter === null) {
            continue;
        }
        $con2 = new AMQPConnection();
        try {
            $con2->{$setter}($value);
            echo $getter . " after setter: ";
            echo $con2->{$getter}();
            echo PHP_EOL;
        } catch (\Throwable $t) {
            echo get_class($t);
            echo ": ";
            echo $t->getMessage();
            echo PHP_EOL;
        }
    }
}

?>
==DONE==
--EXPECTF--
AMQPConnectionException: Parameter 'login' exceeds 1024 character limit.
AMQPConnectionException: Parameter 'login' exceeds 1024 character limit.
getLogin after constructor: user
getLogin after setter: user
AMQPConnectionException: Parameter 'password' exceeds 1024 character limit.
AMQPConnectionException: Parameter 'password' exceeds 1024 character limit.
getPassword after constructor: pass
getPassword after setter: pass
AMQPConnectionException: Parameter 'host' exceeds 512 character limit.
AMQPConnectionException: Parameter 'host' exceeds 512 character limit.
getHost after constructor: host
getHost after setter: host
AMQPConnectionException: Parameter 'vhost' exceeds 512 character limit.
AMQPConnectionException: Parameter 'vhost' exceeds 512 characters limit.
getVhost after constructor: vhost
getVhost after setter: vhost
AMQPConnectionException: Parameter 'port' must be a valid port number  between 1 and 65535.
AMQPConnectionException: Invalid port given. Value must be between 1 and 65535.
AMQPConnectionException: Parameter 'port' must be a valid port number  between 1 and 65535.
AMQPConnectionException: Invalid port given. Value must be between 1 and 65535.
getPort after constructor: 1234
getPort after setter: 1234

Deprecated: AMQPConnection::__construct(): Parameter 'timeout' is deprecated; use 'read_timeout' instead in %s on line %d
AMQPConnectionException: Parameter 'timeout' must be greater than or equal to zero.

Deprecated: AMQPConnection::setTimeout(): AMQPConnection::setTimeout($timeout) method is deprecated; use AMQPConnection::setReadTimeout($timeout) instead in %s on line %d
AMQPConnectionException: Parameter 'timeout' must be greater than or equal to zero.
AMQPConnectionException: Parameter 'read_timeout' must be greater than or equal to zero.
AMQPConnectionException: Parameter 'readTimeout' must be greater than or equal to zero.
AMQPConnectionException: Parameter 'write_timeout' must be greater than or equal to zero.
AMQPConnectionException: Parameter 'writeTimeout' must be greater than or equal to zero.
AMQPConnectionException: Parameter 'connect_timeout' must be greater than or equal to zero.
AMQPConnectionException: Parameter 'rpc_timeout' must be greater than or equal to zero.
AMQPConnectionException: Parameter 'rpcTimeout' must be greater than or equal to zero.
AMQPConnectionException: Parameter 'frame_max' is out of range.
AMQPConnectionException: Parameter 'frame_max' is out of range.
AMQPConnectionException: Parameter 'channel_max' is out of range.
AMQPConnectionException: Parameter 'channel_max' is out of range.
AMQPConnectionException: Parameter 'heartbeat' is out of range.
AMQPConnectionException: Parameter 'heartbeat' is out of range.
==DONE==