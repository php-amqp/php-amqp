/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2007 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Alexandre Kalendarev akalend@mail.ru Copyright (c) 2009-2010 |
  | Lead:                                                                |
  | - Pieter de Zwart                                                    |
  | Maintainers:                                                         |
  | - Brad Rodriguez                                                     |
  | - Jonathan Tansavatdi                                                |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "zend_ini.h"
#include "zend_exceptions.h"

#ifdef PHP_WIN32
# include "win32/php_stdint.h"
# include "win32/signal.h"
#else
# include <stdint.h>
# include <signal.h>
#endif

#include <amqp.h>
#include <amqp_framing.h>

#include "php_amqp.h"
#include "amqp_connection.h"
#include "amqp_channel.h"
#include "amqp_queue.h"
#include "amqp_exchange.h"
#include "amqp_envelope.h"
#include "amqp_connection_resource.h"


#ifdef PHP_WIN32
# include "win32/unistd.h"
#else
# include <unistd.h>
#endif

/* True global resources - no need for thread safety here */
zend_class_entry *amqp_connection_class_entry;
zend_class_entry *amqp_channel_class_entry;
zend_class_entry *amqp_queue_class_entry;
zend_class_entry *amqp_exchange_class_entry;
zend_class_entry *amqp_envelope_class_entry;

zend_class_entry *amqp_exception_class_entry,
				 *amqp_connection_exception_class_entry,
				 *amqp_channel_exception_class_entry,
				 *amqp_queue_exception_class_entry,
				 *amqp_exchange_exception_class_entry;


/* The last parameter of ZEND_BEGIN_ARG_INFO_EX indicates how many of the method flags are required. */
/* The first parameter of ZEND_ARG_INFO indicates whether the variable is being passed by reference */

/* amqp_connection_class ARG_INFO definition */
ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class__construct, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_ARRAY_INFO(0, credentials, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_isConnected, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_connect, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_pconnect, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_pdisconnect, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_disconnect, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_reconnect, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_preconnect, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_getLogin, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_setLogin, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, login)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_getPassword, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_setPassword, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, password)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_getHost, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_setHost, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, host)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_getPort, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_setPort, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, port)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_getVhost, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_setVhost, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, vhost)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_getTimeout, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_setTimeout, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, timeout)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_getReadTimeout, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_setReadTimeout, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, timeout)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_getWriteTimeout, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_setWriteTimeout, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, timeout)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_getUsedChannels, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_getMaxChannels, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_getMaxFrameSize, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_getHeartbeatInterval, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_isPersistent, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

/* amqp_channel_class ARG_INFO definition */
ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class__construct, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, amqp_connection)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_isConnected, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_getChannelId, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_setPrefetchSize, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_getPrefetchSize, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_setPrefetchCount, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, count)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_getPrefetchCount, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_qos, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 2)
	ZEND_ARG_INFO(0, size)
	ZEND_ARG_INFO(0, count)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_startTransaction, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_commitTransaction, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_rollbackTransaction, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_getConnection, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_basicRecover, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, requeue)
ZEND_END_ARG_INFO()

/* amqp_queue_class ARG_INFO definition */
ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_queue_class__construct, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, amqp_channel)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_queue_class_getName, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_queue_class_setName, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, queue_name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_queue_class_getFlags, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_queue_class_setFlags, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_queue_class_getArgument, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, argument)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_queue_class_getArguments, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_queue_class_setArgument, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 2)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_queue_class_setArguments, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_ARRAY_INFO(0, arguments, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_queue_class_declareQueue, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_queue_class_bind, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, exchange_name)
	ZEND_ARG_INFO(0, routing_key)
	ZEND_ARG_INFO(0, arguments)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_queue_class_get, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_queue_class_consume, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, flags)
	ZEND_ARG_INFO(0, consumer_tag)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_queue_class_ack, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, delivery_tag)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_queue_class_nack, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, delivery_tag)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_queue_class_reject, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, delivery_tag)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_queue_class_purge, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_queue_class_cancel, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, consumer_tag)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_queue_class_unbind, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, exchange_name)
	ZEND_ARG_INFO(0, routing_key)
	ZEND_ARG_INFO(0, arguments)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_queue_class_delete, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_queue_class_getChannel, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_queue_class_getConnection, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_queue_class_getConsumerTag, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

/* amqp_exchange ARG_INFO definition */
ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_exchange_class__construct, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, amqp_channel)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_exchange_class_getName, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_exchange_class_setName, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, exchange_name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_exchange_class_getFlags, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_exchange_class_setFlags, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_exchange_class_getType, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_exchange_class_setType, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, exchange_type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_exchange_class_getArgument, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, argument)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_exchange_class_getArguments, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_exchange_class_setArgument, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 2)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_exchange_class_setArguments, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_ARRAY_INFO(0, arguments, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_exchange_class_declareExchange, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_exchange_class_bind, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 2)
	ZEND_ARG_INFO(0, exchange_name)
	ZEND_ARG_INFO(0, routing_key)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_exchange_class_unbind, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 2)
	ZEND_ARG_INFO(0, exchange_name)
	ZEND_ARG_INFO(0, routing_key)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_exchange_class_delete, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, exchange_name)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_exchange_class_publish, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, message)
	ZEND_ARG_INFO(0, routing_key)
	ZEND_ARG_INFO(0, flags)
	ZEND_ARG_ARRAY_INFO(0, headers, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_exchange_class_getChannel, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_exchange_class_getConnection, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

/* amqp_envelope_class ARG_INFO definition */
ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class__construct, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getBody, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getRoutingKey, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getDeliveryTag, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getDeliveryMode, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getExchangeName, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_isRedelivery, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getContentType, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getContentEncoding, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getType, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getTimestamp, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getPriority, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getExpiration, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getUserId, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getAppId, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getMessageId, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getReplyTo, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getCorrelationId, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getHeaders, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getHeader, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()


/* {{{ amqp_functions[]
*
*Every user visible function must have an entry in amqp_functions[].
*/
zend_function_entry amqp_connection_class_functions[] = {
	PHP_ME(amqp_connection_class, __construct, 	arginfo_amqp_connection_class__construct,	ZEND_ACC_PUBLIC)
	PHP_ME(amqp_connection_class, isConnected, 	arginfo_amqp_connection_class_isConnected,	ZEND_ACC_PUBLIC)
	PHP_ME(amqp_connection_class, connect, 		arginfo_amqp_connection_class_connect, 		ZEND_ACC_PUBLIC)
	PHP_ME(amqp_connection_class, pconnect, 	arginfo_amqp_connection_class_pconnect, 	ZEND_ACC_PUBLIC)
	PHP_ME(amqp_connection_class, pdisconnect, 	arginfo_amqp_connection_class_pdisconnect,	ZEND_ACC_PUBLIC)
	PHP_ME(amqp_connection_class, disconnect, 	arginfo_amqp_connection_class_disconnect,	ZEND_ACC_PUBLIC)
	PHP_ME(amqp_connection_class, reconnect, 	arginfo_amqp_connection_class_reconnect,	ZEND_ACC_PUBLIC)
	PHP_ME(amqp_connection_class, preconnect, 	arginfo_amqp_connection_class_preconnect,	ZEND_ACC_PUBLIC)

	PHP_ME(amqp_connection_class, getLogin, 	arginfo_amqp_connection_class_getLogin,		ZEND_ACC_PUBLIC)
	PHP_ME(amqp_connection_class, setLogin, 	arginfo_amqp_connection_class_setLogin,		ZEND_ACC_PUBLIC)

	PHP_ME(amqp_connection_class, getPassword, 	arginfo_amqp_connection_class_getPassword,	ZEND_ACC_PUBLIC)
	PHP_ME(amqp_connection_class, setPassword, 	arginfo_amqp_connection_class_setPassword,	ZEND_ACC_PUBLIC)

	PHP_ME(amqp_connection_class, getHost, 		arginfo_amqp_connection_class_getHost,		ZEND_ACC_PUBLIC)
	PHP_ME(amqp_connection_class, setHost, 		arginfo_amqp_connection_class_setHost,		ZEND_ACC_PUBLIC)

	PHP_ME(amqp_connection_class, getPort, 		arginfo_amqp_connection_class_getPort,		ZEND_ACC_PUBLIC)
	PHP_ME(amqp_connection_class, setPort, 		arginfo_amqp_connection_class_setPort,		ZEND_ACC_PUBLIC)

	PHP_ME(amqp_connection_class, getVhost, 	arginfo_amqp_connection_class_getVhost,		ZEND_ACC_PUBLIC)
	PHP_ME(amqp_connection_class, setVhost, 	arginfo_amqp_connection_class_setVhost,		ZEND_ACC_PUBLIC)

	PHP_ME(amqp_connection_class, getTimeout, 	arginfo_amqp_connection_class_getTimeout,	ZEND_ACC_PUBLIC)
	PHP_ME(amqp_connection_class, setTimeout, 	arginfo_amqp_connection_class_setTimeout,	ZEND_ACC_PUBLIC)

	PHP_ME(amqp_connection_class, getReadTimeout, 	arginfo_amqp_connection_class_getReadTimeout,	ZEND_ACC_PUBLIC)
	PHP_ME(amqp_connection_class, setReadTimeout, 	arginfo_amqp_connection_class_setReadTimeout,	ZEND_ACC_PUBLIC)

	PHP_ME(amqp_connection_class, getWriteTimeout, 	arginfo_amqp_connection_class_getWriteTimeout,	ZEND_ACC_PUBLIC)
	PHP_ME(amqp_connection_class, setWriteTimeout, 	arginfo_amqp_connection_class_setWriteTimeout,	ZEND_ACC_PUBLIC)

	PHP_ME(amqp_connection_class, getUsedChannels, arginfo_amqp_connection_class_getUsedChannels,	ZEND_ACC_PUBLIC)
	PHP_ME(amqp_connection_class, getMaxChannels,  arginfo_amqp_connection_class_getMaxChannels,	ZEND_ACC_PUBLIC)
	PHP_ME(amqp_connection_class, isPersistent, 	arginfo_amqp_connection_class_isPersistent,		ZEND_ACC_PUBLIC)
#if AMQP_VERSION_MAJOR * 100 + AMQP_VERSION_MINOR * 10 + AMQP_VERSION_PATCH > 52
	PHP_ME(amqp_connection_class, getHeartbeatInterval,  arginfo_amqp_connection_class_getHeartbeatInterval,	ZEND_ACC_PUBLIC)
	PHP_ME(amqp_connection_class, getMaxFrameSize,  arginfo_amqp_connection_class_getMaxFrameSize,	ZEND_ACC_PUBLIC)
#endif

	{NULL, NULL, NULL}	/* Must be the last line in amqp_functions[] */
};

zend_function_entry amqp_channel_class_functions[] = {
	PHP_ME(amqp_channel_class, __construct, 	arginfo_amqp_channel_class__construct,		ZEND_ACC_PUBLIC)
	PHP_ME(amqp_channel_class, isConnected, 	arginfo_amqp_channel_class_isConnected,		ZEND_ACC_PUBLIC)

	PHP_ME(amqp_channel_class, getChannelId,    arginfo_amqp_channel_class_getChannelId,    ZEND_ACC_PUBLIC)

	PHP_ME(amqp_channel_class, setPrefetchSize, arginfo_amqp_channel_class_setPrefetchSize,	ZEND_ACC_PUBLIC)
	PHP_ME(amqp_channel_class, getPrefetchSize, arginfo_amqp_channel_class_getPrefetchSize,	ZEND_ACC_PUBLIC)
	PHP_ME(amqp_channel_class, setPrefetchCount,arginfo_amqp_channel_class_setPrefetchCount,ZEND_ACC_PUBLIC)
	PHP_ME(amqp_channel_class, getPrefetchCount,arginfo_amqp_channel_class_getPrefetchCount,ZEND_ACC_PUBLIC)
	PHP_ME(amqp_channel_class, qos,				arginfo_amqp_channel_class_qos,				ZEND_ACC_PUBLIC)

	PHP_ME(amqp_channel_class, startTransaction,	arginfo_amqp_channel_class_startTransaction,	ZEND_ACC_PUBLIC)
	PHP_ME(amqp_channel_class, commitTransaction,	arginfo_amqp_channel_class_commitTransaction,	ZEND_ACC_PUBLIC)
	PHP_ME(amqp_channel_class, rollbackTransaction,	arginfo_amqp_channel_class_rollbackTransaction,	ZEND_ACC_PUBLIC)

	PHP_ME(amqp_channel_class, getConnection,	arginfo_amqp_channel_class_getConnection, ZEND_ACC_PUBLIC)

	PHP_ME(amqp_channel_class, basicRecover,	arginfo_amqp_channel_class_basicRecover, ZEND_ACC_PUBLIC)

	{NULL, NULL, NULL}	/* Must be the last line in amqp_functions[] */
};

zend_function_entry amqp_queue_class_functions[] = {
	PHP_ME(amqp_queue_class, __construct,		arginfo_amqp_queue_class__construct,		ZEND_ACC_PUBLIC)

	PHP_ME(amqp_queue_class, getName,			arginfo_amqp_queue_class_getName,			ZEND_ACC_PUBLIC)
	PHP_ME(amqp_queue_class, setName,			arginfo_amqp_queue_class_setName,			ZEND_ACC_PUBLIC)

	PHP_ME(amqp_queue_class, getFlags,			arginfo_amqp_queue_class_getFlags,			ZEND_ACC_PUBLIC)
	PHP_ME(amqp_queue_class, setFlags,			arginfo_amqp_queue_class_setFlags,			ZEND_ACC_PUBLIC)

	PHP_ME(amqp_queue_class, getArgument,		arginfo_amqp_queue_class_getArgument,		ZEND_ACC_PUBLIC)
	PHP_ME(amqp_queue_class, getArguments,		arginfo_amqp_queue_class_getArguments,		ZEND_ACC_PUBLIC)
	PHP_ME(amqp_queue_class, setArgument,		arginfo_amqp_queue_class_setArgument,		ZEND_ACC_PUBLIC)
	PHP_ME(amqp_queue_class, setArguments,		arginfo_amqp_queue_class_setArguments,		ZEND_ACC_PUBLIC)

	PHP_ME(amqp_queue_class, declareQueue,		arginfo_amqp_queue_class_declareQueue,			ZEND_ACC_PUBLIC)
	PHP_ME(amqp_queue_class, bind,				arginfo_amqp_queue_class_bind,				ZEND_ACC_PUBLIC)

	PHP_ME(amqp_queue_class, get,				arginfo_amqp_queue_class_get,				ZEND_ACC_PUBLIC)
	PHP_ME(amqp_queue_class, consume,			arginfo_amqp_queue_class_consume,			ZEND_ACC_PUBLIC)
	PHP_ME(amqp_queue_class, ack,				arginfo_amqp_queue_class_ack,				ZEND_ACC_PUBLIC)
	PHP_ME(amqp_queue_class, nack,				arginfo_amqp_queue_class_nack,				ZEND_ACC_PUBLIC)
	PHP_ME(amqp_queue_class, reject,			arginfo_amqp_queue_class_reject,			ZEND_ACC_PUBLIC)
	PHP_ME(amqp_queue_class, purge,				arginfo_amqp_queue_class_purge,				ZEND_ACC_PUBLIC)

	PHP_ME(amqp_queue_class, cancel,			arginfo_amqp_queue_class_cancel,			ZEND_ACC_PUBLIC)
	PHP_ME(amqp_queue_class, delete,			arginfo_amqp_queue_class_delete,			ZEND_ACC_PUBLIC)
	PHP_ME(amqp_queue_class, unbind,			arginfo_amqp_queue_class_unbind,			ZEND_ACC_PUBLIC)

	PHP_ME(amqp_queue_class, getChannel,		arginfo_amqp_queue_class_getChannel,		ZEND_ACC_PUBLIC)
	PHP_ME(amqp_queue_class, getConnection,		arginfo_amqp_queue_class_getConnection,		ZEND_ACC_PUBLIC)
	PHP_ME(amqp_queue_class, getConsumerTag,	arginfo_amqp_queue_class_getConsumerTag,	ZEND_ACC_PUBLIC)

	PHP_MALIAS(amqp_queue_class, declare, declareQueue, arginfo_amqp_queue_class_declareQueue,	ZEND_ACC_PUBLIC | ZEND_ACC_DEPRECATED)

	{NULL, NULL, NULL}	/* Must be the last line in amqp_functions[] */
};

zend_function_entry amqp_exchange_class_functions[] = {
	PHP_ME(amqp_exchange_class, __construct,	arginfo_amqp_exchange_class__construct, 	ZEND_ACC_PUBLIC)

	PHP_ME(amqp_exchange_class, getName,		arginfo_amqp_exchange_class_getName,		ZEND_ACC_PUBLIC)
	PHP_ME(amqp_exchange_class, setName,		arginfo_amqp_exchange_class_setName,		ZEND_ACC_PUBLIC)

	PHP_ME(amqp_exchange_class, getFlags,		arginfo_amqp_exchange_class_getFlags,		ZEND_ACC_PUBLIC)
	PHP_ME(amqp_exchange_class, setFlags,		arginfo_amqp_exchange_class_setFlags,		ZEND_ACC_PUBLIC)

	PHP_ME(amqp_exchange_class, getType,		arginfo_amqp_exchange_class_getType,		ZEND_ACC_PUBLIC)
	PHP_ME(amqp_exchange_class, setType,		arginfo_amqp_exchange_class_setType,		ZEND_ACC_PUBLIC)

	PHP_ME(amqp_exchange_class, getArgument,	arginfo_amqp_exchange_class_getArgument,	ZEND_ACC_PUBLIC)
	PHP_ME(amqp_exchange_class, getArguments,	arginfo_amqp_exchange_class_getArguments,	ZEND_ACC_PUBLIC)
	PHP_ME(amqp_exchange_class, setArgument,	arginfo_amqp_exchange_class_setArgument,	ZEND_ACC_PUBLIC)
	PHP_ME(amqp_exchange_class, setArguments,	arginfo_amqp_exchange_class_setArguments,	ZEND_ACC_PUBLIC)

	PHP_ME(amqp_exchange_class, declareExchange,arginfo_amqp_exchange_class_declareExchange,ZEND_ACC_PUBLIC)
	PHP_ME(amqp_exchange_class, bind,			arginfo_amqp_exchange_class_bind,			ZEND_ACC_PUBLIC)
	PHP_ME(amqp_exchange_class, unbind,			arginfo_amqp_exchange_class_unbind,			ZEND_ACC_PUBLIC)
	PHP_ME(amqp_exchange_class, delete,			arginfo_amqp_exchange_class_delete,			ZEND_ACC_PUBLIC)
	PHP_ME(amqp_exchange_class, publish,		arginfo_amqp_exchange_class_publish,		ZEND_ACC_PUBLIC)

	PHP_ME(amqp_exchange_class, getChannel,		arginfo_amqp_exchange_class_getChannel,		ZEND_ACC_PUBLIC)
	PHP_ME(amqp_exchange_class, getConnection,	arginfo_amqp_exchange_class_getConnection,	ZEND_ACC_PUBLIC)

	PHP_MALIAS(amqp_exchange_class, declare, declareExchange, arginfo_amqp_exchange_class_declareExchange, ZEND_ACC_PUBLIC | ZEND_ACC_DEPRECATED)

	{NULL, NULL, NULL}	/* Must be the last line in amqp_functions[] */
};

zend_function_entry amqp_envelope_class_functions[] = {
	PHP_ME(amqp_envelope_class, __construct, 		arginfo_amqp_envelope_class__construct,			ZEND_ACC_PUBLIC)
	PHP_ME(amqp_envelope_class, getBody, 			arginfo_amqp_envelope_class_getBody,			ZEND_ACC_PUBLIC)
	PHP_ME(amqp_envelope_class, getRoutingKey, 		arginfo_amqp_envelope_class_getRoutingKey,		ZEND_ACC_PUBLIC)
	PHP_ME(amqp_envelope_class, getDeliveryTag, 	arginfo_amqp_envelope_class_getDeliveryTag,		ZEND_ACC_PUBLIC)
	PHP_ME(amqp_envelope_class, getDeliveryMode, 	arginfo_amqp_envelope_class_getDeliveryMode,	ZEND_ACC_PUBLIC)
	PHP_ME(amqp_envelope_class, getExchangeName, 	arginfo_amqp_envelope_class_getExchangeName,	ZEND_ACC_PUBLIC)
	PHP_ME(amqp_envelope_class, isRedelivery, 		arginfo_amqp_envelope_class_isRedelivery,		ZEND_ACC_PUBLIC)
	PHP_ME(amqp_envelope_class, getContentType, 	arginfo_amqp_envelope_class_getContentType,		ZEND_ACC_PUBLIC)
	PHP_ME(amqp_envelope_class, getContentEncoding, arginfo_amqp_envelope_class_getContentEncoding,	ZEND_ACC_PUBLIC)
	PHP_ME(amqp_envelope_class, getType, 			arginfo_amqp_envelope_class_getType,			ZEND_ACC_PUBLIC)
	PHP_ME(amqp_envelope_class, getTimestamp, 		arginfo_amqp_envelope_class_getTimestamp,		ZEND_ACC_PUBLIC)
	PHP_ME(amqp_envelope_class, getPriority, 		arginfo_amqp_envelope_class_getPriority,		ZEND_ACC_PUBLIC)
	PHP_ME(amqp_envelope_class, getExpiration, 		arginfo_amqp_envelope_class_getExpiration,		ZEND_ACC_PUBLIC)
	PHP_ME(amqp_envelope_class, getUserId, 			arginfo_amqp_envelope_class_getUserId,			ZEND_ACC_PUBLIC)
	PHP_ME(amqp_envelope_class, getAppId, 			arginfo_amqp_envelope_class_getAppId,			ZEND_ACC_PUBLIC)
	PHP_ME(amqp_envelope_class, getMessageId, 		arginfo_amqp_envelope_class_getMessageId,		ZEND_ACC_PUBLIC)
	PHP_ME(amqp_envelope_class, getReplyTo, 		arginfo_amqp_envelope_class_getReplyTo,			ZEND_ACC_PUBLIC)
	PHP_ME(amqp_envelope_class, getCorrelationId, 	arginfo_amqp_envelope_class_getCorrelationId,	ZEND_ACC_PUBLIC)
	PHP_ME(amqp_envelope_class, getHeaders, 		arginfo_amqp_envelope_class_getHeaders,			ZEND_ACC_PUBLIC)
	PHP_ME(amqp_envelope_class, getHeader, 			arginfo_amqp_envelope_class_getHeader,			ZEND_ACC_PUBLIC)

	{NULL, NULL, NULL}	/* Must be the last line in amqp_functions[] */
};

zend_function_entry amqp_functions[] = {
	{NULL, NULL, NULL}	/* Must be the last line in amqp_functions[] */
};
/* }}} */

/* {{{ amqp_module_entry
*/
zend_module_entry amqp_module_entry = {
	STANDARD_MODULE_HEADER,
	"amqp",
	amqp_functions,
	PHP_MINIT(amqp),
	PHP_MSHUTDOWN(amqp),
	NULL,
	NULL,
	PHP_MINFO(amqp),
	PHP_AMQP_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_AMQP
	ZEND_GET_MODULE(amqp)
#endif

void php_amqp_error(amqp_rpc_reply_t reply, char **message, amqp_connection_object *connection, amqp_channel_object *channel TSRMLS_DC)
{
	assert(connection != NULL);
	assert(connection->connection_resource != NULL);

	switch (php_amqp_connection_resource_error(reply, message, connection->connection_resource, (channel ? channel->channel_id : 0) TSRMLS_CC)) {
		case PHP_AMQP_RESOURCE_RESPONSE_OK:
			break;
		case PHP_AMQP_RESOURCE_RESPONSE_ERROR:
			/* Library or other non-protocol or even protocol related errors may be here, do nothing with this for now. */
			break;
		case PHP_AMQP_RESOURCE_RESPONSE_ERROR_CHANNEL_CLOSED:
			/* Mark channel as closed to prevent sending channel.close request */
			assert(channel != NULL);
			channel->is_connected = '\0';

			/* Close channel */
			php_amqp_close_channel(channel TSRMLS_CC);

			/* No more error handling necessary, returning. */
			break;
		case PHP_AMQP_RESOURCE_RESPONSE_ERROR_CONNECTION_CLOSED:
			/* Mark connection as closed to prevent sending any further requests */
			connection->is_connected = '\0';

			/* Close connection with all its channels */
			php_amqp_disconnect_force(connection TSRMLS_CC);

			/* No more error handling necessary, returning. */
			break;
		default:
			spprintf(message, 0, "Unknown server error, method id 0x%08X (not handled by extension)", reply.reply.id);
			break;
	}
}

void php_amqp_zend_throw_exception(amqp_rpc_reply_t reply, zend_class_entry *exception_ce, const char *message, zend_long code TSRMLS_DC)
{
	switch (reply.reply_type) {
		case AMQP_RESPONSE_NORMAL:
			break;
		case AMQP_RESPONSE_NONE:
			exception_ce = amqp_exception_class_entry;
			break;
		case AMQP_RESPONSE_LIBRARY_EXCEPTION:
			exception_ce = amqp_exception_class_entry;
			break;
		case AMQP_RESPONSE_SERVER_EXCEPTION:
			switch (reply.reply.id) {
				case AMQP_CONNECTION_CLOSE_METHOD:
					/* Fatal errors - pass them to connection level */
					exception_ce = amqp_connection_exception_class_entry;
					break;
				case AMQP_CHANNEL_CLOSE_METHOD:
					/* Most channel-level errors occurs due to previously known action and thus their kind can be predicted. */
					/* exception_ce = amqp_channel_exception_class_entry; */
					break;
			}
			break;
		/* Default for the above switch should be handled by the below default. */
		default:
			exception_ce = amqp_exception_class_entry;
			break;
	}

	zend_throw_exception(exception_ce, message, code TSRMLS_CC);
}

void php_amqp_maybe_release_buffers_on_channel(amqp_connection_object *connection, amqp_channel_object *channel)
{
	assert(connection != NULL);
	assert(channel != NULL);
	assert(channel->channel_id > 0);

	if (connection->connection_resource) {
		amqp_maybe_release_buffers_on_channel(connection->connection_resource->connection_state, channel->channel_id);
	}
}

amqp_bytes_t php_amqp_long_string(const char *cstr, size_t len)
{
	if (len < 1) {
		return amqp_empty_bytes;
	}

	amqp_bytes_t result;
	result.len   = len;
	result.bytes = (void *) cstr;
	return result;
}

char *stringify_bytes(amqp_bytes_t bytes)
{
/* We will need up to 4 chars per byte, plus the terminating 0 */
	char *res = emalloc(bytes.len * 4 + 1);
	uint8_t *data = bytes.bytes;
	char *p = res;
	size_t i;

	for (i = 0; i < bytes.len; i++) {
		if (data[i] >= 32 && data[i] != 127) {
			*p++ = data[i];
		} else {
			*p++ = '\\';
			*p++ = '0' + (data[i] >> 6);
			*p++ = '0' + (data[i] >> 3 & 0x7);
			*p++ = '0' + (data[i] & 0x7);
		}
	}

	*p = 0;
	return res;
}

void internal_convert_zval_to_amqp_table(zval *zvalArguments, amqp_table_t *arguments, char allow_int_keys TSRMLS_DC)
{
	HashTable *argumentHash = Z_ARRVAL_P(zvalArguments);

	// HashTable iteration
	zend_ulong index;
	zend_string* real_key;
	zval *value;

	/* Allocate all the memory necessary for storing the arguments. This might be more than we need */
	arguments->entries = (amqp_table_entry_t *)ecalloc(zend_hash_num_elements(argumentHash), sizeof(amqp_table_entry_t));
	arguments->num_entries = 0;

	ZEND_HASH_FOREACH_KEY_VAL(argumentHash, index, real_key, value) {
		char tmp_str[32];
		char type[16];
		char *key;
		size_t key_len;
		amqp_table_entry_t *table = &arguments->entries[arguments->num_entries++];
		amqp_field_value_t *field = &table->value;

		if (real_key  == NULL) {
			if (allow_int_keys) {
				/* Convert to strings non-string keys */
				key_len = (size_t)sprintf(tmp_str, "%lu", index);
				key     = tmp_str;
			} else {
				/* Skip things that are not strings */
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Ignoring non-string header field '%lu'", index);

				/* Reset entries counter back */
				arguments->num_entries --;
				continue;
			}
		} else {
			key_len = ZSTR_LEN(real_key);
			key = ZSTR_VAL(real_key);
		}

		switch (Z_TYPE_P(value)) {
			case IS_TRUE:
			case IS_FALSE:
				field->kind          = AMQP_FIELD_KIND_BOOLEAN;
				field->value.boolean = (amqp_boolean_t)Z_LVAL_P(value);
				break;
			case IS_DOUBLE:
				field->kind      = AMQP_FIELD_KIND_F64;
				field->value.f64 = Z_DVAL_P(value);
				break;
			case IS_LONG:
				field->kind      = AMQP_FIELD_KIND_I64;
				field->value.i64 = Z_LVAL_P(value);
				break;
			case IS_STRING:
				field->kind        = AMQP_FIELD_KIND_UTF8;
				field->value.bytes = php_amqp_zend_string_copy(Z_STR_P(value));
				break;
			case IS_ARRAY:
				field->kind = AMQP_FIELD_KIND_TABLE;
				internal_convert_zval_to_amqp_table(value, &field->value.table, 1 TSRMLS_CC);
				break;
			default:
				switch(Z_TYPE_P(value)) {
					case IS_NULL:     strcpy(type, "null"); break;
					case IS_OBJECT:   strcpy(type, "object"); break;
					case IS_RESOURCE: strcpy(type, "resource"); break;
					default:          strcpy(type, "unknown");
				}

				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Ignoring field '%s' due to unsupported value type (%s)", key, type);

				/* Reset entries counter back */
				arguments->num_entries --;
				continue;
		}

		table->key = php_amqp_long_string(estrndup(key, key_len), key_len);
	} ZEND_HASH_FOREACH_END();
}

inline amqp_table_t *convert_zval_to_amqp_table(zval *zvalArguments TSRMLS_DC)
{
	amqp_table_t *arguments;
	/* In setArguments, we are overwriting all the existing values */
	arguments = (amqp_table_t *)emalloc(sizeof(amqp_table_t));

	internal_convert_zval_to_amqp_table(zvalArguments, arguments, 0 TSRMLS_CC);

	return arguments;
}

void internal_php_amqp_free_amqp_table(amqp_table_t *object, char clear_root)
{
	if (!object) {
		return;
	}

	if (object->entries) {
		int macroEntryCounter;
		for (macroEntryCounter = 0; macroEntryCounter < object->num_entries; macroEntryCounter++) {
			amqp_table_entry_t *entry = &object->entries[macroEntryCounter];

			efree(entry->key.bytes);

			switch (entry->value.kind) {
				case AMQP_FIELD_KIND_TABLE:
					internal_php_amqp_free_amqp_table(&entry->value.value.table, 0);
					break;
				case AMQP_FIELD_KIND_UTF8:
					php_amqp_safe_free_bytes(&entry->value.value.bytes);
					break;
			}
		}
		efree(object->entries);
	}

	if (clear_root) {
		efree(object);
	}
}

void php_amqp_free_amqp_table(amqp_table_t *object)
{
	internal_php_amqp_free_amqp_table(object, 1);
}

PHP_INI_BEGIN()
	PHP_INI_ENTRY("amqp.host",				DEFAULT_HOST,				PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.vhost",				DEFAULT_VHOST,				PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.port",				DEFAULT_PORT,				PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.timeout",			DEFAULT_TIMEOUT,			PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.read_timeout",		DEFAULT_READ_TIMEOUT,		PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.write_timeout",		DEFAULT_WRITE_TIMEOUT,		PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.connect_timeout",	DEFAULT_CONNECT_TIMEOUT,		PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.login",				DEFAULT_LOGIN,				PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.password",			DEFAULT_PASSWORD,			PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.auto_ack",			DEFAULT_AUTOACK,			PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.prefetch_count",	DEFAULT_PREFETCH_COUNT,		PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.channel_max",		DEFAULT_CHANNEL_MAX,		PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.frame_max",			DEFAULT_FRAME_MAX,			PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.heartbeat",			DEFAULT_HEARTBEAT,			PHP_INI_ALL, NULL)
PHP_INI_END()

/* {{{ PHP_MINIT_FUNCTION
*/
PHP_MINIT_FUNCTION(amqp)
{
	zend_class_entry ce;

	/* Set up the connection resource */
	le_amqp_connection_resource = zend_register_list_destructors_ex(amqp_connection_resource_dtor, NULL, PHP_AMQP_CONNECTION_RES_NAME, module_number);
	le_amqp_connection_resource_persistent = zend_register_list_destructors_ex(NULL, amqp_connection_resource_dtor_persistent, PHP_AMQP_CONNECTION_RES_NAME, module_number);

	INIT_CLASS_ENTRY(ce, "AMQPConnection", amqp_connection_class_functions);
	ce.create_object = amqp_connection_ctor;
	amqp_connection_class_entry = zend_register_internal_class(&ce TSRMLS_CC);

	INIT_CLASS_ENTRY(ce, "AMQPChannel", amqp_channel_class_functions);
	ce.create_object = amqp_channel_ctor;
	amqp_channel_class_entry = zend_register_internal_class(&ce TSRMLS_CC);

	INIT_CLASS_ENTRY(ce, "AMQPQueue", amqp_queue_class_functions);
	ce.create_object = amqp_queue_ctor;
	amqp_queue_class_entry = zend_register_internal_class(&ce TSRMLS_CC);

	INIT_CLASS_ENTRY(ce, "AMQPExchange", amqp_exchange_class_functions);
	ce.create_object = amqp_exchange_ctor;
	amqp_exchange_class_entry = zend_register_internal_class(&ce TSRMLS_CC);

	INIT_CLASS_ENTRY(ce, "AMQPEnvelope", amqp_envelope_class_functions);
	ce.create_object = amqp_envelope_ctor;
	amqp_envelope_class_entry = zend_register_internal_class(&ce TSRMLS_CC);

	/* Class Exceptions */
	INIT_CLASS_ENTRY(ce, "AMQPException", NULL);
	amqp_exception_class_entry = zend_register_internal_class_ex(&ce, zend_exception_get_default(TSRMLS_C));

	INIT_CLASS_ENTRY(ce, "AMQPConnectionException", NULL);
	amqp_connection_exception_class_entry = zend_register_internal_class_ex(&ce, amqp_exception_class_entry);

	INIT_CLASS_ENTRY(ce, "AMQPChannelException", NULL);
	amqp_channel_exception_class_entry = zend_register_internal_class_ex(&ce, amqp_exception_class_entry);

	INIT_CLASS_ENTRY(ce, "AMQPQueueException", NULL);
	amqp_queue_exception_class_entry = zend_register_internal_class_ex(&ce, amqp_exception_class_entry);

	INIT_CLASS_ENTRY(ce, "AMQPExchangeException", NULL);
	amqp_exchange_exception_class_entry = zend_register_internal_class_ex(&ce, amqp_exception_class_entry);

	REGISTER_INI_ENTRIES();

	REGISTER_LONG_CONSTANT("AMQP_NOPARAM",			AMQP_NOPARAM,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AMQP_JUST_CONSUME",		AMQP_JUST_CONSUME,	CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AMQP_DURABLE",			AMQP_DURABLE,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AMQP_PASSIVE",			AMQP_PASSIVE,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AMQP_EXCLUSIVE",		AMQP_EXCLUSIVE,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AMQP_AUTODELETE",		AMQP_AUTODELETE,	CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AMQP_INTERNAL",			AMQP_INTERNAL,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AMQP_NOLOCAL",			AMQP_NOLOCAL,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AMQP_AUTOACK",			AMQP_AUTOACK,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AMQP_IFEMPTY",			AMQP_IFEMPTY,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AMQP_IFUNUSED",			AMQP_IFUNUSED,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AMQP_MANDATORY",		AMQP_MANDATORY,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AMQP_IMMEDIATE",		AMQP_IMMEDIATE,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AMQP_MULTIPLE",			AMQP_MULTIPLE,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AMQP_NOWAIT",			AMQP_NOWAIT,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AMQP_REQUEUE",			AMQP_REQUEUE,		CONST_CS | CONST_PERSISTENT);

	REGISTER_STRING_CONSTANT("AMQP_EX_TYPE_DIRECT",	AMQP_EX_TYPE_DIRECT,	CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("AMQP_EX_TYPE_FANOUT",	AMQP_EX_TYPE_FANOUT,	CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("AMQP_EX_TYPE_TOPIC",	AMQP_EX_TYPE_TOPIC,		CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("AMQP_EX_TYPE_HEADERS",AMQP_EX_TYPE_HEADERS,	CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("AMQP_OS_SOCKET_TIMEOUT_ERRNO",	AMQP_OS_SOCKET_TIMEOUT_ERRNO,	CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PHP_AMQP_MAX_CHANNELS",			PHP_AMQP_MAX_CHANNELS,			CONST_CS | CONST_PERSISTENT);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
*/
PHP_MSHUTDOWN_FUNCTION(amqp)
{
	UNREGISTER_INI_ENTRIES();

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
*/
PHP_MINFO_FUNCTION(amqp)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "Version",					PHP_AMQP_VERSION);
	php_info_print_table_header(2, "Revision",					PHP_AMQP_REVISION);
	php_info_print_table_header(2, "Compiled",					__DATE__ " @ "  __TIME__);
	php_info_print_table_header(2, "AMQP protocol version", 	"0-9-1");
	php_info_print_table_header(2, "librabbitmq version", amqp_version());
	php_info_print_table_header(2, "Default max channels per connection",	DEFAULT_CHANNEL_MAX);
	php_info_print_table_header(2, "Default max frame size",	DEFAULT_FRAME_MAX);
	php_info_print_table_header(2, "Default heartbeats interval",	DEFAULT_HEARTBEAT);
	DISPLAY_INI_ENTRIES();
}
/* }}} */

/*
*Local variables:
*tab-width: 4
*c-basic-offset: 4
*End:
*vim600: noet sw=4 ts=4 fdm=marker
*vim<600: noet sw=4 ts=4
*/
