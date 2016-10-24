--TEST--
AMQPQueue - declare with stalled reference
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

class QueueMock extends \AMQPQueue
{
    public function __construct(AMQPChannel $amqp_channel)
    {
    }
}

$cnn = new ConnectionMock();
$ch = new ChannelMock($cnn);

$e = new QueueMock($ch);


try {
    $e->declareQueue();
} catch (\Exception $e) {
    echo get_class($e), "({$e->getCode()}): ", $e->getMessage(), PHP_EOL;
}

?>
--EXPECT--
AMQPChannelException(0): Could not declare queue. Stale reference to the channel object.
