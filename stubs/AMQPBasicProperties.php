<?php

/**
 * stub class representing AMQPBasicProperties from pecl-amqp
 */
class AMQPBasicProperties
{
    /**
     * @param string $contentType
     * @param string $contentEncoding
     * @param array  $headers
     * @param int    $deliveryMode
     * @param int    $priority
     * @param string $correlationId
     * @param string $replyTo
     * @param string $expiration
     * @param string $messageId
     * @param int    $timestamp
     * @param string $type
     * @param string $userId
     * @param string $appId
     * @param string $clusterId
     */
    public function __construct(
        string $contentType = "",
        string $contentEncoding = "",
        array $headers = [],
        int $deliveryMode = AMQP_DELIVERY_MODE_TRANSIENT,
        int $priority = 0,
        string $correlationId = "",
        string $replyTo = "",
        string $expiration = "",
        string $messageId = "",
        int $timestamp = 0,
        string $type = "",
        string $userId = "",
        string $appId = "",
        string $clusterId = ""
    ) {
    }

    /**
     * Get the message content type.
     *
     * @return string The content type of the message.
     */
    public function getContentType(): string
    {
    }

    /**
     * Get the content encoding of the message.
     *
     * @return string The content encoding of the message.
     */
    public function getContentEncoding(): string
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
     * @return string The correlation id of the message.
     */
    public function getCorrelationId(): string
    {
    }

    /**
     * Get the reply-to address of the message.
     *
     * @return string The contents of the reply to field.
     */
    public function getReplyTo(): string
    {
    }

    /**
     * Get the expiration of the message.
     *
     * @return string The message expiration.
     */
    public function getExpiration(): string
    {
    }

    /**
     * Get the message id of the message.
     *
     * @return string The message id
     */
    public function getMessageId(): string
    {
    }

    /**
     * Get the timestamp of the message.
     *
     * @return int The message timestamp.
     */
    public function getTimestamp(): int
    {
    }

    /**
     * Get the message type.
     *
     * @return string The message type.
     */
    public function getType(): string
    {
    }

    /**
     * Get the message user id.
     *
     * @return string The message user id.
     */
    public function getUserId(): string
    {
    }

    /**
     * Get the application id of the message.
     *
     * @return string The application id of the message.
     */
    public function getAppId(): string
    {
    }

    /**
     * Get the cluster id of the message.
     *
     * @return string The cluster id of the message.
     */
    public function getClusterId(): string
    {
    }
}
