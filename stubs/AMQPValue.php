<?php
/**
 * Interface representing AMQP values
 */
interface AMQPValue
{
    /**
     * @return bool|int|double|string|null|AMQPValue|AMQPDecimal|AMQPTimestamp
     */
    public function toAmqpValue();
}
