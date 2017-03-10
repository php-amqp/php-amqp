<?php

/**
 * stub class representing AMQPDecimal from pecl-amqp
 */
final class AMQPDecimal
{
    const EXPONENT_MIN = 0;
    const EXPONENT_MAX = 255;
    const SIGNIFICAND_MIN = 0;
    const SIGNIFICAND_MAX = 4294967295;

    /**
     * @param $exponent
     * @param $significand
     *
     * @throws AMQPExchangeValue
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
