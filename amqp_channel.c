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
#include "amqp_connection_resource.h"

zend_object_handlers amqp_channel_object_handlers;

HashTable *amqp_channel_object_get_debug_info(zval *object, int *is_temp TSRMLS_DC) {
	zval value;
	HashTable *debug_info;

	/* Get the envelope object from which to read */
	amqp_channel_object *channel = AMQP_CHANNEL_OBJ_P(object);

	/* Let zend clean up for us: */
	*is_temp = 1;

	/* Keep the first number matching the number of entries in this table*/
	ALLOC_HASHTABLE(debug_info);
	ZEND_INIT_SYMTABLE_EX(debug_info, 4 + 1, 0);

	/* Start adding values */
	ZVAL_LONG(&value, channel->channel_id);
	zend_hash_str_add(debug_info, "channel_id", sizeof("channel_id"), &value);

	ZVAL_LONG(&value, channel->prefetch_count);
	zend_hash_str_add(debug_info, "prefetch_count", sizeof("prefetch_count"), &value);

	ZVAL_LONG(&value, channel->prefetch_size);
	zend_hash_str_add(debug_info, "prefetch_size", sizeof("prefetch_size"), &value);

	ZVAL_BOOL(&value, channel->is_connected);
	zend_hash_str_add(debug_info, "is_connected", sizeof("is_connected"), &value);

	ZVAL_COPY(&value, &channel->connection);
	zend_hash_str_add(debug_info, "connection", sizeof("connection"), &value);

	/* Start adding values */
	return debug_info;
}

void php_amqp_close_channel(amqp_channel_object *channel TSRMLS_DC)
{
	amqp_connection_object *connection;

	assert(channel != NULL);

	/* Pull out and verify the connection */
	connection = Z_AMQP_CONNECTION_OBJ(channel->connection);

	// TODO
	if (connection) {
        /* First, remove it from active channels table to prevent recursion in case of connection error */
        php_amqp_connection_resource_unregister_channel(connection->connection_resource, channel->channel_id);
	} else {
	    channel->is_connected = '\0';
	}

	if (!channel->is_connected) {
		/* Nothing to do more - channel was previously marked as closed, possibly, due to channel-level error */
		return;
	}

	channel->is_connected = '\0';

	if (connection->connection_resource && connection->connection_resource->is_connected) {
		assert(connection->connection_resource != NULL);

		amqp_channel_close(connection->connection_resource->connection_state, channel->channel_id, AMQP_REPLY_SUCCESS);

		amqp_rpc_reply_t res = amqp_get_rpc_reply(connection->connection_resource->connection_state);

		if (res.reply_type != AMQP_RESPONSE_NORMAL) {
			PHP_AMQP_INIT_ERROR_MESSAGE();

			php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

			php_amqp_zend_throw_exception(res, amqp_channel_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
			php_amqp_maybe_release_buffers_on_channel(connection, channel);

			PHP_AMQP_DESTROY_ERROR_MESSAGE();
			return;
		}

		php_amqp_maybe_release_buffers_on_channel(connection, channel);
	}
}

void amqp_channel_free_obj(zend_object *object TSRMLS_DC)
{
	amqp_channel_object *channel = amqp_channel_object_fetch_object(object);

	if (channel->is_connected) {
		php_amqp_close_channel(channel TSRMLS_CC);
	}

	/* Destroy the connection storage */
	zend_object_release(Z_OBJ(channel->connection));

	zend_object_std_dtor(&channel->zo TSRMLS_CC);

	efree(channel);
}

zend_object* amqp_channel_ctor(zend_class_entry *ce TSRMLS_DC)
{
	amqp_channel_object *channel = ecalloc(1,
										   sizeof(amqp_channel_object)
											+ zend_object_properties_size(ce));

	zend_object_std_init(&channel->zo, ce TSRMLS_CC);
	object_properties_init(&channel->zo, ce);

	memcpy((void *)&amqp_channel_object_handlers, (void *)zend_get_std_object_handlers(), sizeof(zend_object_handlers));

	amqp_channel_object_handlers.get_debug_info = amqp_channel_object_get_debug_info;
	amqp_channel_object_handlers.offset = XtOffsetOf(amqp_channel_object, zo);
	amqp_channel_object_handlers.free_obj = amqp_channel_free_obj;

	channel->zo.handlers = &amqp_channel_object_handlers;

	return &channel->zo;
}

/* {{{ proto AMQPChannel::__construct(AMQPConnection obj)
 */
PHP_METHOD(amqp_channel_class, __construct)
{
	amqp_channel_object *channel = AMQP_CHANNEL_OBJ_P(getThis());
	amqp_connection_object *connection;
	zval* connection_param;

	/* Parse out the method parameters */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &connection_param, amqp_connection_class_entry) == FAILURE) {
		zend_throw_exception(amqp_channel_exception_class_entry, "Parameter must be an instance of AMQPConnection.", 0 TSRMLS_CC);
		RETURN_NULL();
	}

	ZVAL_COPY(&channel->connection, connection_param);

	connection = Z_AMQP_CONNECTION_OBJ(channel->connection);

	/* Set the prefetch count */
	channel->prefetch_count = INI_INT("amqp.prefetch_count");

	/* Pull out and verify the connection */
	AMQP_VERIFY_CONNECTION(connection, "Could not create channel.");

	/* Figure out what the next available channel is on this connection */
	channel->channel_id = php_amqp_connection_resource_get_available_channel_id(connection->connection_resource);

	/* Check that we got a valid channel */
	if (!channel->channel_id) {
		zend_throw_exception(amqp_channel_exception_class_entry, "Could not create channel. Connection has no open channel slots remaining.", 0 TSRMLS_CC);
		return;
	}

	if (php_amqp_connection_resource_register_channel(connection->connection_resource, channel, channel->channel_id) == FAILURE) {
		zend_throw_exception(amqp_channel_exception_class_entry, "Could not create channel. Failed to add channel to connection slot.", 0 TSRMLS_CC);
	}

	/* Open up the channel for use */
	amqp_channel_open_ok_t *r = amqp_channel_open(connection->connection_resource->connection_state, channel->channel_id);

	if (!r) {
		amqp_rpc_reply_t res = amqp_get_rpc_reply(connection->connection_resource->connection_state);

		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_channel_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		PHP_AMQP_DESTROY_ERROR_MESSAGE();

		php_amqp_connection_resource_unregister_channel(connection->connection_resource, channel->channel_id);
		channel->channel_id = 0;
		return;
	}

	/* r->channel_id is a 16-bit channel number insibe amqp_bytes_t, assertion below will without converting to uint16_t*/
	/* assert (r->channel_id == channel->channel_id);*/
	php_amqp_maybe_release_buffers_on_channel(connection, channel);

	channel->is_connected = '\1';

	/* Set the prefetch count: */
	amqp_basic_qos(
		connection->connection_resource->connection_state,
		channel->channel_id,
		0,							/* prefetch window size */
		channel->prefetch_count,	/* prefetch message count */
		/* NOTE that RabbitMQ has reinterpreted global flag field. See https://www.rabbitmq.com/amqp-0-9-1-reference.html#basic.qos.global for details */
		0							/* global flag */
	);

	amqp_rpc_reply_t res = amqp_get_rpc_reply(connection->connection_resource->connection_state);

	if (res.reply_type != AMQP_RESPONSE_NORMAL) {
		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_channel_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		PHP_AMQP_DESTROY_ERROR_MESSAGE();
		return;
	}

	php_amqp_maybe_release_buffers_on_channel(connection, channel);
}
/* }}} */


/* {{{ proto bool amqp::isConnected()
check amqp channel */
PHP_METHOD(amqp_channel_class, isConnected)
{
	amqp_channel_object *channel = AMQP_CHANNEL_OBJ_P(getThis());

	/* Parse out the method parameters */
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	/* If the channel_connect is 1, we have a channel */
	if (channel->is_connected == '\1') {
		RETURN_TRUE;
	}

	/* We have no channel */
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto bool amqp::getChannelId()
get amqp channel ID */
PHP_METHOD(amqp_channel_class, getChannelId)
{
	amqp_channel_object *channel = AMQP_CHANNEL_OBJ_P(getThis());

	/* Parse out the method parameters */
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_LONG(channel->channel_id);
}
/* }}} */

/* {{{ proto bool amqp::setPrefetchCount(long count)
set the number of prefetches */
PHP_METHOD(amqp_channel_class, setPrefetchCount)
{
	amqp_channel_object *channel = AMQP_CHANNEL_OBJ_P(getThis());
	amqp_connection_object *connection = Z_AMQP_CONNECTION_OBJ(channel->connection);
	zend_long prefetch_count;

	/* Parse out the method parameters */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &prefetch_count) == FAILURE) {
		return;
	}

	AMQP_VERIFY_CONNECTION(connection, "Could not set prefetch count.");

	/* If we are already connected, set the new prefetch count */
	if (channel->is_connected) {
		amqp_basic_qos(
			connection->connection_resource->connection_state,
			channel->channel_id,
			0,
			prefetch_count,
			0
		);

		amqp_rpc_reply_t res = amqp_get_rpc_reply(connection->connection_resource->connection_state);

		if (res.reply_type != AMQP_RESPONSE_NORMAL) {
			PHP_AMQP_INIT_ERROR_MESSAGE();

			php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

			php_amqp_zend_throw_exception(res, amqp_channel_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
			php_amqp_maybe_release_buffers_on_channel(connection, channel);

			PHP_AMQP_DESTROY_ERROR_MESSAGE();
			return;
		}

		php_amqp_maybe_release_buffers_on_channel(connection, channel);
	}

	/* Set the prefetch count - the implication is to disable the size */
	channel->prefetch_count = prefetch_count;
	channel->prefetch_size = 0;

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto long amqp::setPrefetchCount()
get the number of prefetches */
PHP_METHOD(amqp_channel_class, getPrefetchCount)
{
	amqp_channel_object *channel = AMQP_CHANNEL_OBJ_P(getThis());

	/* Parse out the method parameters */
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_LONG(channel->prefetch_count);
}
/* }}} */

/* {{{ proto bool amqp::setPrefetchSize(long size)
set the number of prefetches */
PHP_METHOD(amqp_channel_class, setPrefetchSize)
{
	amqp_channel_object *channel = AMQP_CHANNEL_OBJ_P(getThis());
	amqp_connection_object *connection;
	zend_long prefetch_size;

	/* Parse out the method parameters */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &prefetch_size) == FAILURE) {
		return;
	}

	connection = Z_AMQP_CONNECTION_OBJ(channel->connection);

	AMQP_VERIFY_CONNECTION(connection, "Could not set prefetch size.");

	/* If we are already connected, set the new prefetch count */
	if (channel->is_connected) {
		amqp_basic_qos(
			connection->connection_resource->connection_state,
			channel->channel_id,
			prefetch_size,
			0,
			0
		);

		amqp_rpc_reply_t res = amqp_get_rpc_reply(connection->connection_resource->connection_state);

		if (res.reply_type != AMQP_RESPONSE_NORMAL) {
			PHP_AMQP_INIT_ERROR_MESSAGE();

			php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

			php_amqp_zend_throw_exception(res, amqp_channel_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
			php_amqp_maybe_release_buffers_on_channel(connection, channel);

			PHP_AMQP_DESTROY_ERROR_MESSAGE();
			return;
		}

		php_amqp_maybe_release_buffers_on_channel(connection, channel);
	}

	/* Set the prefetch size - the implication is to disable the count */
	channel->prefetch_count = 0;
	channel->prefetch_size = prefetch_size;

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto long amqp::getPrefetchSize()
get the number of prefetches */
PHP_METHOD(amqp_channel_class, getPrefetchSize)
{
	amqp_channel_object *channel = AMQP_CHANNEL_OBJ_P(getThis());

	/* Parse out the method parameters */
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_LONG(channel->prefetch_size);
}
/* }}} */

/* {{{ proto amqp::qos(long size, long count)
set the number of prefetches */
PHP_METHOD(amqp_channel_class, qos)
{
	amqp_channel_object *channel = AMQP_CHANNEL_OBJ_P(getThis());
	amqp_connection_object *connection;
	zend_long prefetch_size;
	zend_long prefetch_count;

	/* Parse out the method parameters */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &prefetch_size, &prefetch_count) == FAILURE) {
		return;
	}

	/* Set the prefetch size - the implication is to disable the count */
	channel->prefetch_size = prefetch_size;
	channel->prefetch_count = prefetch_count;

	connection = Z_AMQP_CONNECTION_OBJ(channel->connection);

	AMQP_VERIFY_CONNECTION(connection, "Could not set qos parameters.");

	/* If we are already connected, set the new prefetch count */
	if (channel->is_connected) {
		amqp_basic_qos(
			connection->connection_resource->connection_state,
			channel->channel_id,
			channel->prefetch_size,
			channel->prefetch_count,
			/* NOTE that RabbitMQ has reinterpreted global flag field. See https://www.rabbitmq.com/amqp-0-9-1-reference.html#basic.qos.global for details */
			0							/* Global flag - whether this change should affect every channel */
		);

		amqp_rpc_reply_t res = amqp_get_rpc_reply(connection->connection_resource->connection_state);

		if (res.reply_type != AMQP_RESPONSE_NORMAL) {
			PHP_AMQP_INIT_ERROR_MESSAGE();

			php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

			php_amqp_zend_throw_exception(res, amqp_channel_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
			php_amqp_maybe_release_buffers_on_channel(connection, channel);

			PHP_AMQP_DESTROY_ERROR_MESSAGE();
			return;
		}

		php_amqp_maybe_release_buffers_on_channel(connection, channel);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto amqp::startTransaction()
start a transaction on the given channel */
PHP_METHOD(amqp_channel_class, startTransaction)
{
	amqp_channel_object *channel = AMQP_CHANNEL_OBJ_P(getThis());
	amqp_connection_object *connection;
	amqp_rpc_reply_t res;

	/* Parse out the method parameters */
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	connection = Z_AMQP_CONNECTION_OBJ(channel->connection);

	AMQP_VERIFY_CONNECTION(connection, "Could not start the transaction.");

	amqp_tx_select(
		connection->connection_resource->connection_state,
		channel->channel_id
	);

	res = amqp_get_rpc_reply(connection->connection_resource->connection_state);

	if (res.reply_type != AMQP_RESPONSE_NORMAL) {
		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_channel_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		PHP_AMQP_DESTROY_ERROR_MESSAGE();
		return;
	}

	php_amqp_maybe_release_buffers_on_channel(connection, channel);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto amqp::startTransaction()
start a transaction on the given channel */
PHP_METHOD(amqp_channel_class, commitTransaction)
{
	amqp_channel_object *channel = AMQP_CHANNEL_OBJ_P(getThis());
	amqp_connection_object *connection;
	amqp_rpc_reply_t res;

	/* Parse out the method parameters */
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	connection = Z_AMQP_CONNECTION_OBJ(channel->connection);

	AMQP_VERIFY_CONNECTION(connection, "Could not start the transaction.");

	amqp_tx_commit(
		connection->connection_resource->connection_state,
		channel->channel_id
	);

	res = amqp_get_rpc_reply(connection->connection_resource->connection_state);

	if (res.reply_type != AMQP_RESPONSE_NORMAL) {
		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_channel_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		PHP_AMQP_DESTROY_ERROR_MESSAGE();
		return;
	}

	php_amqp_maybe_release_buffers_on_channel(connection, channel);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto amqp::startTransaction()
start a transaction on the given channel */
PHP_METHOD(amqp_channel_class, rollbackTransaction)
{
	amqp_channel_object *channel = AMQP_CHANNEL_OBJ_P(getThis());
	amqp_connection_object *connection = Z_AMQP_CONNECTION_OBJ(channel->connection);
	amqp_rpc_reply_t res;

	/* Parse out the method parameters */
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	AMQP_VERIFY_CONNECTION(connection, "Could not start the transaction.");

	amqp_tx_rollback(
		connection->connection_resource->connection_state,
		channel->channel_id
	);

	res = amqp_get_rpc_reply(connection->connection_resource->connection_state);

	if (res.reply_type != AMQP_RESPONSE_NORMAL) {
		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_channel_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		PHP_AMQP_DESTROY_ERROR_MESSAGE();
		return;
	}

	php_amqp_maybe_release_buffers_on_channel(connection, channel);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto AMQPChannel::getConnection()
Get the AMQPConnection object in use */
PHP_METHOD(amqp_channel_class, getConnection)
{
	amqp_channel_object *channel = AMQP_CHANNEL_OBJ_P(getThis());

	/* Parse out the method parameters */
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_ZVAL(&channel->connection, 1, 0);
}
/* }}} */

/* {{{ proto bool amqp::basicRecover([bool requeue=TRUE])
Redeliver unacknowledged messages */
PHP_METHOD(amqp_channel_class, basicRecover)
{
	amqp_channel_object *channel = AMQP_CHANNEL_OBJ_P(getThis());
	amqp_connection_object *connection;
	amqp_rpc_reply_t res;
	zend_bool requeue = 1;

	/* Parse out the method parameters */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|b", &requeue) == FAILURE) {
		return;
	}

	connection = Z_AMQP_CONNECTION_OBJ(channel->connection);

	AMQP_VERIFY_CONNECTION(connection, "Could not redeliver unacknowledged messages.");

	amqp_basic_recover(
		connection->connection_resource->connection_state,
		channel->channel_id,
		(amqp_boolean_t) requeue
	);

	res = amqp_get_rpc_reply(connection->connection_resource->connection_state);

	if (res.reply_type != AMQP_RESPONSE_NORMAL) {
		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_channel_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		PHP_AMQP_DESTROY_ERROR_MESSAGE();
		return;
	}

	php_amqp_maybe_release_buffers_on_channel(connection, channel);

	RETURN_TRUE;
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
