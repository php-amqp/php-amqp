<?php
/**
 * stub class representing AMQPQueue from pecl-amqp
 */
class AMQPQueue
{
    /**
     * Acknowledge the receipt of a message.
     *
     * This method allows the acknowledgement of a message that is retrieved
     * without the AMQP_AUTOACK flag through AMQPQueue::get() or
     * AMQPQueue::consume()
     *
     * @param string  $delivery_tag The message delivery tag of which to
     *                              acknowledge receipt.
     * @param integer $flags        The only valid flag that can be passed is
     *                              AMQP_MULTIPLE.
     *
     * @throws AMQPChannelException    If the channel is not open.
     * @throws AMQPConnectionException If the connection to the broker was lost.
     *
     * @return boolean
     */
    public function ack(/** @noinspection PhpUnusedParameterInspection */ $delivery_tag, $flags = AMQP_NOPARAM)
    {
        return true;
    }

    /**
     * Bind the given queue to a routing key on an exchange.
     *
     * @param string $exchange_name Name of the exchange to bind to.
     * @param string $routing_key   Pattern or routing key to bind with.
     *
     * @throws AMQPChannelException    If the channel is not open.
     * @throws AMQPConnectionException If the connection to the broker was lost.
     *
     * @return boolean
     */
    public function bind (/** @noinspection PhpUnusedParameterInspection */ $exchange_name, $routing_key)
    {
        return true;
    }

    /**
     * Cancel a queue that is already bound to an exchange and routing key.
     *
     * @param string $consumer_tag The queue name to cancel, if the queue
     *                             object is not already representative of
     *                             a queue.
     *
     * @throws AMQPChannelException    If the channel is not open.
     * @throws AMQPConnectionException If the connection to the broker was lost.
     *
     * @return bool;
     */
    public function cancel (/** @noinspection PhpUnusedParameterInspection */ $consumer_tag = '')
    {
        return true;
    }

    /**
     * Create an instance of an AMQPQueue object.
     *
     * @param AMQPChannel $amqp_channel The amqp channel to use.
     *
     * @throws AMQPQueueException      When amqp_channel is not connected to a
     *                                 broker.
     * @throws AMQPConnectionException If the connection to the broker was lost.
     */
    public function __construct (AMQPChannel $amqp_channel)
    {
    }

    /**
     * Consume messages from a queue.
     *
     * Blocking function that will retrieve the next message from the queue as
     * it becomes available and will pass it off to the callback.
     *
     * @param callable $callback A callback function to which the
     *                           consumed message will be passed. The
     *                           function must accept at a minimum
     *                           one parameter, an AMQPEnvelope object,
     *                           and an optional second parameter
     *                           the AMQPQueue from which the message was
     *                           consumed. The AMQPQueue::consume() will
     *                           not return the processing thread back to
     *                           the PHP script until the callback
     *                           function returns FALSE.
     * @param integer  $flags    A bitmask of any of the flags: AMQP_NOACK.
     *
     * @throws AMQPChannelException    If the channel is not open.
     * @throws AMQPConnectionException If the connection to the broker was lost.
     *
     * @return void
     */
    public function consume (callable $callback, $flags = AMQP_NOPARAM)
    {
    }

    /**
     * Declare a new queue on the broker.
     *
     * @throws AMQPChannelException    If the channel is not open.
     * @throws AMQPConnectionException If the connection to the broker was lost.
     *
     * @return integer the message count.
     */
    public function declareQueue ()
    {
        return 0;
    }

    /**
     * Delete a queue from the broker.
     *
     * This includes its entire contents of unread or unacknowledged messages.
     *
     * @throws AMQPChannelException    If the channel is not open.
     * @throws AMQPConnectionException If the connection to the broker was lost.
     *
     * @return bool true on success or false on failure.
     */
    public function delete ()
    {
        return true;
    }

    /**
     * Retrieve the next message from the queue.
     *
     * Retrieve the next available message from the queue. If no messages are
     * present in the queue, this function will return FALSE immediately. This
     * is a non blocking alternative to the AMQPQueue::consume() method.
     * Currently, the only supported flag for the flags parameter is
     * AMQP_AUTOACK. If this flag is passed in, then the message returned will
     * automatically be marked as acknowledged by the broker as soon as the
     * frames are sent to the client.
     *
     * @param integer $flags A bitmask of supported flags for the
     *                       method call. Currently, the only the
     *                       supported flag is AMQP_AUTOACK. If this
     *                       value is not provided, it will use the
     *                       value of ini-setting amqp.auto_ack.
     *
     * @throws AMQPChannelException    If the channel is not open.
     * @throws AMQPConnectionException If the connection to the broker was lost.
     *
     * @return AMQPEnvelope|boolean
     */
    public function get (/** @noinspection PhpUnusedParameterInspection */$flags = AMQP_NOPARAM)
    {
        return new AMQPEnvelope();
    }

    /**
     * Get the argument associated with the given key.
     *
     * @param string $key The key to look up.
     *
     * @return string|integer|boolean The string or integer value associated
     *                                with the given key, or false if the key
     *                                is not set.
     */
    public function getArgument(/** @noinspection PhpUnusedParameterInspection */$key)
    {
        return '';
    }

    /**
     * Get all set arguments as an array of key/value pairs.
     *
     * @return array An array containing all of the set key/value pairs.
     */
    public function getArguments ()
    {
        return array();
    }

    /**
     * Get all the flags currently set on the given queue.
     *
     * @return int An integer bitmask of all the flags currently set on this
     *             exchange object.
     */
    public function getFlags ()
    {
        return 0;
    }

    /**
     * Get the configured name.
     *
     * @return string The configured name as a string.
     */
    public function getName ()
    {
        return '';
    }

    /**
     * Mark a message as explicitly not acknowledged.
     *
     * Mark the message identified by delivery_tag as explicitly not
     * acknowledged. This method can only be called on messages that have not
     * yet been acknowledged, meaning that messages retrieved with by
     * AMQPQueue::consume() and AMQPQueue::get() and using the AMQP_AUTOACK
     * flag are not eligible. When called, the broker will immediately put the
     * message back onto the queue, instead of waiting until the connection is
     * closed. This method is only supported by the RabbitMQ broker. The
     * behavior of calling this method while connected to any other broker is
     * undefined.
     *
     * @param string  $delivery_tag Delivery tag of last message to reject.
     * @param integer $flags        AMQP_REQUEUE to requeue the message(s).
     *
     * @throws AMQPChannelException    If the channel is not open.
     * @throws AMQPConnectionException If the connection to the broker was lost.
     *
     * @return boolean
     */
    public function nack (/** @noinspection PhpUnusedParameterInspection */$delivery_tag, $flags = AMQP_NOPARAM)
    {
        return true;
    }

    /**
     * Purge the contents of a queue.
     *
     * @throws AMQPChannelException    If the channel is not open.
     * @throws AMQPConnectionException If the connection to the broker was lost.
     *
     * @return boolean
     */
    public function purge ()
    {
        return true;
    }

    /**
     * Set a queue argument.
     *
     * @param string $key   The key to set.
     * @param mixed  $value The value to set.
     *
     * @return boolean
     */
    public function setArgument (/** @noinspection PhpUnusedParameterInspection */$key, $value)
    {
        return true;
    }

    /**
     * Set all arguments on the given queue.
     *
     * All other argument settings will be wiped.
     *
     * @param array $arguments An array of key/value pairs of arguments.
     *
     * @return boolean
     */
    public function setArguments (/** @noinspection PhpUnusedParameterInspection */array $arguments)
    {
        return true;
    }

    /**
     * Set the flags on the queue.
     *
     * @param integer $flags A bitmask of flags:
     *                       AMQP_DURABLE, AMQP_PASSIVE,
     *                       AMQP_EXCLUSIVE, AMQP_AUTODELETE.
     *
     * @return boolean
     */
    public function setFlags (/** @noinspection PhpUnusedParameterInspection */$flags)
    {
        return true;
    }

    /**
     * Set the queue name.
     *
     * @param string $queue_name The name of the queue.
     *
     * @return boolean
     */
    public function setName (/** @noinspection PhpUnusedParameterInspection */$queue_name)
    {
        return true;
    }

    /**
     * Remove a routing key binding on an exchange from the given queue.
     *
     * @param string $exchange_name The name of the exchange on which the
     *                              queue is bound.
     * @param string $routing_key   The binding routing key used by the
     *                              queue.
     *
     * @throws AMQPChannelException    If the channel is not open.
     * @throws AMQPConnectionException If the connection to the broker was lost.
     *
     * @return boolean
     */
    public function unbind (/** @noinspection PhpUnusedParameterInspection */$exchange_name, $routing_key)
    {
        return true;
    }
}

