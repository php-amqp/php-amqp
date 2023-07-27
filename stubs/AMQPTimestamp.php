<?php

/**
 * stub class representing AMQPTimestamp from pecl-amqp
 */
final class AMQPTimestamp
{
    /**
     * @var string
     */
    const MIN = "0";

    /**
     * @var string
     */
    const MAX = "18446744073709551616";

    /**
     * @param string $timestamp
     *
     * @throws AMQPValueException
     */
    public function __construct(string $timestamp)
    {
    }

    public function getTimestamp(): string
    {
    }

    public function __toString(): string
    {
    }
}
