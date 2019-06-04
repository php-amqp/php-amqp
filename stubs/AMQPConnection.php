<?php

/**
 * stub class representing AMQPConnection from pecl-amqp
 */
class AMQPConnection
{
    /**
     * Establish a transient connection with the AMQP broker.
     *
     * This method will initiate a connection with the AMQP broker.
     *
     * @throws AMQPConnectionException
     * @return boolean TRUE on success or throw an exception on failure.
     */
    public function connect()
    {
    }

    /**
     * Create an instance of AMQPConnection.
     *
     * Creates an AMQPConnection instance representing a connection to an AMQP
     * broker. A connection will not be established until
     * AMQPConnection::connect() is called.
     *
     *  $credentials = array(
     *      'host'  => amqp.host The host to connect too. Note: Max 1024 characters.
     *      'port'  => amqp.port Port on the host.
     *      'vhost' => amqp.vhost The virtual host on the host. Note: Max 128 characters.
     *      'login' => amqp.login The login name to use. Note: Max 128 characters.
     *      'password' => amqp.password Password. Note: Max 128 characters.
     *      'read_timeout'  => Timeout in for income activity. Note: 0 or greater seconds. May be fractional.
     *      'write_timeout' => Timeout in for outcome activity. Note: 0 or greater seconds. May be fractional.
     *      'connect_timeout' => Connection timeout. Note: 0 or greater seconds. May be fractional.
     *      'rpc_timeout' => RPC timeout. Note: 0 or greater seconds. May be fractional.
     *
     *      Connection tuning options (see http://www.rabbitmq.com/amqp-0-9-1-reference.html#connection.tune for details):
     *      'channel_max' => Specifies highest channel number that the server permits. 0 means standard extension limit
     *                       (see PHP_AMQP_MAX_CHANNELS constant)
     *      'frame_max'   => The largest frame size that the server proposes for the connection, including frame header
     *                       and end-byte. 0 means standard extension limit (depends on librabbimq default frame size limit)
     *      'heartbeat'   => The delay, in seconds, of the connection heartbeat that the server wants.
     *                       0 means the server does not want a heartbeat. Note, librabbitmq has limited heartbeat support,
     *                       which means heartbeats checked only during blocking calls.
     *
     *      TLS support (see https://www.rabbitmq.com/ssl.html for details):
     *      'cacert' => Path to the CA cert file in PEM format..
     *      'cert'   => Path to the client certificate in PEM foramt.
     *      'key'    => Path to the client key in PEM format.
     *      'verify' => Enable or disable peer verification. If peer verification is enabled then the common name in the
     *                  server certificate must match the server name. Peer verification is enabled by default.
     * )
     *
     * @param array $credentials Optional array of credential information for
     *                           connecting to the AMQP broker.
     */
    public function __construct(array $credentials = array())
    {
    }

    /**
     * Closes the transient connection with the AMQP broker.
     *
     * This method will close an open connection with the AMQP broker.
     *
     * @return boolean true if connection was successfully closed, false otherwise.
     */
    public function disconnect()
    {
    }

    /**
     * Get the configured host.
     *
     * @return string The configured hostname of the broker
     */
    public function getHost()
    {
    }

    /**
     * Get the configured login.
     *
     * @return string The configured login as a string.
     */
    public function getLogin()
    {
    }

    /**
     * Get the configured password.
     *
     * @return string The configured password as a string.
     */
    public function getPassword()
    {
    }

    /**
     * Get the configured port.
     *
     * @return int The configured port as an integer.
     */
    public function getPort()
    {
    }

    /**
     * Get the configured vhost.
     *
     * @return string The configured virtual host as a string.
     */
    public function getVhost()
    {
    }

    /**
     * Check whether the connection to the AMQP broker is still valid.
     *
     * It does so by checking the return status of the last connect-command.
     *
     * @return boolean True if connected, false otherwise.
     */
    public function isConnected()
    {
    }

    /**
     * Establish a persistent connection with the AMQP broker.
     *
     * This method will initiate a connection with the AMQP broker
     * or reuse an existing one if present.
     *
     * @throws AMQPConnectionException
     * @return boolean TRUE on success or throws an exception on failure.
     */
    public function pconnect()
    {
    }

    /**
     * Closes a persistent connection with the AMQP broker.
     *
     * This method will close an open persistent connection with the AMQP
     * broker.
     *
     * @return boolean true if connection was found and closed,
     *                 false if no persistent connection with this host,
     *                 port, vhost and login could be found,
     */
    public function pdisconnect()
    {
    }

    /**
     * Close any open transient connections and initiate a new one with the AMQP broker.
     *
     * @return boolean TRUE on success or FALSE on failure.
     */
    public function reconnect()
    {
    }

    /**
     * Close any open persistent connections and initiate a new one with the AMQP broker.
     *
     * @return boolean TRUE on success or FALSE on failure.
     */
    public function preconnect()
    {
    }


    /**
     * Set the hostname used to connect to the AMQP broker.
     *
     * @param string $host The hostname of the AMQP broker.
     *
     * @throws AMQPConnectionException If host is longer then 1024 characters.
     *
     * @return boolean TRUE on success or FALSE on failure.
     */
    public function setHost($host)
    {
    }

    /**
     * Set the login string used to connect to the AMQP broker.
     *
     * @param string $login The login string used to authenticate
     *                      with the AMQP broker.
     *
     * @throws AMQPConnectionException If login is longer then 32 characters.
     *
     * @return boolean TRUE on success or FALSE on failure.
     */
    public function setLogin($login)
    {
    }

    /**
     * Set the password string used to connect to the AMQP broker.
     *
     * @param string $password The password string used to authenticate
     *                         with the AMQP broker.
     *
     * @throws AMQPConnectionException If password is longer then 32characters.
     *
     * @return boolean TRUE on success or FALSE on failure.
     */
    public function setPassword($password)
    {
    }

    /**
     * Set the port used to connect to the AMQP broker.
     *
     * @param integer $port The port used to connect to the AMQP broker.
     *
     * @throws AMQPConnectionException If port is longer not between
     *                                 1 and 65535.
     *
     * @return boolean TRUE on success or FALSE on failure.
     */
    public function setPort($port)
    {
    }

    /**
     * Sets the virtual host to which to connect on the AMQP broker.
     *
     * @param string $vhost The virtual host to use on the AMQP
     *                      broker.
     *
     * @throws AMQPConnectionException If host is longer then 32 characters.
     *
     * @return boolean true on success or false on failure.
     */
    public function setVhost($vhost)
    {
    }

    /**
     * Sets the interval of time to wait for income activity from AMQP broker
     *
     * @deprecated use AMQPConnection::setReadTimout($timeout) instead
     *
     * @param int $timeout
     *
     * @return bool
     */
    public function setTimeout($timeout)
    {
    }

    /**
     * Get the configured interval of time to wait for income activity
     * from AMQP broker
     *
     * @deprecated use AMQPConnection::getReadTimout() instead
     *
     * @return float
     */
    public function getTimeout()
    {
    }

    /**
     * Sets the interval of time to wait for income activity from AMQP broker
     *
     * @param int $timeout
     *
     * @return bool
     */
    public function setReadTimeout($timeout)
    {
    }

    /**
     * Get the configured interval of time to wait for income activity
     * from AMQP broker
     *
     * @return float
     */
    public function getReadTimeout()
    {
    }

    /**
     * Sets the interval of time to wait for outcome activity to AMQP broker
     *
     * @param int $timeout
     *
     * @return bool
     */
    public function setWriteTimeout($timeout)
    {
    }

    /**
     * Get the configured interval of time to wait for outcome activity
     * to AMQP broker
     *
     * @return float
     */
    public function getWriteTimeout()
    {
    }

    /**
     * Sets the interval of time to wait for RPC activity to AMQP broker
     *
     * @param int $timeout
     *
     * @return bool
     */
    public function setRpcTimeout($timeout)
    {
    }

    /**
     * Get the configured interval of time to wait for RPC activity
     * to AMQP broker
     *
     * @return float
     */
    public function getRpcTimeout()
    {
    }

    /**
     * Return last used channel id during current connection session.
     *
     * @return int
     */
    public function getUsedChannels()
    {
    }

    /**
     * Get the maximum number of channels the connection can handle.
     *
     * When connection is connected, effective connection value returned, which is normally the same as original
     * correspondent value passed to constructor, otherwise original value passed to constructor returned.
     *
     * @return int
     */
    public function getMaxChannels()
    {
    }

    /**
     * Get max supported frame size per connection in bytes.
     *
     * When connection is connected, effective connection value returned, which is normally the same as original
     * correspondent value passed to constructor, otherwise original value passed to constructor returned.
     *
     * @return int
     */
    public function getMaxFrameSize()
    {
    }

    /**
     * Get number of seconds between heartbeats of the connection in seconds.
     *
     * When connection is connected, effective connection value returned, which is normally the same as original
     * correspondent value passed to constructor, otherwise original value passed to constructor returned.
     *
     * @return int
     */
    public function getHeartbeatInterval()
    {
    }

    /**
     * Whether connection persistent.
     *
     * When connection is not connected, boolean false always returned
     *
     * @return bool
     */
    public function isPersistent()
    {
    }

    /**
     * Get path to the CA cert file in PEM format
     *
     * @return string
     */
    public function getCACert()
    {
    }

    /**
     * Set path to the CA cert file in PEM format
     *
     * @param string $cacert
     */
    public function setCACert($cacert)
    {
    }

    /**
     * Get path to the client certificate in PEM format
     *
     * @return string
     */
    public function getCert()
    {
    }

    /**
     * Set path to the client certificate in PEM format
     *
     * @param string $cert
     */
    public function setCert($cert)
    {
    }

    /**
     * Get path to the client key in PEM format
     *
     * @return string
     */
    public function getKey()
    {
    }

    /**
     * Set path to the client key in PEM format
     *
     * @param string $key
     */
    public function setKey($key)
    {
    }

    /**
     * Get whether peer verification enabled or disabled
     *
     * @return bool
     */
    public function getVerify()
    {
    }

    /**
     * Enable or disable peer verification
     *
     * @param bool $verify
     */
    public function setVerify($verify)
    {
    }

    /**
     * set authentication method
     *
     * @param int $method AMQP_SASL_METHOD_PLAIN | AMQP_SASL_METHOD_EXTERNAL
     */
    public function setSaslMethod($method)
    {
    }

    /**
     * @return int
     */
    public function getSaslMethod()
    {
    }
}
