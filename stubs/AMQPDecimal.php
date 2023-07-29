<?php

/**
 * stub class representing AMQPDecimal from pecl-amqp
 */
final class AMQPDecimal
{
    /**
     * @var int
     */
    const EXPONENT_MIN = 0;

    /**
     * @var int
     */
    const EXPONENT_MAX = 255;

    /**
     * @var int
     */
    const SIGNIFICAND_MIN = 0;

    /**
     * @var int
     */
    const SIGNIFICAND_MAX = 4294967295;

    private int $exponent;
    private int $significand;

    /**
     * @param int $exponent
     * @param int $significand
     *
     * @throws AMQPValueException
     */
    public function __construct(int $exponent, int $significand)
    {
    }

    /** @return int */
    public function getExponent(): int
    {
    }

    /** @return int */
    public function getSignificand(): int
    {
    }
}
