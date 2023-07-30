<?php
/**
 * Passing in this constant as a flag will forcefully disable all other flags.
 * Use this if you want to temporarily disable the amqp.auto_ack ini setting.
 */
const AMQP_NOPARAM = 0;

/**
 * Passing in this constant as a flag to proper methods will forcefully ignore all other flags.
 * Do not send basic.consume request during AMQPQueue::consume(). Use this if you want to run callback on top of previously
 * declared consumers.
 */
const AMQP_JUST_CONSUME = 1;

/**
 * Durable exchanges and queues will survive a broker restart, complete with all of their data.
 */
const AMQP_DURABLE = 2;

/**
 * Passive exchanges and queues will not be redeclared, but the broker will throw an error if the exchange or queue does not exist.
 */
const AMQP_PASSIVE = 4;

/**
 * Valid for queues only, this flag indicates that only one client can be listening to and consuming from this queue.
 */
const AMQP_EXCLUSIVE = 8;

/**
 * For exchanges, the auto delete flag indicates that the exchange will be deleted as soon as no more queues are bound
 * to it. If no queues were ever bound the exchange, the exchange will never be deleted. For queues, the auto delete
 * flag indicates that the queue will be deleted as soon as there are no more listeners subscribed to it. If no
 * subscription has ever been active, the queue will never be deleted. Note: Exclusive queues will always be
 * automatically deleted with the client disconnects.
 */
const AMQP_AUTODELETE = 16;

/**
 * Clients are not allowed to make specific queue bindings to exchanges defined with this flag.
 */
const AMQP_INTERNAL = 32;

/**
 * When passed to the consume method for a clustered environment, do not consume from the local node.
 */
const AMQP_NOLOCAL = 64;

/**
 * When passed to the {@link AMQPQueue::get()} and {@link AMQPQueue::consume()} methods as a flag,
 * the messages will be immediately marked as acknowledged by the server upon delivery.
 */
const AMQP_AUTOACK = 128;

/**
 * Passed on queue creation, this flag indicates that the queue should be deleted if it becomes empty.
 */
const AMQP_IFEMPTY = 256;

/**
 * Passed on queue or exchange creation, this flag indicates that the queue or exchange should be
 * deleted when no clients are connected to the given queue or exchange.
 */
const AMQP_IFUNUSED = 512;

/**
 * When publishing a message, the message must be routed to a valid queue. If it is not, an error will be returned.
 */
const AMQP_MANDATORY = 1024;

/**
 * When publishing a message, mark this message for immediate processing by the broker. (High priority message.)
 */
const AMQP_IMMEDIATE = 2048;

/**
 * If set during a call to {@link AMQPQueue::ack()}, the delivery tag is treated as "up to and including", so that multiple
 * messages can be acknowledged with a single method. If set to zero, the delivery tag refers to a single message.
 * If the AMQP_MULTIPLE flag is set, and the delivery tag is zero, this indicates acknowledgement of all outstanding
 * messages.
 */
const AMQP_MULTIPLE = 4096;

/**
 * If set during a call to {@link AMQPExchange::bind()}, the server will not respond to the method.The client should not wait
 * for a reply method. If the server could not complete the method it will raise a channel or connection exception.
 */
const AMQP_NOWAIT = 8192;

/**
 * If set during a call to {@link AMQPQueue::nack()}, the message will be placed back to the queue.
 */
const AMQP_REQUEUE = 16384;

/**
 * A direct exchange type.
 */
const AMQP_EX_TYPE_DIRECT = 'direct';

/**
 * A fanout exchange type.
 */
const AMQP_EX_TYPE_FANOUT = 'fanout';

/**
 * A topic exchange type.
 */
const AMQP_EX_TYPE_TOPIC = 'topic';

/**
 * A header exchange type.
 */
const AMQP_EX_TYPE_HEADERS = 'headers';


const AMQP_OS_SOCKET_TIMEOUT_ERRNO = 536870947;



const PHP_AMQP_MAX_CHANNELS = 256;


const AMQP_SASL_METHOD_PLAIN = 0;


const AMQP_SASL_METHOD_EXTERNAL = 1;

/**
 * Default delivery mode, keeps the message in memory when the message is placed in a queue.
 */
const AMQP_DELIVERY_MODE_TRANSIENT = 1;

/**
 * Writes the message to the disk when the message is placed in a durable queue.
 */
const AMQP_DELIVERY_MODE_PERSISTENT = 2;

/**
 * Extension version string
 */
const AMQP_EXTENSION_VERSION = '1.1.12alpha3';

/**
 * Extension major version
 */
const AMQP_EXTENSION_VERSION_MAJOR = 0;

/**
 * Extension minor version
 */
const AMQP_EXTENSION_VERSION_MINOR = 1;

/**
 * Extension patch version
 */
const AMQP_EXTENSION_VERSION_PATCH = 12;

/**
 * Extension extra version suffix
 */
const AMQP_EXTENSION_VERSION_EXTRA = 'alpha3';

/**
 * Extension version ID
 */
const AMQP_EXTENSION_VERSION_ID = '10112';
