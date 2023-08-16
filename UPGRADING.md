# Breaking changes in 2.0.0

## `AMQPBasicProperties` breaking changes

### Public method type changes

```diff
  __construct(
-     string $content_type = '',
+     ?string $contentType = null,
-     string $content_encoding = '',
+     ?string $contentEncoding = null,
      array $headers = [],
-     int $delivery_mode = 2,
+     int $deliveryMode = 1,
      int $priority = 0,
-     string $correlation_id = '',
+     ?string $correlationId = null,
-     string $reply_to = '',
+     ?string $replyTo = null,
-     string $expiration = '',
+     ?string $expiration = null,
-     string $message_id = '',
+     ?string $messageId = null,
-     int $timestamp = 0,
+     ?int $timestamp = null,
-     string $type = '',
+     ?string $type = null,
-     string $user_id = '',
+     ?string $userId = null,
-     string $app_id = '',
+     ?string $appId = null,
-     string $cluster_id = ''
+     ?string $clusterId = null
  )
```
```diff
- getContentType(): string
+ getContentType(): ?string

```
```diff
- getContentEncoding(): string
+ getContentEncoding(): ?string

```
```diff
- getCorrelationId(): string
+ getCorrelationId(): ?string

```
```diff
- getReplyTo(): string
+ getReplyTo(): ?string

```
```diff
- getExpiration(): string
+ getExpiration(): ?string

```
```diff
- getMessageId(): string
+ getMessageId(): ?string

```
```diff
- getTimestamp(): string
+ getTimestamp(): ?int

```
```diff
- getType(): string
+ getType(): ?string

```
```diff
- getUserId(): string
+ getUserId(): ?string

```
```diff
- getAppId(): string
+ getAppId(): ?string

```
```diff
- getClusterId(): string
+ getClusterId(): ?string

```

## `AMQPChannel` breaking changes

### Public method type changes

```diff
- commitTransaction(): bool
+ commitTransaction(): void

```
```diff
- qos(int $size, int $count, bool $global): bool
+ qos(int $size, int $count, bool $global = false): void

```
```diff
- rollbackTransaction(): bool
+ rollbackTransaction(): void

```
```diff
- setPrefetchCount(int $count): bool
+ setPrefetchCount(int $count): void

```
```diff
- setPrefetchSize(int $size): bool
+ setPrefetchSize(int $size): void

```
```diff
- setGlobalPrefetchCount(int $count): bool
+ setGlobalPrefetchCount(int $count): void

```
```diff
- setGlobalPrefetchSize(int $size): bool
+ setGlobalPrefetchSize(int $size): void

```
```diff
- startTransaction(): bool
+ startTransaction(): void

```
```diff
- setConfirmCallback(?callable $ack_callback = null, ?callable $nack_callback = null): void
+ setConfirmCallback(?callable $ackCallback, ?callable $nackCallback = null): void

```
```diff
- setReturnCallback(?callable $return_callback = null): void
+ setReturnCallback(?callable $returnCallback): void

```

### Parameter name changes

```diff
- __construct(AMQPConnection $amqp_connection)
+ __construct(AMQPConnection $connection)

```


## `AMQPConnection` breaking changes

### Change in semantics

* `connect()`, `pconnect()`, `disconnect()`, `set*()` etc. no longer return true or throw an exception but instead are void and throw

### Public method additions

 * `getConnectTimeout(): float`
 * `setConnectionName(?string $connectionName): void`
 * `getConnectionName(): ?string`

### Public method type changes

```diff
- connect(): bool
+ connect(): void

```
```diff
- disconnect(): bool
+ disconnect(): void

```
```diff
- pconnect(): bool
+ pconnect(): void

```
```diff
- pdisconnect(): bool
+ pdisconnect(): void

```
```diff
- reconnect(): bool
+ reconnect(): void

```
```diff
- preconnect(): bool
+ preconnect(): void

```
```diff
- setHost(string $host): bool
+ setHost(string $host): void

```
```diff
- setLogin(string $login): bool
+ setLogin(string $login): void

```
```diff
- setPassword(string $password): bool
+ setPassword(string $password): void

```
```diff
- setPort(int $port): bool
+ setPort(int $port): void

```
```diff
- setVhost(string $vhost): bool
+ setVhost(string $vhost): void

```
```diff
- setTimeout(float $timeout): bool
+ setTimeout(float $timeout): void

```
```diff
- setReadTimeout(float $timeout): bool
+ setReadTimeout(float $timeout): void

```
```diff
- setWriteTimeout(float $timeout): bool
+ setWriteTimeout(float $timeout): void

```
```diff
- setRpcTimeout(float $timeout): bool
+ setRpcTimeout(float $timeout): void

```
```diff
- getCACert(): string
+ getCACert(): ?string

```
```diff
- setCACert(string $cacert): void
+ setCACert(?string $cacert): void

```
```diff
- getCert(): string
+ getCert(): ?string

```
```diff
- setCert(string $cert): void
+ setCert(?string $cert): void

```
```diff
- getKey(): string
+ getKey(): ?string

```
```diff
- setKey(string $key): void
+ setKey(?string $key): void

```

### Parameter name changes

```diff
- setSaslMethod(int $method): void
+ setSaslMethod(int $saslMethod): void

```


## `AMQPDecimal` breaking changes

### Public method type changes

```diff
- __construct(unknown $exponent, unknown $significand)
+ __construct(int $exponent, int $significand)

```

## `AMQPEnvelope` breaking changes

### Public method type changes

```diff
- getConsumerTag(): string
+ getConsumerTag(): ?string

```
```diff
- getDeliveryTag(): string
+ getDeliveryTag(): ?int

```
```diff
- getExchangeName(): string
+ getExchangeName(): ?string

```
```diff
- getHeader(string $header_key): bool|string
+ getHeader(string $headerName): ?string

```

### Parameter name changes

```diff
- hasHeader(string $header_key): bool
+ hasHeader(string $headerName): bool

```


## `AMQPEnvelopeException` breaking changes

### Public method additions

 * `getEnvelope(): AMQPEnvelope`

## `AMQPExchange` breaking changes

### Change in semantics

* `declareExchange()`, `bind()` , etc. no longer return true or throw an exception but instead are void and throw
* `setArgument(string $argumentName, null)` no longer unsets the argument `$key` but sets it to `null` instead. To remove an argument use `removeArgument(string $argumentName)` instead
* `getArgument(string $argumentName)` now throws an `AMQPExchangeException` if the argument does not exist. It returned false previously 

### Public method additions

* `declare(): void`
* `removeArgument(string $argumentName): void`

### Public method type changes

```diff
- bind(string $exchange_name, string $routing_key = '', array $arguments = []): bool
+ bind(string $exchangeName, ?string $routingKey = null, array $arguments = []): void

```
```diff
- unbind(string $exchange_name, string $routing_key = '', array $arguments = []): bool
+ unbind(string $exchangeName, ?string $routingKey = null, array $arguments = []): void

```
```diff
- declareExchange(): bool
+ declareExchange(): void

```
```diff
- delete(string $exchangeName = null, int $flags = 0): bool
+ delete(?string $exchangeName = null, ?int $flags = null): void

```
```diff
- getArgument(string $key): bool|int|string
+ getArgument(string $argumentName): bool|float|int|string|null

```
```diff
- getName(): string
+ getName(): ?string

```
```diff
- getType(): string
+ getType(): ?string

```
```diff
- publish(string $message, string $routing_key = null, int $flags = 0, array $attributes = []): bool
+ publish(string $message, ?string $routingKey = null, ?int $flags = null, array $headers = []): void

```
```diff
- setArgument(string $key, int|string $value): bool
+ setArgument(string $argumentName, bool|float|int|string|null $argumentValue): void

```
```diff
- setArguments(array $arguments): bool
+ setArguments(array $arguments): void

```
```diff
- setName(string $exchange_name): void
+ setName(?string $exchangeName): void

```
```diff
- setType(string $exchange_type): void
+ setType(?string $exchangeType): void

```

### Parameter name changes

```diff
- __construct(AMQPChannel $amqp_channel)
+ __construct(AMQPChannel $channel)

```

```diff
- hasArgument(string $key): bool
+ hasArgument(string $argumentName): bool

```


## `AMQPQueue` breaking changes

### Change in semantics

* `declareQueue`, `ack()`, `nack()`, `bind()`, etc. etc. methods no longer return true or throw an exception but instead are void and throw
* `get()` now either return null instead of false if no message was received
* `setArgument(string $key, null)` no longer unsets the argument `$key` but sets it to `null` instead. To remove an argument use `removeArgument(string $key)` instead
* `getArgument(string $argumentName)` now throws an `AMQPQueueException` if the argument does not exist. It returned false previously

### Public method additions

* `declare(): int`
* `removeArgument(string $argumentName): void`

### Public method type changes

```diff
- ack(string $delivery_tag, int $flags = 0): bool
+ ack(int $deliveryTag, ?int $flags = null): void

```
```diff
- bind(string $exchange_name, string $routing_key = null, array $arguments = []): bool
+ bind(string $exchangeName, ?string $routingKey = null, array $arguments = []): void

```
```diff
- cancel(string $consumer_tag = ''): bool
+ cancel(string $consumerTag = ''): void

```
```diff
- consume(?callable $callback = null, int $flags = 0, string $consumerTag = null): void
+ consume(?callable $callback = null, ?int $flags = null, ?string $consumerTag = null): void

```
```diff
- delete(int $flags = 0): int
+ delete(?int $flags = null): int

```
```diff
- get(int $flags = 0): AMQPEnvelope|bool
+ get(?int $flags = null): ?AMQPEnvelope

```
```diff
- getArgument(string $key): bool|int|string
+ getArgument(string $argumentName): bool|float|int|string|null

```
```diff
- getName(): string
+ getName(): ?string

```
```diff
- nack(string $delivery_tag, int $flags = 0): bool
+ nack(int $deliveryTag, ?int $flags = null): void

```
```diff
- reject(string $delivery_tag, int $flags = 0): bool
+ reject(int $deliveryTag, ?int $flags = null): void

```
```diff
- purge(): bool
+ purge(): int

```
```diff
- setArgument(string $key, mixed $value): bool
+ setArgument(string $argumentName, bool|float|int|string|null $argumentValue): void

```
```diff
- setArguments(array $arguments): bool
+ setArguments(array $arguments): void

```
```diff
- setFlags(int $flags): bool
+ setFlags(?int $flags): void

```
```diff
- setName(string $queue_name): bool
+ setName(string $name): void

```
```diff
- unbind(string $exchange_name, string $routing_key = null, array $arguments = []): bool
+ unbind(string $exchangeName, ?string $routingKey = null, array $arguments = []): void

```

### Parameter name changes

```diff
- __construct(AMQPChannel $amqp_channel)
+ __construct(AMQPChannel $channel)

```

```diff
- hasArgument(string $key): bool
+ hasArgument(string $argumentName): bool

```


## `AMQPTimestamp` breaking changes

### Public constant changes

```diff
- const MIN = '0'
+ const MIN = 0.0

```

```diff
- const MAX = '18446744073709551616'
+ const MAX = 1.8446744073709552E+19

```


### Public method type changes

```diff
- __construct(string $timestamp)
+ __construct(float $timestamp)

```
```diff
- getTimestamp(): string
+ getTimestamp(): float

```
