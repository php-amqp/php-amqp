<?php

/**
 * stub class representing AMQPEnvelope from pecl-amqp
 */
class AMQPEnvelope extends AMQPBasicProperties
{
    public function __construct()
    {
    }

    /**
     * Get the body of the message.
     *
     * @return string The contents of the message body.
     */
    public function getBody()
    {
    }

    /**
     * Get the routing key of the message.
     *
     * @return string The message routing key.
     */
    public function getRoutingKey()
    {
    }

    /**
     * Get the consumer tag of the message.
     *
     * @return string|null The consumer tag of the message.
     */
    public function getConsumerTag()
    {
    }

    /**
     * Get the delivery tag of the message.
     *
     * @return integer|null The delivery tag of the message.
     */
    public function getDeliveryTag()
    {
    }

    /**
     * Get the exchange name on which the message was published.
     *
     * @return string|null The exchange name on which the message was published.
     */
    public function getExchangeName()
    {
    }

    /**
     * Whether this is a redelivery of the message.
     *
     * Whether this is a redelivery of a message. If this message has been
     * delivered and AMQPEnvelope::nack() was called, the message will be put
     * back on the queue to be redelivered, at which point the message will
     * always return TRUE when this method is called.
     *
     * @return bool TRUE if this is a redelivery, FALSE otherwise.
     */
    public function isRedelivery()
    {
    }

    /**
     * Get a specific message header.
     *
     * @param string $name Name of the header to get the value from.
     *
     * @return string|null The contents of the specified header or null if not set.
     */
    public function getHeader($name)
    {
    }

    /**
     * Check whether specific message header exists.
     *
     * @param string $name Name of the header to check.
     *
     * @return boolean
     */
    public function hasHeader($name)
    {
    }
}
