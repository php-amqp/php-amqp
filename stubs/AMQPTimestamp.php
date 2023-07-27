<?php

/**
 * stub class representing AMQPTimestamp from pecl-amqp
 */
final class AMQPTimestamp
{
    /**
     * @var float
     */
    const MIN = 0.0;

    /**
     * @var float
     */
    const MAX = 18446744073709551616;

    private $timestamp;

    /**
     * @param float $timestamp
     *
     * @throws AMQPValueException
     */
    public function __construct(float $timestamp)
    {
    }

    public function getTimestamp(): float
    {
    }

    public function __toString(): string
    {
    }
}
