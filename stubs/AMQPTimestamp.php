<?php

/**
 * stub class representing AMQPTimestamp from pecl-amqp
 *
 * @readonly
 */
final /* readonly */ class AMQPTimestamp implements AMQPValue
{
    /**
     * @var float
     */
    public const MIN = 0.0;

    /**
     * @var float
     */
    public const MAX = 18446744073709551616;

    private float $timestamp;

    /**
     * @throws AMQPValueException
     */
    public function __construct(float $timestamp)
    {
    }

    public function __toString(): string
    {
    }

    public function getTimestamp(): float
    {
    }

    public function toAmqpValue()
    {
    }
}
