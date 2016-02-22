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

/* $Id: amqp_channel.c 318036 2011-10-11 20:30:46Z pdezwart $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
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

#ifdef PHP_WIN32
# include "win32/unistd.h"
#else
# include <unistd.h>
#endif

#include "php_amqp.h"
#include "amqp_connection.h"
#include "amqp_connection_resource.h"
#include "amqp_channel.h"

zend_class_entry *amqp_channel_class_entry;
#define this_ce amqp_channel_class_entry

zend_object_handlers amqp_channel_object_handlers;

void php_amqp_close_channel(amqp_channel_resource *channel_resource TSRMLS_DC)
{
	assert(channel_resource != NULL);

	amqp_connection_resource *connection_resource = channel_resource->connection_resource;

	if (connection_resource != NULL) {
		/* First, remove it from active channels table to prevent recursion in case of connection error */
        php_amqp_connection_resource_unregister_channel(connection_resource, channel_resource->channel_id);
	} else {
	    channel_resource->is_connected = '\0';
	}

	assert(channel_resource->connection_resource == NULL);

	if (!channel_resource->is_connected) {
		/* Nothing to do more - channel was previously marked as closed, possibly, due to channel-level error */
		return;
	}

	channel_resource->is_connected = '\0';

	if (connection_resource && connection_resource->is_connected && channel_resource->channel_id > 0) {
		assert(connection_resource != NULL);

		amqp_channel_close(connection_resource->connection_state, channel_resource->channel_id, AMQP_REPLY_SUCCESS);

		amqp_rpc_reply_t res = amqp_get_rpc_reply(connection_resource->connection_state);

		if (PHP_AMQP_MAYBE_ERROR(res, channel_resource)) {
			php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry TSRMLS_CC);
			php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
			return;
		}

		php_amqp_maybe_release_buffers_on_channel(connection_resource, channel_resource);
	}
}

void amqp_channel_free(PHP5to7_obj_free_zend_object *object TSRMLS_DC)
{
	amqp_channel_object *channel = PHP_AMQP_FETCH_CHANNEL(object);

	if (channel->channel_resource != NULL) {
		php_amqp_close_channel(channel->channel_resource TSRMLS_CC);

		efree(channel->channel_resource);
		channel->channel_resource = NULL;
	}

	zend_object_std_dtor(&channel->zo TSRMLS_CC);

#if PHP_MAJOR_VERSION < 7
	efree(object);
#endif
}


PHP5to7_zend_object_value amqp_channel_ctor(zend_class_entry *ce TSRMLS_DC)
{
	amqp_channel_object *channel = PHP5to7_ECALLOC_CHANNEL_OBJECT(ce);

	zend_object_std_init(&channel->zo, ce TSRMLS_CC);
	AMQP_OBJECT_PROPERTIES_INIT(channel->zo, ce);

#if PHP_MAJOR_VERSION >=7
	channel->zo.handlers = &amqp_channel_object_handlers;

	return &channel->zo;
#else
	PHP5to7_zend_object_value new_value;

	new_value.handle = zend_objects_store_put(
			channel,
			NULL,
			(zend_objects_free_object_storage_t) amqp_channel_free,
			NULL TSRMLS_CC
	);

	new_value.handlers = zend_get_std_object_handlers();

	return new_value;
#endif
}


/* {{{ proto AMQPChannel::__construct(AMQPConnection obj)
 */
static PHP_METHOD(amqp_channel_class, __construct)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;

	zval *connection_object = NULL;

	amqp_channel_resource *channel_resource;
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	/* Parse out the method parameters */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &connection_object) == FAILURE) {
		zend_throw_exception(amqp_channel_exception_class_entry, "Parameter must be an instance of AMQPConnection.", 0 TSRMLS_CC);
		RETURN_NULL();
	}

	channel = PHP_AMQP_GET_CHANNEL(getThis());

	/* Set the prefetch count */
	zend_update_property_long(this_ce, getThis(), ZEND_STRL("prefetch_count"), INI_INT("amqp.prefetch_count") TSRMLS_CC);

	/* Pull out and verify the connection */
	connection = PHP_AMQP_GET_CONNECTION(connection_object);
	PHP_AMQP_VERIFY_CONNECTION(connection, "Could not create channel.");

	if (!connection->connection_resource) {
		zend_throw_exception(amqp_channel_exception_class_entry, "Could not create channel. No connection resource.", 0 TSRMLS_CC);
		return;
	}

	if (!connection->connection_resource->is_connected) {
		zend_throw_exception(amqp_channel_exception_class_entry, "Could not create channel. Connection resource is not connected.", 0 TSRMLS_CC);
		return;
	}

	zend_update_property(this_ce, getThis(), ZEND_STRL("connection"), connection_object TSRMLS_CC);

	channel_resource = (amqp_channel_resource*)ecalloc(1, sizeof(amqp_channel_resource));
	channel->channel_resource = channel_resource;

	/* Figure out what the next available channel is on this connection */
	channel_resource->channel_id = php_amqp_connection_resource_get_available_channel_id(connection->connection_resource);

	/* Check that we got a valid channel */
	if (!channel_resource->channel_id) {
		zend_throw_exception(amqp_channel_exception_class_entry, "Could not create channel. Connection has no open channel slots remaining.", 0 TSRMLS_CC);
		return;
	}

	if (php_amqp_connection_resource_register_channel(connection->connection_resource, channel_resource, channel_resource->channel_id) == FAILURE) {
		zend_throw_exception(amqp_channel_exception_class_entry, "Could not create channel. Failed to add channel to connection slot.", 0 TSRMLS_CC);
	}

	/* Open up the channel for use */
	amqp_channel_open_ok_t *r = amqp_channel_open(channel_resource->connection_resource->connection_state, channel_resource->channel_id);


	if (!r) {
		amqp_rpc_reply_t res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

		php_amqp_error(res, &PHP_AMQP_G(error_message), channel_resource->connection_resource, channel_resource TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_channel_exception_class_entry, PHP_AMQP_G(error_message), 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

		/* Prevent double free, it may happens in case case channel resource was already freed due to some hard error. */
		if (channel_resource->connection_resource) {
			php_amqp_connection_resource_unregister_channel(channel_resource->connection_resource, channel_resource->channel_id);
			channel_resource->channel_id = 0;
		}

		return;
	}

	/* r->channel_id is a 16-bit channel number insibe amqp_bytes_t, assertion below will without converting to uint16_t*/
	/* assert (r->channel_id == channel_resource->channel_id);*/
	php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

	channel_resource->is_connected = '\1';

	/* Set the prefetch count: */
	amqp_basic_qos(
		channel_resource->connection_resource->connection_state,
		channel_resource->channel_id,
		0,							/* prefetch window size */
		(uint16_t)PHP_AMQP_READ_THIS_PROP_LONG("prefetch_count"),	/* prefetch message count */
		/* NOTE that RabbitMQ has reinterpreted global flag field. See https://www.rabbitmq.com/amqp-0-9-1-reference.html#basic.qos.global for details */
		0							/* global flag */
	);

	amqp_rpc_reply_t res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

	if (PHP_AMQP_MAYBE_ERROR(res, channel_resource)) {
		php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
		return;
	}

	php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
}
/* }}} */


/* {{{ proto bool amqp::isConnected()
check amqp channel */
static PHP_METHOD(amqp_channel_class, isConnected)
{
	amqp_channel_resource *channel_resource;

	PHP_AMQP_NOPARAMS();

	channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(getThis());

	RETURN_BOOL(channel_resource && channel_resource->is_connected);
}
/* }}} */

/* {{{ proto bool amqp::getChannelId()
get amqp channel ID */
static PHP_METHOD(amqp_channel_class, getChannelId)
{
	amqp_channel_resource *channel_resource;

	PHP_AMQP_NOPARAMS();

	channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(getThis());

	if (!channel_resource) {
		RETURN_NULL();
	}

	RETURN_LONG(channel_resource->channel_id);
}
/* }}} */

/* {{{ proto bool amqp::setPrefetchCount(long count)
set the number of prefetches */
static PHP_METHOD(amqp_channel_class, setPrefetchCount)
{
	amqp_channel_resource *channel_resource;
	PHP5to7_param_long_type_t prefetch_count;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &prefetch_count) == FAILURE) {
		return;
	}

	channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(getThis());
	PHP_AMQP_VERIFY_CHANNEL_CONNECTION_RESOURCE(channel_resource, "Could not set prefetch count.");
	// TODO: verify that connection is active and resource exists. that is enough

	/* If we are already connected, set the new prefetch count */
	if (channel_resource->is_connected) {
		amqp_basic_qos(
			channel_resource->connection_resource->connection_state,
			channel_resource->channel_id,
			0,
			(uint16_t)prefetch_count,
			0
		);

		amqp_rpc_reply_t res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

		if (PHP_AMQP_MAYBE_ERROR(res, channel_resource)) {
			php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry TSRMLS_CC);
			php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
			return;
		}

		php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
	}

	/* Set the prefetch count - the implication is to disable the size */
	zend_update_property_long(this_ce, getThis(), ZEND_STRL("prefetch_count"), prefetch_count TSRMLS_CC);
	zend_update_property_long(this_ce, getThis(), ZEND_STRL("prefetch_size"), 0 TSRMLS_CC);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto long amqp::setPrefetchCount()
get the number of prefetches */
static PHP_METHOD(amqp_channel_class, getPrefetchCount)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;
	PHP_AMQP_NOPARAMS();
	PHP_AMQP_RETURN_THIS_PROP("prefetch_count")
}
/* }}} */


/* {{{ proto bool amqp::setPrefetchSize(long size)
set the number of prefetches */
static PHP_METHOD(amqp_channel_class, setPrefetchSize)
{
	amqp_channel_resource *channel_resource;
	PHP5to7_param_long_type_t prefetch_size;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &prefetch_size) == FAILURE) {
		return;
	}

	channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(getThis());
	PHP_AMQP_VERIFY_CHANNEL_CONNECTION_RESOURCE(channel_resource, "Could not set prefetch size.");

	/* If we are already connected, set the new prefetch count */
	if (channel_resource->is_connected) {
		amqp_basic_qos(
			channel_resource->connection_resource->connection_state,
			channel_resource->channel_id,
			(uint16_t)prefetch_size,
			0,
			0
		);

		amqp_rpc_reply_t res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

		if (PHP_AMQP_MAYBE_ERROR(res, channel_resource)) {
			php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry TSRMLS_CC);
			php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
			return;
		}

		php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
	}

	/* Set the prefetch size - the implication is to disable the count */
	zend_update_property_long(this_ce, getThis(), ZEND_STRL("prefetch_count"), 0 TSRMLS_CC);
	zend_update_property_long(this_ce, getThis(), ZEND_STRL("prefetch_size"), prefetch_size TSRMLS_CC);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto long amqp::getPrefetchSize()
get the number of prefetches */
static PHP_METHOD(amqp_channel_class, getPrefetchSize)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;
	PHP_AMQP_NOPARAMS();
	PHP_AMQP_RETURN_THIS_PROP("prefetch_size")
}
/* }}} */



/* {{{ proto amqp::qos(long size, long count)
set the number of prefetches */
static PHP_METHOD(amqp_channel_class, qos)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;

	amqp_channel_resource *channel_resource;
	PHP5to7_param_long_type_t prefetch_size;
	PHP5to7_param_long_type_t prefetch_count;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &prefetch_size, &prefetch_count) == FAILURE) {
		return;
	}

	channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(getThis());
	PHP_AMQP_VERIFY_CHANNEL_CONNECTION_RESOURCE(channel_resource, "Could not set qos parameters.");

	/* Set the prefetch size - the implication is to disable the count */
	zend_update_property_long(this_ce, getThis(), ZEND_STRL("prefetch_size"), prefetch_size TSRMLS_CC);
	zend_update_property_long(this_ce, getThis(), ZEND_STRL("prefetch_count"), prefetch_count TSRMLS_CC);

	/* If we are already connected, set the new prefetch count */
	if (channel_resource->is_connected) {
		amqp_basic_qos(
			channel_resource->connection_resource->connection_state,
			channel_resource->channel_id,
			(uint16_t)PHP_AMQP_READ_THIS_PROP_LONG("prefetch_size"),
			(uint16_t)PHP_AMQP_READ_THIS_PROP_LONG("prefetch_count"),
			/* NOTE that RabbitMQ has reinterpreted global flag field. See https://www.rabbitmq.com/amqp-0-9-1-reference.html#basic.qos.global for details */
			0							/* Global flag - whether this change should affect every channel_resource */
		);

		amqp_rpc_reply_t res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

		if (PHP_AMQP_MAYBE_ERROR(res, channel_resource)) {
			php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry TSRMLS_CC);
			php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
			return;
		}

		php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
	}

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto amqp::startTransaction()
start a transaction on the given channel */
static PHP_METHOD(amqp_channel_class, startTransaction)
{
	amqp_channel_resource *channel_resource;

	amqp_rpc_reply_t res;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
		return;
	}

	channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(getThis());
	PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not start the transaction.");

	amqp_tx_select(
		channel_resource->connection_resource->connection_state,
		channel_resource->channel_id
	);

	res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

	if (PHP_AMQP_MAYBE_ERROR(res, channel_resource)) {
		php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
		return;
	}

	php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto amqp::startTransaction()
start a transaction on the given channel */
static PHP_METHOD(amqp_channel_class, commitTransaction)
{
	amqp_channel_resource *channel_resource;

	amqp_rpc_reply_t res;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
		return;
	}

	channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(getThis());
	PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not start the transaction.");

	amqp_tx_commit(
		channel_resource->connection_resource->connection_state,
		channel_resource->channel_id
	);

	res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

	if (PHP_AMQP_MAYBE_ERROR(res, channel_resource)) {
		php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
		return;
	}

	php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto amqp::startTransaction()
start a transaction on the given channel */
static PHP_METHOD(amqp_channel_class, rollbackTransaction)
{
	amqp_channel_resource *channel_resource;

	amqp_rpc_reply_t res;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
		return;
	}

	
	channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(getThis());
	PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not rollback the transaction.");

	amqp_tx_rollback(
		channel_resource->connection_resource->connection_state,
		channel_resource->channel_id
	);

	res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

	if (PHP_AMQP_MAYBE_ERROR(res, channel_resource)) {
		php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
		return;
	}

	php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto AMQPChannel::getConnection()
Get the AMQPConnection object in use */
static PHP_METHOD(amqp_channel_class, getConnection)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;
	PHP_AMQP_NOPARAMS();
	PHP_AMQP_RETURN_THIS_PROP("connection")
}
/* }}} */

/* {{{ proto bool amqp::basicRecover([bool requeue=TRUE])
Redeliver unacknowledged messages */
static PHP_METHOD(amqp_channel_class, basicRecover)
{
	amqp_channel_resource *channel_resource;

	amqp_rpc_reply_t res;

	zend_bool requeue = 1;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|b", &requeue) == FAILURE) {
		return;
	}

	channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(getThis());
	PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not redeliver unacknowledged messages.");

	amqp_basic_recover(
		channel_resource->connection_resource->connection_state,
		channel_resource->channel_id,
		(amqp_boolean_t) requeue
	);

	res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

	if (PHP_AMQP_MAYBE_ERROR(res, channel_resource)) {
		php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
		return;
	}

	php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

	RETURN_TRUE;
}
/* }}} */


/* amqp_channel_class ARG_INFO definition */
ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class__construct, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
				ZEND_ARG_OBJ_INFO(0, amqp_connection, AMQPConnection, 0)
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

		{NULL, NULL, NULL}
};

PHP_MINIT_FUNCTION(amqp_channel)
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "AMQPChannel", amqp_channel_class_functions);
	ce.create_object = amqp_channel_ctor;
	amqp_channel_class_entry = zend_register_internal_class(&ce TSRMLS_CC);

	zend_declare_property_null(this_ce, ZEND_STRL("connection"), ZEND_ACC_PRIVATE TSRMLS_CC);

	zend_declare_property_null(this_ce, ZEND_STRL("prefetch_count"), ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(this_ce, ZEND_STRL("prefetch_size"), 0, ZEND_ACC_PRIVATE TSRMLS_CC);

#if PHP_MAJOR_VERSION >=7
	memcpy(&amqp_channel_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

	amqp_channel_object_handlers.offset = XtOffsetOf(amqp_connection_object, zo);
	amqp_channel_object_handlers.free_obj = amqp_channel_free;
#endif

	return SUCCESS;
}
/*
*Local variables:
*tab-width: 4
*c-basic-offset: 4
*End:
*vim600: noet sw=4 ts=4 fdm=marker
*vim<600: noet sw=4 ts=4
*/
