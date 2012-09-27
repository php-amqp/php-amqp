<?php
/**
 * stub class representing AMQPExchange from pecl-amqp
 */
class AMQPExchange
{

    /**
     * Bind to another exchange.
     *
     * Bind an exchange to another exchange using the specified routing key.
     *
     * @param string $destination_exchange_name The name of the destination
     *                                          exchange in the binding.
     * @param string $source_exchange_name      The name of the source
     *                                          exchange in the binding.
     * @param string $routing_key               The routing key to use as a
     *                                          binding.
     *
     * @throws AMQPExchangeException   On failure.
     * @throws AMQPChannelException    If the channel is not open.
     * @throws AMQPConnectionException If the connection to the broker was lost.
     *
     * @return boolean true on success or false on failure.
     */
    public function bind($destination_exchange_name, $source_exchange_name, $routing_key)
    {
        return true;
    }

    /**
     * Create an instance of AMQPExchange.
     *
     * Returns a new instance of an AMQPExchange object, associated with the
     * given AMQPChannel object.
     *
     * @param AMQPChannel $amqp_channel A valid AMQPChannel object, connected
     *                                  to a broker.
     *
     * @throws AMQPExchangeException   When amqp_channel is not connected to
     *                                 a broker.
     * @throws AMQPConnectionException If the connection to the broker was
     *                                 lost.
     */
    public function __construct(AMQPChannel $amqp_channel)
    {
    }

    /**
     * Declare a new exchange on the broker.
     *
     * @throws AMQPExchangeException   On failure.
     * @throws AMQPChannelException    If the channel is not open.
     * @throws AMQPConnectionException If the connection to the broker was lost.
     *
     * @return boolean TRUE on success or FALSE on failure.
     */
    public function declareExchange()
    {
        return true;
    }

    /**
     * Delete the exchange from the broker.
     *
     * @param integer $flags Optionally AMQP_IFUNUSED can be specified to indicate
     *                       the exchange should not be deleted until no clients
     *                       are connected to it.
     *
     * @throws AMQPExchangeException   On failure.
     * @throws AMQPChannelException    If the channel is not open.
     * @throws AMQPConnectionException If the connection to the broker was lost.
     *
     * @return boolean true on success or false on failure.
     */
    public function delete ($flags = AMQP_NOPARAM)
    {
        return true;
    }

    /**
     * Get the argument associated with the given key.
     *
     * @param string $key The key to look up.
     *
     * @return string|integer|boolean The string or integer value associated
     *                                with the given key, or FALSE if the key
     *                                is not set.
     */
    public function getArgument ($key)
    {
        return '';
    }

    /**
     * Get all arguments set on the given exchange.
     *
     * @return array An array containing all of the set key/value pairs.
     */
    public function getArguments()
    {
        return array();
    }

    /**
     * Get all the flags currently set on the given exchange.
     *
     * @return int An integer bitmask of all the flags currently set on this
     *             exchange object.
     */
    public function getFlags()
    {
        return 0;
    }

    /**
     * Get the configured name.
     *
     * @return string The configured name as a string.
     */
    public function getName()
    {
        return '';
    }

    /**
     * Get the configured type.
     *
     * @return string The configured type as a string.
     */
    public function getType()
    {
        return '';
    }

    /**
     * Publish a message to an exchange.
     *
     * Publish a message to the exchange represented by the AMQPExchange object.
     *
     * @param string  $message     The message to publish.
     * @param string  $routing_key The routing key to which to publish.
     * @param integer $flags       One or more of AMQP_MANDATORY and
     *                             AMQP_IMMEDIATE.
     * @param array   $attributes  One of content_type, content_encoding,
     *                             message_id, user_id, app_id, delivery_mode,
     *                             priority, timestamp, expiration, type
     *                             or reply_to.
     *
     * @throws AMQPExchangeException   On failure.
     * @throws AMQPChannelException    If the channel is not open.
     * @throws AMQPConnectionException If the connection to the broker was lost.
     *
     * @return boolean TRUE on success or FALSE on failure.
     */
    public function publish ($message, $routing_key, $flags = AMQP_NOPARAM, array $attributes = array())
    {
        return true;
    }

    /**
     * Set the value for the given key.
     *
     * @param string         $key   Name of the argument to set.
     * @param string|integer $value Value of the argument to set.
     *
     * @return boolean TRUE on success or FALSE on failure.
     */
    public function setArgument($key, $value)
    {
        return true;
    }

    /**
     * Set all arguments on the exchange.
     *
     * @param array $arguments An array of key/value pairs of arguments.
     *
     * @return boolean TRUE on success or FALSE on failure.
     */
    public function setArguments(array $arguments)
    {
        return true;
    }

    /**
     * Set the flags on an exchange.
     *
     * @param integer $flags A bitmask of flags. This call currently only
     *                       considers the following flags:
     *                       AMQP_DURABLE, AMQP_PASSIVE.
     *
     * @return boolean True on success or false on failure.
     */
    public function setFlags($flags)
    {
        return true;
    }

    /**
     * Set the name of the exchange.
     *
     * @param string $exchange_name The name of the exchange to set as string.
     *
     * @return boolean TRUE on success or FALSE on failure.
     */
    public function setName($exchange_name)
    {
        return true;
    }

    /**
     * Set the type of the exchange.
     *
     * Set the type of the exchange. This can be any of AMQP_EX_TYPE_DIRECT,
     * AMQP_EX_TYPE_FANOUT, AMQP_EX_TYPE_HEADER or AMQP_EX_TYPE_TOPIC.
     *
     * @param string $exchange_type The type of exchange as a string.
     *
     * @return boolean TRUE on success or FALSE on failure.
     */
    public function setType($exchange_type)
    {
        return true;
    }
}

