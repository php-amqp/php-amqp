<?php
/**
 * stub class representing AMQPConsumer from pecl-amqp
 */
class AMQPConsumer
{
    /**
     * Create an instance of an AMQPQueue object.
     *
     * @param AMQPQueue $queue The queue from which to consume
     *
     * @param callable $fnCallback The function to call when a message is received
     */
    public function __construct (
        AMQPQueue $queue, 
        callable $fnCallback
    ) {
    }
    
    /**
     * Return the queue object bound to this consumer
     *
     * @return AMQPQueue
     */
    public function getQueue()
    {
    }

    /**
     * Start consuming messages from the bound queue
     *
     * @param integer  $flags       A bitmask of any of the flags: AMQP_AUTOACK.
     * @param string   $consumerTag A string describing this consumer. Used
     *                              for canceling subscriptions with cancel().
     *
     * @throws AMQPChannelException    If the channel is not open.
     * @throws AMQPConnectionException If the connection to the broker was lost.
     *
     * @return void
     */
    public function basicConsume (
        $flags = AMQP_NOPARAM,
        $consumerTag = null
    ) {
    }

    /**
     * Consume one message from the queue and return the status of the callback
     *
     * @throws AMQPChannelException    If the channel is not open.
     * @throws AMQPConnectionException If the connection to the broker was lost.
     *
     * @return boolean True if the callback function returned a true value
     */
    public function consumeOne ()
    {
    }
}

