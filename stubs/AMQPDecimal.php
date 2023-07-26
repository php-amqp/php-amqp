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

    /**
     * @param int $exponent
     * @param int $significand
     *
     * @throws AMQPValueException
     */
    public function __construct($exponent, $significand)
    {
    }

    /** @return int */
    public function getExponent()
    {
    }

    /** @return int */
    public function getSignificand()
    {
    }
}
