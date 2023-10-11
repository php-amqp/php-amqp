<?php

/**
 * stub class representing AMQPEnvelope from pecl-amqp
 */
class AMQPEnvelope extends AMQPBasicProperties
{
    private string $body = '';

    private ?string $consumerTag = null;

    private ?int $deliveryTag = null;

    private bool $isRedelivery = false;

    private ?string $exchangeName = null;

    private string $routingKey = '';

    public function __construct()
    {
    }

    /**
     * Get the body of the message.
     *
     * @return string The contents of the message body.
     */
    public function getBody(): string
    {
    }

    /**
     * Get the routing key of the message.
     *
     * @return string The message routing key.
     */
    public function getRoutingKey(): string
    {
    }

    /**
     * Get the consumer tag of the message.
     *
     * @return string|null The consumer tag of the message.
     */
    public function getConsumerTag(): ?string
    {
    }

    /**
     * Get the delivery tag of the message.
     *
     * @return integer|null The delivery tag of the message.
     */
    public function getDeliveryTag(): ?int
    {
    }

    /**
     * Get the exchange name on which the message was published.
     *
     * @return string|null The exchange name on which the message was published.
     */
    public function getExchangeName(): ?string
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
    public function isRedelivery(): bool
    {
    }

    /**
     * Get a specific message header.
     *
     * @param string $headerName Name of the header to get the value from.
     *
     * @return mixed The contents of the specified header or null if not set.
     */
    public function getHeader(string $headerName)
    {
    }

    /**
     * Check whether specific message header exists.
     *
     * @param string $headerName Name of the header to check.
     *
     * @return boolean
     */
    public function hasHeader(string $headerName): bool
    {
    }
}
