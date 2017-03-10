<?php

/**
 * stub class representing AMQPTimestamp from pecl-amqp
 */
final class AMQPTimestamp
{
    const MIN = "0";
    const MAX = "18446744073709551616";

    /**
     * @param string $timestamp
     *
     * @throws AMQPExchangeValue
     */
    public function __construct($timestamp)
    {
    }

    /** @return string */
    public function getTimestamp()
    {
    }

    /** @return string */
    public function __toString()
    {
    }
}
