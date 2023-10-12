<?php

/**
 * stub class representing AMQPChannel from pecl-amqp
 */
class AMQPChannel
{
    private AMQPConnection $connection;

    private ?int $prefetchCount = null;

    private ?int $prefetchSize;

    private ?int $globalPrefetchCount;

    private ?int $globalPrefetchSize;

    private array $consumers = [];

    /**
     * Create an instance of an AMQPChannel object.
     *
     * @param AMQPConnection $connection An instance of AMQPConnection
     *                                   with an active connection to a
     *                                   broker.
     *
     * @throws AMQPConnectionException If the connection to the broker
     *                                 was lost.
     */
    public function __construct(AMQPConnection $connection)
    {
    }

    /**
     * Commit a pending transaction.
     *
     * @throws AMQPChannelException If no transaction was started prior to
     *                              calling this method.
     * @throws AMQPConnectionException If the connection to the broker was lost.
     */
    public function commitTransaction(): void
    {
    }

    /**
     * Check the channel connection.
     *
     * @return bool Indicates whether the channel is connected.
     */
    public function isConnected(): bool
    {
    }

    /**
     * Closes the channel.
     */
    public function close(): void
    {
    }

    /**
     * Return internal channel ID
     *
     * @return integer
     */
    public function getChannelId(): int
    {
    }

    /**
     * Set the Quality Of Service settings for the given channel.
     *
     * Specify the amount of data to prefetch in terms of window size (octets)
     * or number of messages from a queue during a AMQPQueue::consume() or
     * AMQPQueue::get() method call. The client will prefetch data up to size
     * octets or count messages from the server, whichever limit is hit first.
     * Setting either value to 0 will instruct the client to ignore that
     * particular setting. A call to AMQPChannel::qos() will overwrite any
     * values set by calling AMQPChannel::setPrefetchSize() and
     * AMQPChannel::setPrefetchCount(). If the call to either
     * AMQPQueue::consume() or AMQPQueue::get() is done with the AMQP_AUTOACK
     * flag set, the client will not do any prefetching of data, regardless of
     * the QOS settings.
     *
     * @param integer $size The window size, in octets, to prefetch.
     * @param integer $count The number of messages to prefetch.
     * @param bool $global TRUE for global, FALSE for consumer. FALSE by default.
     *
     * @throws AMQPConnectionException If the connection to the broker was lost.
     */
    public function qos(int $size, int $count, bool $global = false): void
    {
    }

    /**
     * Rollback a transaction.
     *
     * Rollback an existing transaction. AMQPChannel::startTransaction() must
     * be called prior to this.
     *
     * @throws AMQPChannelException If no transaction was started prior to
     *                              calling this method.
     * @throws AMQPConnectionException If the connection to the broker was lost.
     */
    public function rollbackTransaction(): void
    {
    }

    /**
     * Set the number of messages to prefetch from the broker for each consumer.
     *
     * Set the number of messages to prefetch from the broker during a call to
     * AMQPQueue::consume() or AMQPQueue::get().
     *
     * @param integer $count The number of messages to prefetch.
     *
     * @throws AMQPConnectionException If the connection to the broker was lost.
     */
    public function setPrefetchCount(int $count): void
    {
    }

    /**
     * Get the number of messages to prefetch from the broker for each consumer.
     *
     * @return integer
     */
    public function getPrefetchCount(): int
    {
    }

    /**
     * Set the window size to prefetch from the broker for each consumer.
     *
     * Set the prefetch window size, in octets, during a call to
     * AMQPQueue::consume() or AMQPQueue::get(). Any call to this method will
     * automatically set the prefetch message count to 0, meaning that the
     * prefetch message count setting will be ignored. If the call to either
     * AMQPQueue::consume() or AMQPQueue::get() is done with the AMQP_AUTOACK
     * flag set, this setting will be ignored.
     *
     * @param integer $size The window size, in octets, to prefetch.
     *
     * @throws AMQPConnectionException If the connection to the broker was lost.
     */
    public function setPrefetchSize(int $size): void
    {
    }

    /**
     * Get the window size to prefetch from the broker for each consumer.
     *
     * @return integer
     */
    public function getPrefetchSize(): int
    {
    }

    /**
     * Set the number of messages to prefetch from the broker across all consumers.
     *
     * Set the number of messages to prefetch from the broker during a call to
     * AMQPQueue::consume() or AMQPQueue::get().
     *
     * @param integer $count The number of messages to prefetch.
     *
     * @throws AMQPConnectionException If the connection to the broker was lost.
     */
    public function setGlobalPrefetchCount(int $count): void
    {
    }

    /**
     * Get the number of messages to prefetch from the broker across all consumers.
     *
     * @return integer
     */
    public function getGlobalPrefetchCount(): int
    {
    }

    /**
     * Set the window size to prefetch from the broker for all consumers.
     *
     * Set the prefetch window size, in octets, during a call to
     * AMQPQueue::consume() or AMQPQueue::get(). Any call to this method will
     * automatically set the prefetch message count to 0, meaning that the
     * prefetch message count setting will be ignored. If the call to either
     * AMQPQueue::consume() or AMQPQueue::get() is done with the AMQP_AUTOACK
     * flag set, this setting will be ignored.
     *
     * @param integer $size The window size, in octets, to prefetch.
     *
     * @throws AMQPConnectionException If the connection to the broker was lost.
     */
    public function setGlobalPrefetchSize(int $size): void
    {
    }

    /**
     * Get the window size to prefetch from the broker for all consumers.
     *
     * @return integer
     */
    public function getGlobalPrefetchSize(): int
    {
    }

    /**
     * Start a transaction.
     *
     * This method must be called on the given channel prior to calling
     * AMQPChannel::commitTransaction() or AMQPChannel::rollbackTransaction().
     *
     * @throws AMQPConnectionException If the connection to the broker was lost.
     */
    public function startTransaction(): void
    {
    }

    /**
     * Get the AMQPConnection object in use
     */
    public function getConnection(): AMQPConnection
    {
    }

    /**
     * Redeliver unacknowledged messages.
     */
    public function basicRecover(bool $requeue = true): void
    {
    }

    /**
     * Set the channel to use publisher acknowledgements. This can only used on a non-transactional channel.
     */
    public function confirmSelect(): void
    {
    }

    /**
     * Set callback to process basic.ack and basic.nac AMQP server methods (applicable when channel in confirm mode).
     *
     * Callback functions with all arguments have the following signature:
     *
     *      function ack_callback(int $delivery_tag, bool $multiple) : bool;
     *      function nack_callback(int $delivery_tag, bool $multiple, bool $requeue) : bool;
     *
     * and should return boolean FALSE when wait loop should be canceled.
     *
     * Note, basic.nack server method will only be delivered if an internal error occurs in the Erlang process
     * responsible for a queue (see https://www.rabbitmq.com/confirms.html for details).
     */
    public function setConfirmCallback(?callable $ackCallback, callable $nackCallback = null): void
    {
    }

    /**
     * Wait until all messages published since the last call have been either ack'd or nack'd by the broker.
     *
     * Note, this method also catch all basic.return message from server.
     *
     * @param float $timeout Timeout in seconds. May be fractional.
     *
     * @throws AMQPQueueException If timeout occurs.
     */
    public function waitForConfirm(float $timeout = 0.0): void
    {
    }

    /**
     * Set callback to process basic.return AMQP server method
     *
     * Callback function with all arguments has the following signature:
     *
     *      function callback(int $reply_code,
     *                        string $reply_text,
     *                        string $exchange,
     *                        string $routing_key,
     *                        AMQPBasicProperties $properties,
     *                        string $body) : bool;
     *
     * and should return boolean FALSE when wait loop should be canceled.
     */
    public function setReturnCallback(?callable $returnCallback): void
    {
    }

    /**
     * Start wait loop for basic.return AMQP server methods
     *
     * @param float $timeout Timeout in seconds. May be fractional.
     *
     * @throws AMQPQueueException If timeout occurs.
     */
    public function waitForBasicReturn(float $timeout = 0.0): void
    {
    }

    /**
     * Return array of current consumers where key is consumer and value is AMQPQueue consumer is running on
     *
     * @return AMQPQueue[]
     */
    public function getConsumers(): array
    {
    }
}
