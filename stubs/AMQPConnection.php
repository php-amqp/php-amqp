<?php

/**
 * stub class representing AMQPConnection from pecl-amqp
 */
class AMQPConnection
{

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
     *
     *      'connection_name' => A user determined name for the connection
     * )
     *
     * @param array $credentials Optional array of credential information for
     *                           connecting to the AMQP broker.
     */
    public function __construct(array $credentials = [])
    {
    }

    /**
     * Check whether the connection to the AMQP broker is still valid.
     *
     * It does so by checking the return status of the last connect-command.
     *
     * @return boolean True if connected, false otherwise.
     */
    public function isConnected(): bool
    {
    }

    /**
     * Whether connection persistent.
     *
     * When no connection is established, it will always return false
     *
     * @return boolean
     */
    public function isPersistent(): bool
    {
    }

    /**
     * Establish a transient connection with the AMQP broker.
     *
     * This method will initiate a connection with the AMQP broker.
     *
     * @throws AMQPConnectionException
     * @return void
     */
    public function connect(): void
    {
    }

    /**
     * Closes the transient connection with the AMQP broker.
     *
     * This method will close an open connection with the AMQP broker.
     *
     * @throws AMQPConnectionException When attempting to disconnect a persistent connection
     * @return void
     */
    public function disconnect(): void
    {
    }

    /**
     * Close any open transient connections and initiate a new one with the AMQP broker.
     *
     * @throws AMQPConnectionException
     * @return void
     */
    public function reconnect(): void
    {
    }

    /**
     * Establish a persistent connection with the AMQP broker.
     *
     * This method will initiate a connection with the AMQP broker
     * or reuse an existing one if present.
     *
     * @throws AMQPConnectionException
     * @return void
     */
    public function pconnect(): void
    {
    }

    /**
     * Closes a persistent connection with the AMQP broker.
     *
     * This method will close an open persistent connection with the AMQP
     * broker.
     *
     * @throws AMQPConnectionException When attempting to disconnect a transient connection
     * @return void
     */
    public function pdisconnect(): void
    {
    }

    /**
     * Close any open persistent connections and initiate a new one with the AMQP broker.
     *
     * @throws AMQPConnectionException
     * @return void
     */
    public function preconnect(): void
    {
    }

    /**
     * Get the configured host.
     *
     * @return string The configured hostname of the broker
     */
    public function getHost(): string
    {
    }

    /**
     * Get the configured login.
     *
     * @return string The configured login as a string.
     */
    public function getLogin(): string
    {
    }

    /**
     * Get the configured password.
     *
     * @return string The configured password as a string.
     */
    public function getPassword(): string
    {
    }

    /**
     * Get the configured port.
     *
     * @return int The configured port as an integer.
     */
    public function getPort(): int
    {
    }

    /**
     * Get the configured vhost.
     *
     * @return string The configured virtual host as a string.
     */
    public function getVhost(): string
    {
    }


    /**
     * Set the hostname used to connect to the AMQP broker.
     *
     * @param string $host The hostname of the AMQP broker.
     *
     * @throws AMQPConnectionException If host is longer then 1024 characters.
     *
     * @return void
     */
    public function setHost(string $host): void
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
     * @return void
     */
    public function setLogin(string $login): void
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
     * @return void
     */
    public function setPassword(string $password): void
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
     * @return void
     */
    public function setPort(int $port): void
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
     * @return void
     */
    public function setVhost(string $vhost): void
    {
    }

    /**
     * Sets the interval of time to wait for income activity from AMQP broker
     *
     * @deprecated use AMQPConnection::setReadTimeout($timeout) instead
     *
     * @param float $timeout
     *
     * @throws AMQPConnectionException If timeout is less than 0.
     *
     * @return void
     */
    public function setTimeout(float $timeout): void
    {
    }

    /**
     * Get the configured interval of time to wait for income activity
     * from AMQP broker
     *
     * @deprecated use AMQPConnection::getReadTimeout() instead
     *
     * @return float
     */
    public function getTimeout(): float
    {
    }

    /**
     * Sets the interval of time (in seconds) to wait for income activity from AMQP broker
     *
     * @param float $timeout
     *
     * @throws AMQPConnectionException If timeout is less than 0.
     *
     * @return void
     */
    public function setReadTimeout(float $timeout): void
    {
    }

    /**
     * Get the configured interval of time (in seconds) to wait for income activity
     * from AMQP broker
     *
     * @return float
     */
    public function getReadTimeout(): float
    {
    }

    /**
     * Sets the interval of time (in seconds) to wait for outcome activity to AMQP broker
     *
     * @param float $timeout
     *
     * @throws AMQPConnectionException If timeout is less than 0.
     *
     * @return void
     */
    public function setWriteTimeout(float $timeout): void
    {
    }

    /**
     * Get the configured interval of time (in seconds) to wait for outcome activity
     * to AMQP broker
     *
     * @return float
     */
    public function getWriteTimeout(): float
    {
    }

    /**
     * Get the configured timeout (in seconds) for connecting to the AMQP broker
     *
     * @return float
     */
    public function getConnectTimeout(): float
    {
    }

    /**
     * Sets the interval of time to wait (in seconds) for RPC activity to AMQP broker
     *
     * @param float $timeout
     *
     * @throws AMQPConnectionException If timeout is less than 0.
     *
     * @return void
     */
    public function setRpcTimeout(float $timeout): void
    {
    }

    /**
     * Get the configured interval of time (in seconds) to wait for RPC activity
     * to AMQP broker
     *
     * @return float
     */
    public function getRpcTimeout(): float
    {
    }

    /**
     * Return last used channel id during current connection session.
     *
     * @return int
     */
    public function getUsedChannels(): int
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
    public function getMaxChannels(): int
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
    public function getMaxFrameSize(): int
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
    public function getHeartbeatInterval(): int
    {
    }

    /**
     * Get path to the CA cert file in PEM format
     *
     * @return string
     */
    public function getCACert(): string
    {
    }

    /**
     * Set path to the CA cert file in PEM format
     *
     * @param string $cacert
     * @return void
     */
    public function setCACert(string $cacert): void
    {
    }

    /**
     * Get path to the client certificate in PEM format
     *
     * @return string
     */
    public function getCert(): string
    {
    }

    /**
     * Set path to the client certificate in PEM format
     *
     * @param string $cert
     * @return void
     */
    public function setCert(string $cert): void
    {
    }

    /**
     * Get path to the client key in PEM format
     *
     * @return string
     */
    public function getKey(): string
    {
    }

    /**
     * Set path to the client key in PEM format
     *
     * @param string $key
     * @return void
     */
    public function setKey(string $key): void
    {
    }

    /**
     * Get whether peer verification enabled or disabled
     *
     * @return bool
     */
    public function getVerify(): bool
    {
    }

    /**
     * Enable or disable peer verification
     *
     * @param bool $verify
     * @return void
     */
    public function setVerify(bool $verify): void
    {
    }

    /**
     * set authentication method
     *
     * @param int $saslMethod AMQP_SASL_METHOD_PLAIN | AMQP_SASL_METHOD_EXTERNAL
     * @return void
     */
    public function setSaslMethod(int $saslMethod): void
    {
    }

    /**
     * @return int
     */
    public function getSaslMethod(): int
    {
    }

    /**
     * @param string|null $connectionName
     * @return void
     */
    public function setConnectionName(?string $connectionName): void
    {
    }

    /**
     * @return string|null
     */
    public function getConnectionName(): ?string
    {
    }
}
