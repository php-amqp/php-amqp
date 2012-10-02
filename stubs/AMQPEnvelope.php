<?php
/**
 * stub class representing AMQPEnvelope from pecl-amqp
 */
class AMQPEnvelope
{
    /**
     * Get the application id of the message.
     *
     * @return string The application id of the message.
     */
    public function getAppId()
    {
        return '';
    }

    /**
     * Get the body of the message.
     *
     * @return string The contents of the message body.
     */
    public function getBody()
    {
        return '';
    }

    /**
     * Get the content encoding of the message.
     *
     * @return string The content encoding of the message.
     */
    public function getContentEncoding()
    {
        return '';
    }

    /**
     * Get the message content type.
     *
     * @return string The content type of the message.
     */
    public function getContentType()
    {
        return '';
    }

    /**
     * Get the message correlation id.
     *
     * @return string The correlation id of the message.
     */
    public function getCorrelationId()
    {
        return '';
    }

    /**
     * Get the delivery tag of the message.
     *
     * @return string The delivery tag of the message.
     */
    public function getDeliveryTag()
    {
        return '';
    }

    /**
     * Get the exchange name on which the message was published.
     *
     * @return string The exchange name on which the message was published.
     */
    public function getExchange()
    {
        return '';
    }

    /**
     * Get the expiration of the message.
     *
     * @return string The message expiration.
     */
    public function getExpiration()
    {
        return '';
    }

    /**
     * Get a specific message header.
     *
     * @param string $header_key Name of the header to get the value from.
     *
     * @return string|boolean The contents of the specified header or FALSE
     *                        if not set.
     */
    public function getHeader(/** @noinspection PhpUnusedParameterInspection */$header_key)
    {
        return '';
    }

    /**
     * Get the headers of the message.
     *
     * @return array An array of key value pairs associated with the message.
     */
    public function getHeaders()
    {
        return array();
    }

    /**
     * Get the message id of the message.
     *
     * @return string The message id
     */
    public function getMessageId()
    {
        return '';
    }

    /**
     * Get the priority of the message.
     *
     * @todo verify return type. docs wrong?
     *
     * @return string The message priority.
     */
    public function getPriority()
    {
        return '';
    }

    /**
     * Get the reply-to address of the message.
     *
     * @return string The contents of the reply to field.
     */
    public function getReplyTo()
    {
        return '';
    }

    /**
     * Get the routing key of the message.
     *
     * @return string The message routing key.
     */
    public function getRoutingKey()
    {
        return '';
    }

    /**
     * Get the timestamp of the message.
     *
     * @return string The message timestamp.
     */
    public function getTimeStamp()
    {
        return '';
    }

    /**
     * Get the message type.
     *
     * @return string The message type.
     */
    public function getType()
    {
        return '';
    }

    /**
     * Get the message user id.
     *
     * @return string The message user id.
     */
    public function getUserId()
    {
        return '';
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
        return true;
    }
}

