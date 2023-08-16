<?php

/**
 * stub class representing AMQPExchange from pecl-amqp
 */
class AMQPExchange
{
    private AMQPConnection $connection;

    private AMQPChannel $channel;

    private ?string $name = null;

    private ?string $type = null;

    private bool $passive = false;

    private bool $durable = false;

    private bool $autoDelete = false;

    private bool $internal = false;

    private array $arguments = [];

    /**
     * Create an instance of AMQPExchange.
     *
     * Returns a new instance of an AMQPExchange object, associated with the
     * given AMQPChannel object.
     *
     * @param AMQPChannel $channel A valid AMQPChannel object, connected
     *                             to a broker.
     *
     * @throws AMQPExchangeException When amqp_channel is not connected to
     *                               a broker.
     * @throws AMQPConnectionException If the connection to the broker was
     *                                 lost.
     */
    public function __construct(AMQPChannel $channel)
    {
    }

    /**
     * Bind to another exchange.
     *
     * Bind an exchange to another exchange using the specified routing key.
     *
     * @param string $exchangeName Name of the exchange to bind.
     * @param string|null $routingKey The routing key to use for binding.
     * @param array $arguments Additional binding arguments.
     *
     * @throws AMQPChannelException If the channel is not open.
     * @throws AMQPConnectionException If the connection to the broker was lost.
     * @throws AMQPExchangeException On failure.
     */
    public function bind(string $exchangeName, ?string $routingKey = null, array $arguments = []): void
    {
    }

    /**
     * Remove binding to another exchange.
     *
     * Remove a routing key binding on an another exchange from the given exchange.
     *
     * @param string $exchangeName Name of the exchange to bind.
     * @param string|null $routingKey The routing key to use for binding.
     * @param array $arguments Additional binding arguments.
     *
     * @throws AMQPChannelException If the channel is not open.
     * @throws AMQPConnectionException If the connection to the broker was lost.
     * @throws AMQPExchangeException On failure.
     */
    public function unbind(string $exchangeName, ?string $routingKey = null, array $arguments = []): void
    {
    }

    /**
     * Declare a new exchange on the broker.
     *
     * @throws AMQPExchangeException On failure.
     * @throws AMQPChannelException If the channel is not open.
     * @throws AMQPConnectionException If the connection to the broker was lost.
     */
    public function declareExchange(): void
    {
    }

    /**
     * Declare a new exchange on the broker.
     *
     * @throws AMQPExchangeException On failure.
     * @throws AMQPChannelException If the channel is not open.
     * @throws AMQPConnectionException If the connection to the broker was lost.
     */
    public function declare(): void
    {
    }

    /**
     * Delete the exchange from the broker.
     *
     * @param string $exchangeName Optional name of exchange to delete. If not specified it uses the name of the
     *                             exchange object
     * @param integer $flags Optionally AMQP_IFUNUSED can be specified
     *                       to indicate the exchange should not be
     *                       deleted until no clients are connected to
     *                       it.
     *
     * @throws AMQPExchangeException On failure.
     * @throws AMQPChannelException If the channel is not open.
     * @throws AMQPConnectionException If the connection to the broker was lost.
     */
    public function delete(?string $exchangeName = null, ?int $flags = null): void
    {
    }

    /**
     * Get the argument associated with the given key.
     *
     * @param string $argumentName The key to look up.
     * @throws AMQPExchangeException If key does not exist
     * @return bool|int|double|string|null
     */
    public function getArgument(string $argumentName)
    {
    }

    /**
     * Check whether argument associated with the given key exists.
     *
     * @param string $argumentName The key to look up.
     *
     * @return boolean
     */
    public function hasArgument(string $argumentName): bool
    {
    }

    /**
     * Get all arguments set on the given exchange.
     *
     * @return array An array containing all the set key/value pairs.
     */
    public function getArguments(): array
    {
    }

    /**
     * Get all the flags currently set on the given exchange.
     *
     * @return int An integer bitmask of all the flags currently set on this
     *             exchange object.
     */
    public function getFlags(): int
    {
    }

    /**
     * Get the configured name.
     *
     * @return string|null The configured name as a string.
     */
    public function getName(): ?string
    {
    }

    /**
     * Get the configured type.
     *
     * @return string|null The configured type as a string.
     */
    public function getType(): ?string
    {
    }

    /**
     * Publish a message to an exchange.
     *
     * Publish a message to the exchange represented by the AMQPExchange object.
     *
     * @param string $message The message to publish.
     * @param string|null $routingKey The optional routing key to which to
     *                                publish to.
     * @param integer $flags One or more of AMQP_MANDATORY and
     *                       AMQP_IMMEDIATE.
     * @param array $headers One of content_type, content_encoding,
     *                       message_id, user_id, app_id, delivery_mode,
     *                       priority, timestamp, expiration, type
     *                       or reply_to, headers.
     * @throws AMQPChannelException If the channel is not open.
     * @throws AMQPConnectionException If the connection to the broker was lost.
     * @throws AMQPExchangeException On failure.
     */
    public function publish(
        string $message,
        ?string $routingKey = null,
        ?int $flags = null,
        array $headers = []
    ): void {
    }

    /**
     * Set the value for the given key.
     *
     * @param string $argumentName Name of the argument to set.
     * @param bool|int|double|string|null $argumentValue Value of the argument to set.
     */
    public function setArgument(string $argumentName, $argumentValue): void
    {
    }

    /**
     * Set the value for the given key.
     *
     * @param string $argumentName Name of the argument to remove.
     */
    public function removeArgument(string $argumentName): void
    {
    }

    /**
     * Set all arguments on the exchange.
     *
     * @param array $arguments An array of key/value pairs of arguments.
     */
    public function setArguments(array $arguments): void
    {
    }

    /**
     * Set the flags on an exchange.
     *
     * @param integer $flags A bitmask of flags. This call currently only
     *                       considers the following flags:
     *                       AMQP_DURABLE, AMQP_PASSIVE
     *                       (and AMQP_DURABLE, if librabbitmq version >= 0.5.3)
     */
    public function setFlags(?int $flags): void
    {
    }

    /**
     * Set the name of the exchange.
     *
     * @param string|null $exchangeName The name of the exchange to set as string.
     */
    public function setName(?string $exchangeName): void
    {
    }

    /**
     * Set the type of the exchange.
     *
     * Set the type of the exchange. This can be any of AMQP_EX_TYPE_DIRECT,
     * AMQP_EX_TYPE_FANOUT, AMQP_EX_TYPE_HEADERS or AMQP_EX_TYPE_TOPIC.
     *
     * @param string|null $exchangeType The type of exchange as a string.
     */
    public function setType(?string $exchangeType): void
    {
    }

    /**
     * Get the AMQPChannel object in use
     */
    public function getChannel(): AMQPChannel
    {
    }

    /**
     * Get the AMQPConnection object in use
     */
    public function getConnection(): AMQPConnection
    {
    }
}
