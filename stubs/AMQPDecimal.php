<?php

/**
 * stub class representing AMQPDecimal from pecl-amqp
 *
 * @readonly
 */
final /* readonly */ class AMQPDecimal implements AMQPValue
{
    /**
     * @var int
     */
    public const EXPONENT_MIN = 0;

    /**
     * @var int
     */
    public const EXPONENT_MAX = 255;

    /**
     * @var int
     */
    public const SIGNIFICAND_MIN = 0;

    /**
     * @var int
     */
    public const SIGNIFICAND_MAX = 4294967295;

    private int $exponent;

    private int $significand;

    /**
     * @throws AMQPValueException
     */
    public function __construct(int $exponent, int $significand)
    {
    }

    public function getExponent(): int
    {
    }

    public function getSignificand(): int
    {
    }

    public function toAmqpValue()
    {
    }
}
