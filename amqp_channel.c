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
	zval *value;
	HashTable *debug_info;

	/* Get the envelope object from which to read */
	amqp_channel_object *channel = (amqp_channel_object *)zend_object_store_get_object(object TSRMLS_CC);

	/* Let zend clean up for us: */
	*is_temp = 1;

	/* Keep the first number matching the number of entries in this table*/
	ALLOC_HASHTABLE(debug_info);
	ZEND_INIT_SYMTABLE_EX(debug_info, 4 + 1, 0);

	/* Start adding values */
	MAKE_STD_ZVAL(value);
	ZVAL_LONG(value, channel->channel_id);
	zend_hash_add(debug_info, "channel_id", sizeof("channel_id"), &value, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(value);
	ZVAL_LONG(value, channel->prefetch_count);
	zend_hash_add(debug_info, "prefetch_count", sizeof("prefetch_count"), &value, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(value);
	ZVAL_LONG(value, channel->prefetch_size);
	zend_hash_add(debug_info, "prefetch_size", sizeof("prefetch_size"), &value, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(value);
	ZVAL_BOOL(value, channel->is_connected);
	zend_hash_add(debug_info, "is_connected", sizeof("is_connected"), &value, sizeof(zval *), NULL);

	/* Start adding values */
	return debug_info;
}

void php_amqp_close_channel(amqp_channel_object *channel TSRMLS_DC)
{
	amqp_connection_object *connection;

	assert(channel != NULL);

	/* Pull out and verify the connection */
	connection = AMQP_GET_CONNECTION(channel);

	if (connection != NULL) {
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

void amqp_channel_dtor(void *object TSRMLS_DC)
{
	amqp_channel_object *channel = (amqp_channel_object*)object;

	if (channel->is_connected) {
		php_amqp_close_channel(channel TSRMLS_CC);
	}

	if (channel->connection != NULL) {
		/* Destroy the connection storage */
		zval_ptr_dtor(&channel->connection);
	}

	zend_object_std_dtor(&channel->zo TSRMLS_CC);

	efree(object);
}

zend_object amqp_channel_ctor(zend_class_entry *ce TSRMLS_DC)
{
	zend_object new_value;
	amqp_channel_object *channel = (amqp_channel_object*)emalloc(sizeof(amqp_channel_object));

	memset(channel, 0, sizeof(amqp_channel_object));

	zend_object_std_init(&channel->zo, ce TSRMLS_CC);
	AMQP_OBJECT_PROPERTIES_INIT(channel->zo, ce);

	new_value.handle = zend_objects_store_put(
		channel,
		(zend_objects_store_dtor_t)zend_objects_destroy_object,
		(zend_objects_free_object_storage_t)amqp_channel_dtor,
		NULL TSRMLS_CC
	);

	memcpy((void *)&amqp_channel_object_handlers, (void *)zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	amqp_channel_object_handlers.get_debug_info = amqp_channel_object_get_debug_info;
	new_value.handlers = &amqp_channel_object_handlers;

	return new_value;
}

/* {{{ proto AMQPChannel::__construct(AMQPConnection obj)
 */
PHP_METHOD(amqp_channel_class, __construct)
{
	zval *id;
	zval *connection_object = NULL;

	amqp_channel_object *channel;
	amqp_connection_object *connection;

	/* Parse out the method parameters */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "OO", &id, amqp_channel_class_entry, &connection_object, amqp_connection_class_entry) == FAILURE) {
		zend_throw_exception(amqp_channel_exception_class_entry, "Parameter must be an instance of AMQPConnection.", 0 TSRMLS_CC);
		RETURN_NULL();
	}

	channel = (amqp_channel_object *)zend_object_store_get_object(id TSRMLS_CC);
	channel->connection = connection_object;

	Z_ADDREF_P(connection_object);

	/* Set the prefetch count */
	channel->prefetch_count = INI_INT("amqp.prefetch_count");

	/* Pull out and verify the connection */
	connection = AMQP_GET_CONNECTION(channel);
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
	zval *id;
	amqp_channel_object *channel;

	/* Try to pull amqp object out of method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_channel_class_entry) == FAILURE) {
		return;
	}

	/* Get the channel object out of the store */
	channel = (amqp_channel_object *)zend_object_store_get_object(id TSRMLS_CC);

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
	zval *id;
	amqp_channel_object *channel;

	/* Try to pull amqp object out of method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_channel_class_entry) == FAILURE) {
		return;
	}

	/* Get the channel object out of the store */
	channel = (amqp_channel_object *)zend_object_store_get_object(id TSRMLS_CC);

	RETURN_LONG(channel->channel_id);
}
/* }}} */

/* {{{ proto bool amqp::setPrefetchCount(long count)
set the number of prefetches */
PHP_METHOD(amqp_channel_class, setPrefetchCount)
{
	zval *id;
	amqp_channel_object *channel;
	amqp_connection_object *connection;
	long prefetch_count;

	/* Get the vhost from the method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ol", &id, amqp_channel_class_entry, &prefetch_count) == FAILURE) {
		return;
	}

	/* Get the channel object out of the store */
	channel = (amqp_channel_object *)zend_object_store_get_object(id TSRMLS_CC);

	connection = AMQP_GET_CONNECTION(channel);
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
	amqp_channel_object *channel;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "")) {
		return;
	}

	channel = (amqp_channel_object *)zend_object_store_get_object(getThis() TSRMLS_CC);

	RETURN_LONG(channel->prefetch_count);
}
/* }}} */

/* {{{ proto bool amqp::setPrefetchSize(long size)
set the number of prefetches */
PHP_METHOD(amqp_channel_class, setPrefetchSize)
{
	zval *id;
	amqp_channel_object *channel;
	amqp_connection_object *connection;
	long prefetch_size;

	/* Get the vhost from the method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ol", &id, amqp_channel_class_entry, &prefetch_size) == FAILURE) {
		return;
	}

	/* Get the channel object out of the store */
	channel = (amqp_channel_object *)zend_object_store_get_object(id TSRMLS_CC);

	connection = AMQP_GET_CONNECTION(channel);
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
	amqp_channel_object *channel;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "")) {
		return;
	}

	/* Get the channel object out of the store */
	channel = (amqp_channel_object *)zend_object_store_get_object(getThis() TSRMLS_CC);

	RETURN_LONG(channel->prefetch_size);
}
/* }}} */

/* {{{ proto amqp::qos(long size, long count)
set the number of prefetches */
PHP_METHOD(amqp_channel_class, qos)
{
	zval *id;
	amqp_channel_object *channel;
	amqp_connection_object *connection;
	long prefetch_size;
	long prefetch_count;

	/* Get the vhost from the method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Oll", &id, amqp_channel_class_entry, &prefetch_size, &prefetch_count) == FAILURE) {
		return;
	}

	/* Get the channel object out of the store */
	channel = (amqp_channel_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Set the prefetch size - the implication is to disable the count */
	channel->prefetch_size = prefetch_size;
	channel->prefetch_count = prefetch_count;

	connection = AMQP_GET_CONNECTION(channel);
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
	zval *id;
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	amqp_rpc_reply_t res;

	/* Get the vhost from the method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_channel_class_entry) == FAILURE) {
		return;
	}

	/* Get the channel object out of the store */
	channel = (amqp_channel_object *)zend_object_store_get_object(id TSRMLS_CC);

	connection = AMQP_GET_CONNECTION(channel);
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
	zval *id;
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	amqp_rpc_reply_t res;

	/* Get the vhost from the method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_channel_class_entry) == FAILURE) {
		return;
	}

	/* Get the channel object out of the store */
	channel = (amqp_channel_object *)zend_object_store_get_object(id TSRMLS_CC);

	connection = AMQP_GET_CONNECTION(channel);
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
	zval *id;
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	amqp_rpc_reply_t res;

	/* Get the vhost from the method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_channel_class_entry) == FAILURE) {
		return;
	}

	/* Get the channel object out of the store */
	channel = (amqp_channel_object *)zend_object_store_get_object(id TSRMLS_CC);

	connection = AMQP_GET_CONNECTION(channel);
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
	zval *id;
	amqp_channel_object *channel;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_channel_class_entry) == FAILURE) {
		return;
	}

	channel = (amqp_channel_object *)zend_object_store_get_object(id TSRMLS_CC);

	RETURN_ZVAL(channel->connection, 1, 0);
}
/* }}} */

/* {{{ proto bool amqp::basicRecover([bool requeue=TRUE])
Redeliver unacknowledged messages */
PHP_METHOD(amqp_channel_class, basicRecover)
{
	zval *id;
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	amqp_rpc_reply_t res;

	zend_bool requeue = 1;

	/* Get the vhost from the method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O|b", &id, amqp_channel_class_entry, &requeue) == FAILURE) {
		return;
	}

	/* Get the channel object out of the store */
	channel = (amqp_channel_object *)zend_object_store_get_object(id TSRMLS_CC);

	connection = AMQP_GET_CONNECTION(channel);
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
