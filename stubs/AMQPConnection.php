<?php

/**
 * stub class representing AMQPConnection from pecl-amqp
 */
class AMQPConnection
{
    private string $login;

    private string $password;

    private string $host;

    private string $vhost;

    private int $port;

    private float $readTimeout;

    private float $writeTimeout;

    private float $connectTimeout;

    private float $rpcTimeout;

    private int $channelMax;

    private int $frameMax;

    private int $heartbeat;

    private ?string $cacert;

    private ?string $key;

    private ?string $cert;

    private bool $verify = true;

    private int $saslMethod = AMQP_SASL_METHOD_PLAIN;

    private ?string $connectionName;

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
     *      'read_timeout'  => Timeout in for consume. Note: 0 or greater seconds. May be fractional.
     *      'write_timeout' => Timeout in for publish. Note: 0 or greater seconds. May be fractional.
     *      'connect_timeout' => Connection timeout. Note: 0 or greater seconds. May be fractional.
     *      'rpc_timeout' => Timeout for RPC-style AMQP methods. Note: 0 or greater seconds. May be fractional.
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
     * Cannot reliably detect dropped connections or unusual socket errors, as it does not actively
     * engage the socket.
     *
     * @return boolean TRUE if connected, FALSE otherwise.
     */
    public function isConnected(): bool
    {
    }

    /**
     * Whether connection persistent.
     *
     * When no connection is established, it will always return FALSE. The same disclaimer as for
     * {@see AMQPConnection::isConnected()} applies.
     *
     * @return boolean TRUE if persistently connected, FALSE otherwise.
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
     */
    public function disconnect(): void
    {
    }

    /**
     * Close any open transient connections and initiate a new one with the AMQP broker.
     *
     * @throws AMQPConnectionException
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
     */
    public function pdisconnect(): void
    {
    }

    /**
     * Close any open persistent connections and initiate a new one with the AMQP broker.
     *
     * @throws AMQPConnectionException
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
     */
    public function setVhost(string $vhost): void
    {
    }

    /**
     * Sets the interval of time to wait for income activity from AMQP broker
     *
     * @deprecated use AMQPConnection::setReadTimeout($timeout) instead
     *
     * @throws AMQPConnectionException If timeout is less than 0.
     */
    public function setTimeout(float $timeout): void
    {
    }

    /**
     * Get the configured interval of time to wait for income activity
     * from AMQP broker
     *
     * @deprecated use AMQPConnection::getReadTimeout() instead
     */
    public function getTimeout(): float
    {
    }

    /**
     * Sets the interval of time (in seconds) to wait for income activity from AMQP broker
     *
     * @throws AMQPConnectionException If timeout is less than 0.
     */
    public function setReadTimeout(float $timeout): void
    {
    }

    /**
     * Get the configured interval of time (in seconds) to wait for income activity
     * from AMQP broker
     */
    public function getReadTimeout(): float
    {
    }

    /**
     * Sets the interval of time (in seconds) to wait for outcome activity to AMQP broker
     *
     * @throws AMQPConnectionException If timeout is less than 0.
     */
    public function setWriteTimeout(float $timeout): void
    {
    }

    /**
     * Get the configured interval of time (in seconds) to wait for outcome activity
     * to AMQP broker
     */
    public function getWriteTimeout(): float
    {
    }

    /**
     * Get the configured timeout (in seconds) for connecting to the AMQP broker
     */
    public function getConnectTimeout(): float
    {
    }

    /**
     * Sets the interval of time to wait (in seconds) for RPC activity to AMQP broker
     *
     * @throws AMQPConnectionException If timeout is less than 0.
     */
    public function setRpcTimeout(float $timeout): void
    {
    }

    /**
     * Get the configured interval of time (in seconds) to wait for RPC activity
     * to AMQP broker
     */
    public function getRpcTimeout(): float
    {
    }

    /**
     * Return last used channel id during current connection session.
     */
    public function getUsedChannels(): int
    {
    }

    /**
     * Get the maximum number of channels the connection can handle.
     *
     * When connection is connected, effective connection value returned, which is normally the same as original
     * correspondent value passed to constructor, otherwise original value passed to constructor returned.
     */
    public function getMaxChannels(): int
    {
    }

    /**
     * Get max supported frame size per connection in bytes.
     *
     * When connection is connected, effective connection value returned, which is normally the same as original
     * correspondent value passed to constructor, otherwise original value passed to constructor returned.
     */
    public function getMaxFrameSize(): int
    {
    }

    /**
     * Get number of seconds between heartbeats of the connection in seconds.
     *
     * When connection is connected, effective connection value returned, which is normally the same as original
     * correspondent value passed to constructor, otherwise original value passed to constructor returned.
     */
    public function getHeartbeatInterval(): int
    {
    }

    /**
     * Get path to the CA cert file in PEM format
     */
    public function getCACert(): ?string
    {
    }

    /**
     * Set path to the CA cert file in PEM format
     */
    public function setCACert(?string $cacert): void
    {
    }

    /**
     * Get path to the client certificate in PEM format
     */
    public function getCert(): ?string
    {
    }

    /**
     * Set path to the client certificate in PEM format
     */
    public function setCert(?string $cert): void
    {
    }

    /**
     * Get path to the client key in PEM format
     */
    public function getKey(): ?string
    {
    }

    /**
     * Set path to the client key in PEM format
     */
    public function setKey(?string $key): void
    {
    }

    /**
     * Get whether peer verification enabled or disabled
     */
    public function getVerify(): bool
    {
    }

    /**
     * Enable or disable peer verification
     */
    public function setVerify(bool $verify): void
    {
    }

    /**
     * set authentication method
     *
     * @param int $saslMethod AMQP_SASL_METHOD_PLAIN | AMQP_SASL_METHOD_EXTERNAL
     */
    public function setSaslMethod(int $saslMethod): void
    {
    }

    public function getSaslMethod(): int
    {
    }

    public function setConnectionName(?string $connectionName): void
    {
    }

    public function getConnectionName(): ?string
    {
    }
}
