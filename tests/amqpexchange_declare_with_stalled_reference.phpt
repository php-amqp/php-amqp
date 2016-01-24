--TEST--
AMQPExchange - declare with stalled reference
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
class ConnectionMock extends  AMQPConnection {
    public function __construct(array $credentials = array())
    {
    }
}

class ChannelMock extends AMQPChannel {
    public function __construct(AMQPConnection $amqp_connection)
    {
    }
}

class ExchangeMock extends \AMQPExchange
{
    public function __construct(AMQPChannel $amqp_channel)
    {
    }
}

$cnn = new ConnectionMock();
$ch = new ChannelMock($cnn);

$e = new ExchangeMock($ch);


try {
    $e->declareExchange();
} catch (\Exception $e) {
    echo get_class($e), ': ', $e->getMessage(), PHP_EOL;
}

?>
--EXPECT--
AMQPChannelException: Could not declare exchange. Stale reference to the channel object.
