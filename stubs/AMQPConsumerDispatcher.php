<?php
/**
 * stub class representing AMQPConsumerDispatcher from pecl-amqp
 */
class AMQPConsumerDispatcher
{
    /**
     * Create an instance of an AMQPConsumerDispatcher object.
     *
     * @param array $consumers The consumers from which to consume
     *
     */
    public function __construct (
        array $consumers 
    ) {
    }
    
    /**
     * Returns true if there are any consumers still bound
     *
     * @return boolean
     */
    public function hasConsumers()
    {
    }

    /**
     * Remove a consumer from the list
     *
     * @param AMQPConsumer  $consumer The consumer to remove
     *
     * @return void
     */
    public function removeConsumer (
        AMQPConsumer $consumer
    ) {
    }

    /**
     * Wait for a message to arrive on one of the consumers' queues, or the timeout to expire
     *
     * @param int $timeout The maximum time in seconds to wait for a message before returning null
     *
     * @throws AMQPChannelException    If the channel is not open.
     * @throws AMQPConnectionException If the connection to the broker was lost.
     *
     * @return AMQPConsumer if a message was received, or null if the timeout expired
     */
    public function select (
        $timeout = 0
    ) {
    }
}

