--TEST--
AMQPConnection getSocket
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
elseif (!getenv("PHP_AMQP_HOST")) print "skip PHP_AMQP_HOST environment variable is not set";
elseif (!extension_loaded("sockets")) print "skip sockets extension is not loaded";
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->connect();
$socket = $cnn->getSocket();
$socket = $cnn->getSocket();
var_dump($socket);
var_dump(socket_get_option($socket, SOL_SOCKET, SO_KEEPALIVE));
var_dump(socket_get_option($socket, SOL_SOCKET, TCP_NODELAY));
var_dump(socket_set_nonblock($socket));
var_dump(socket_read($socket, 1));
var_dump(socket_last_error($socket) === SOCKET_EWOULDBLOCK);
?>
--EXPECTF--
object(Socket)#%d (0) {
}
int(1)
int(0)
bool(true)
bool(false)
bool(true)