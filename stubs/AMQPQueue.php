<?php

/**
 * stub class representing AMQPQueue from pecl-amqp
 */
class AMQPQueue
{
    /**
     * Create an instance of an AMQPQueue object.
     *
     * @param AMQPChannel $channel The amqp channel to use.
     *
     * @throws AMQPQueueException      When amqp_channel is not connected to a
     *                                 broker.
     * @throws AMQPConnectionException If the connection to the broker was lost.
     */
    public function __construct(AMQPChannel $channel)
    {
    }

    /**
     * Acknowledge the receipt of a message.
     *
     * This method allows the acknowledgement of a message that is retrieved
     * without the AMQP_AUTOACK flag through AMQPQueue::get() or
     * AMQPQueue::consume()
     *
     * @param integer $deliveryTag The message delivery tag of which to
     *                              acknowledge receipt.
     * @param integer $flags        The only valid flag that can be passed is
     *                              AMQP_MULTIPLE.
     *
     * @return void
     *@throws AMQPConnectionException If the connection to the broker was lost.
     *
     * @throws AMQPChannelException    If the channel is not open.
     */
    public function ack($deliveryTag, $flags = AMQP_NOPARAM)
    {
    }

    /**
     * Bind the given queue to a routing key on an exchange.
     *
     * @param string $exchangeName Name of the exchange to bind to.
     * @param string $routingKey   Pattern or routing key to bind with.
     * @param array  $arguments     Additional binding arguments.
     *
     * @return void
     *@throws AMQPConnectionException If the connection to the broker was lost.
     *
     * @throws AMQPChannelException    If the channel is not open.
     */
    public function bind($exchangeName, $routingKey = null, array $arguments = array())
    {
    }

    /**
     * Cancel a queue that is already bound to an exchange and routing key.
     *
     * @param string $consumerTag The consumer tag to cancel. If no tag is provided,
     *                             or it is empty string, the latest consumer
     *                             tag on this queue will be taken and after
     *                             the successful cancellation request it will set to null.
     *                             If the consumer_tag parameter is empty and the latest
     *                             consumer tag is empty, no `basic.cancel` request will be
     *                             sent.
     *                             If either the consumer tag passed matches the latest tag
     *                             or no consumer tag was passed and the latest tag was used
     *                             the internal consumer tag will be set to null, so that
     *                             `AMQPQueue::getConsumerTag()` will return null afterwards.
     *
     * @return void
     *@throws AMQPConnectionException If the connection to the broker was lost.
     *
     * @throws AMQPChannelException    If the channel is not open.
     */
    public function cancel($consumerTag = '')
    {
    }

    /**
     * Consume messages from a queue.
     *
     * Blocking function that will retrieve the next message from the queue as
     * it becomes available and will pass it off to the callback.
     *
     * @param callable|null $callback    A callback function to which the
     *                              consumed message will be passed. The
     *                              function must accept at a minimum
     *                              one parameter, an AMQPEnvelope object,
     *                              and an optional second parameter
     *                              the AMQPQueue object from which callback
     *                              was invoked. The AMQPQueue::consume() will
     *                              not return the processing thread back to
     *                              the PHP script until the callback
     *                              function returns FALSE.
     *                              If the callback is omitted or null is passed,
     *                              then the messages delivered to this client will
     *                              be made available to the first real callback
     *                              registered. That allows one to have a single
     *                              callback consuming from multiple queues.
     * @param integer $flags        A bitmask of any of the flags: AMQP_AUTOACK,
     *                              AMQP_JUST_CONSUME. Note: when AMQP_JUST_CONSUME
     *                              flag used all other flags are ignored and
     *                              $consumerTag parameter has no sense.
     *                              AMQP_JUST_CONSUME flag prevent from sending
     *                              `basic.consume` request and just run $callback
     *                              if it provided. Calling method with empty $callback
     *                              and AMQP_JUST_CONSUME makes no sense.
     * @param string|null $consumerTag     A string describing this consumer. Used
     *                              for canceling subscriptions with cancel().
     *
     * @throws AMQPChannelException    If the channel is not open.
     * @throws AMQPConnectionException If the connection to the broker was lost.
     * @throws AMQPEnvelopeException   When no queue found for envelope.
     * @throws AMQPQueueException      If timeout occurs or queue is not exists.
     *
     * @return void
     */
    public function consume(
        callable $callback = null,
        $flags = AMQP_NOPARAM,
        $consumerTag = null
    ) {
    }

    /**
     * Declare a new queue on the broker.
     *
     * @throws AMQPChannelException    If the channel is not open.
     * @throws AMQPConnectionException If the connection to the broker was lost.
     * @throws AMQPQueueException      On failure.
     *
     * @return integer the message count.
     */
    public function declareQueue()
    {
    }

    /**
     * Delete a queue from the broker.
     *
     * This includes its entire contents of unread or unacknowledged messages.
     *
     * @param integer $flags        Optionally AMQP_IFUNUSED can be specified
     *                              to indicate the queue should not be
     *                              deleted until no clients are connected to
     *                              it.
     *
     * @throws AMQPChannelException    If the channel is not open.
     * @throws AMQPConnectionException If the connection to the broker was lost.
     *
     * @return integer The number of deleted messages.
     */
    public function delete($flags = AMQP_NOPARAM)
    {
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
     * @throws AMQPQueueException      If queue is not exist.
     *
     * @return AMQPEnvelope|null
     */
    public function get($flags = AMQP_NOPARAM)
    {
    }

    /**
     * Get the argument associated with the given key.
     *
     * @param string $argumentName The key to look up.
     *
     * @return string|integer|null The string or integer value associated
     *                                with the given key, or false if the key
     *                                is not set.
     */
    public function getArgument($argumentName)
    {
    }

    /**
     * Get all set arguments as an array of key/value pairs.
     *
     * @return array An array containing all the set key/value pairs.
     */
    public function getArguments()
    {
    }

    /**
     * Get all the flags currently set on the given queue.
     *
     * @return int An integer bitmask of all the flags currently set on this
     *             exchange object.
     */
    public function getFlags()
    {
    }

    /**
     * Get the configured name.
     *
     * @return string|null The configured name as a string.
     */
    public function getName()
    {
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
     * @param integer $deliveryTag Delivery tag of last message to reject.
     * @param integer $flags        AMQP_REQUEUE to requeue the message(s),
     *                              AMQP_MULTIPLE to nack all previous
     *                              unacked messages as well.
     *
     * @return void
     *@throws AMQPConnectionException If the connection to the broker was lost.
     *
     * @throws AMQPChannelException    If the channel is not open.
     */
    public function nack($deliveryTag, $flags = AMQP_NOPARAM)
    {
    }

    /**
     * Mark one message as explicitly not acknowledged.
     *
     * Mark the message identified by delivery_tag as explicitly not
     * acknowledged. This method can only be called on messages that have not
     * yet been acknowledged, meaning that messages retrieved with by
     * AMQPQueue::consume() and AMQPQueue::get() and using the AMQP_AUTOACK
     * flag are not eligible.
     *
     * @param integer $deliveryTag Delivery tag of the message to reject.
     * @param integer $flags        AMQP_REQUEUE to requeue the message(s).
     *
     * @return void
     *@throws AMQPConnectionException If the connection to the broker was lost.
     *
     * @throws AMQPChannelException    If the channel is not open.
     */
    public function reject($deliveryTag, $flags = AMQP_NOPARAM)
    {
    }

    /**
     * Purge the contents of a queue.
     *
     * Returns the number of purged messages
     *
     * @throws AMQPChannelException    If the channel is not open.
     * @throws AMQPConnectionException If the connection to the broker was lost.
     *
     * @return int
     */
    public function purge()
    {
    }

    /**
     * Set a queue argument.
     *
     * @param string $argumentName   The key to set.
     * @param mixed  $argumentValue The value to set.
     *
     * @return void
     */
    public function setArgument($argumentName, $argumentValue)
    {
    }

    /**
     * Set all arguments on the given queue.
     *
     * All other argument settings will be wiped.
     *
     * @param array $arguments An array of key/value pairs of arguments.
     *
     * @return void
     */
    public function setArguments(array $arguments)
    {
    }

    /**
     * Check whether a queue has specific argument.
     *
     * @param string $argumentName   The key to check.
     *
     * @return boolean
     */
    public function hasArgument($argumentName)
    {
    }

    /**
     * Set the flags on the queue.
     *
     * @param integer $flags A bitmask of flags:
     *                       AMQP_DURABLE, AMQP_PASSIVE,
     *                       AMQP_EXCLUSIVE, AMQP_AUTODELETE.
     *
     * @return void
     */
    public function setFlags($flags)
    {
    }

    /**
     * Set the queue name.
     *
     * @param string $name The name of the queue.
     *
     * @return void
     */
    public function setName($name)
    {
    }

    /**
     * Remove a routing key binding on an exchange from the given queue.
     *
     * @param string $exchangeName The name of the exchange on which the
     *                              queue is bound.
     * @param string $routingKey   The binding routing key used by the
     *                              queue.
     * @param array  $arguments     Additional binding arguments.
     *
     * @return void
     *@throws AMQPConnectionException If the connection to the broker was lost.
     *
     * @throws AMQPChannelException    If the channel is not open.
     */
    public function unbind($exchangeName, $routingKey = null, array $arguments = array())
    {
    }

    /**
     * Get the AMQPChannel object in use
     *
     * @return AMQPChannel
     */
    public function getChannel()
    {
    }

    /**
     * Get the AMQPConnection object in use
     *
     * @return AMQPConnection
     */
    public function getConnection()
    {
    }

    /**
     * Get latest consumer tag. If no consumer available or the latest on was canceled null will be returned.
     *
     * @return string|null
     */
    public function getConsumerTag()
    {
    }
}
