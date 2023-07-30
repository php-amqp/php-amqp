<?php

/**
 * stub class representing AMQPBasicProperties from pecl-amqp
 */
class AMQPBasicProperties
{
    private ?string $contentType = null;

    private ?string $contentEncoding = null;

    private array $headers = [];

    private int $deliveryMode = AMQP_DELIVERY_MODE_TRANSIENT;

    private int $priority = 0;

    private ?string $correlationId = null;

    private ?string $replyTo = null;

    private ?string $expiration = null;

    private ?string $messageId = null;

    private ?int $timestamp = null;

    private ?string $type = null;

    private ?string $userId = null;

    private ?string $appId = null;

    private ?string $clusterId = null;

    /**
     * @param ?string $contentType
     * @param ?string $contentEncoding
     * @param ?string $correlationId
     * @param ?string $replyTo
     * @param ?string $expiration
     * @param ?string $messageId
     * @param ?int $timestamp
     * @param ?string $type
     * @param ?string $userId
     * @param ?string $appId
     * @param ?string $clusterId
     */
    public function __construct(
        ?string $contentType = null,
        ?string $contentEncoding = null,
        array $headers = [],
        int $deliveryMode = AMQP_DELIVERY_MODE_TRANSIENT,
        int $priority = 0,
        ?string $correlationId = null,
        ?string $replyTo = null,
        ?string $expiration = null,
        ?string $messageId = null,
        ?int $timestamp = null,
        ?string $type = null,
        ?string $userId = null,
        ?string $appId = null,
        ?string $clusterId = null
    ) {
    }

    /**
     * Get the message content type.
     *
     * @return string|null The content type of the message.
     */
    public function getContentType(): ?string
    {
    }

    /**
     * Get the content encoding of the message.
     *
     * @return string|null The content encoding of the message.
     */
    public function getContentEncoding(): ?string
    {
    }

    /**
     * Get the headers of the message.
     *
     * @return array An array of key value pairs associated with the message.
     */
    public function getHeaders(): array
    {
    }

    /**
     * Get the delivery mode of the message.
     *
     * @return int The delivery mode of the message.
     */
    public function getDeliveryMode(): int
    {
    }

    /**
     * Get the priority of the message.
     *
     * @return int The message priority.
     */
    public function getPriority(): int
    {
    }

    /**
     * Get the message correlation id.
     *
     * @return string|null The correlation id of the message.
     */
    public function getCorrelationId(): ?string
    {
    }

    /**
     * Get the reply-to address of the message.
     *
     * @return string|null The contents of the reply to field.
     */
    public function getReplyTo(): ?string
    {
    }

    /**
     * Get the expiration of the message.
     *
     * @return string|null The message expiration.
     */
    public function getExpiration(): ?string
    {
    }

    /**
     * Get the message id of the message.
     *
     * @return string|null The message id
     */
    public function getMessageId(): ?string
    {
    }

    /**
     * Get the timestamp of the message.
     *
     * @return int|null The message timestamp.
     */
    public function getTimestamp(): ?int
    {
    }

    /**
     * Get the message type.
     *
     * @return string|null The message type.
     */
    public function getType(): ?string
    {
    }

    /**
     * Get the message user id.
     *
     * @return string|null The message user id.
     */
    public function getUserId(): ?string
    {
    }

    /**
     * Get the application id of the message.
     *
     * @return string|null The application id of the message.
     */
    public function getAppId(): ?string
    {
    }

    /**
     * Get the cluster id of the message.
     *
     * @return string|null The cluster id of the message.
     */
    public function getClusterId(): ?string
    {
    }
}
