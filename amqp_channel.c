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

#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3
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
	ZEND_INIT_SYMTABLE_EX(debug_info, 3 + 1, 0);

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

	/* Start adding values */
	return debug_info;
}
#endif

void amqp_channel_dtor(void *object TSRMLS_DC)
{
	amqp_channel_object *channel = (amqp_channel_object*)object;
	amqp_connection_object *connection;

	AMQP_ASSIGN_CONNECTION(connection, channel);

	if (connection) {
		remove_channel_from_connection(connection, channel);
	}

	/* Destroy the connection storage */
	if (channel->connection) {
		zval_ptr_dtor(&channel->connection);
	}

	zend_object_std_dtor(&channel->zo TSRMLS_CC);

	efree(object);
}

zend_object_value amqp_channel_ctor(zend_class_entry *ce TSRMLS_DC)
{
	zend_object_value new_value;
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

#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3
	memcpy((void *)&amqp_channel_object_handlers, (void *)zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	amqp_channel_object_handlers.get_debug_info = amqp_channel_object_get_debug_info;
	new_value.handlers = &amqp_channel_object_handlers;
#else
	new_value.handlers = zend_get_std_object_handlers();
#endif

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

	amqp_rpc_reply_t res;

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
	channel->channel_id = get_next_available_channel(connection, channel);

	/* Check that we got a valid channel */
	if (channel->channel_id < 0) {
		zend_throw_exception(amqp_channel_exception_class_entry, "Could not create channel. Connection has no open channel slots remaining.", 0 TSRMLS_CC);
		return;
	}

	/* Open up the channel for use */
	amqp_channel_open(connection->connection_resource->connection_state, channel->channel_id);

	res = AMQP_RPC_REPLY_T_CAST amqp_get_rpc_reply(connection->connection_resource->connection_state);

	if (res.reply_type != AMQP_RESPONSE_NORMAL) {
		char str[256];
		char ** pstr = (char **) &str;
		amqp_error(res, pstr);
		zend_throw_exception(amqp_channel_exception_class_entry, *pstr, 0 TSRMLS_CC);
		amqp_maybe_release_buffers(connection->connection_resource->connection_state);
		return;
	}
	amqp_maybe_release_buffers(connection->connection_resource->connection_state);

	channel->is_connected = '\1';

	/* Set the prefetch count: */
	amqp_basic_qos(
		connection->connection_resource->connection_state,
		channel->channel_id,
		0,							/* prefetch window size */
		channel->prefetch_count,	/* prefetch message count */
		0							/* global flag */
	);
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

	/* Set the prefetch count - the implication is to disable the size */
	channel->prefetch_count = prefetch_count;
	channel->prefetch_size = 0;

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not set prefetch count.");

	/* If we are already connected, set the new prefetch count */
	if (channel->is_connected) {
		amqp_basic_qos(
			connection->connection_resource->connection_state,
			channel->channel_id,
			channel->prefetch_size,
			channel->prefetch_count,
			0
		);
	}

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

	/* Set the prefetch size - the implication is to disable the count */
	channel->prefetch_count = 0;
	channel->prefetch_size = prefetch_size;

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not set prefetch size.");

	/* If we are already connected, set the new prefetch count */
	if (channel->is_connected) {
		amqp_basic_qos(
			connection->connection_resource->connection_state,
			channel->channel_id,
			channel->prefetch_size,
			channel->prefetch_count,
			0
		);
	}

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
			0							/* Global flag - whether this change should affect every channel */
		);
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
	amqp_tx_select_t s;
	amqp_method_number_t select_ok = AMQP_TX_SELECT_OK_METHOD;
	amqp_rpc_reply_t res;


	/* Get the vhost from the method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_channel_class_entry) == FAILURE) {
		return;
	}

	/* Get the channel object out of the store */
	channel = (amqp_channel_object *)zend_object_store_get_object(id TSRMLS_CC);

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not start the transaction.");

	res = AMQP_RPC_REPLY_T_CAST amqp_simple_rpc(
		connection->connection_resource->connection_state,
		channel->channel_id,
		AMQP_TX_SELECT_METHOD,
		&select_ok,
		&s
	);

	if (res.reply_type != AMQP_RESPONSE_NORMAL) {
		char str[256];
		char **pstr = (char **)&str;
		amqp_error(res, pstr);

		channel->is_connected = 0;
		zend_throw_exception(amqp_channel_exception_class_entry, *pstr, 0 TSRMLS_CC);
		amqp_maybe_release_buffers(connection->connection_resource->connection_state);
		return;
	}
	amqp_maybe_release_buffers(connection->connection_resource->connection_state);

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
	amqp_tx_commit_t s;
	amqp_rpc_reply_t res;
	amqp_method_number_t commit_ok = AMQP_TX_COMMIT_OK_METHOD;


	/* Get the vhost from the method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_channel_class_entry) == FAILURE) {
		return;
	}

	/* Get the channel object out of the store */
	channel = (amqp_channel_object *)zend_object_store_get_object(id TSRMLS_CC);

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not start the transaction.");

	res = AMQP_RPC_REPLY_T_CAST amqp_simple_rpc(
		connection->connection_resource->connection_state,
		channel->channel_id,
		AMQP_TX_COMMIT_METHOD,
		&commit_ok,
		&s
	);

	if (res.reply_type != AMQP_RESPONSE_NORMAL) {
		char str[256];
		char **pstr = (char **)&str;
		amqp_error(res, pstr);

		channel->is_connected = 0;
		zend_throw_exception(amqp_channel_exception_class_entry, *pstr, 0 TSRMLS_CC);
		amqp_maybe_release_buffers(connection->connection_resource->connection_state);
		return;
	}
	amqp_maybe_release_buffers(connection->connection_resource->connection_state);

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
	amqp_tx_rollback_t s;
	amqp_method_number_t rollback_ok = AMQP_TX_ROLLBACK_OK_METHOD;
	amqp_rpc_reply_t res;


	/* Get the vhost from the method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_channel_class_entry) == FAILURE) {
		return;
	}

	/* Get the channel object out of the store */
	channel = (amqp_channel_object *)zend_object_store_get_object(id TSRMLS_CC);

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not start the transaction.");

	res = AMQP_RPC_REPLY_T_CAST amqp_simple_rpc(
		connection->connection_resource->connection_state,
		channel->channel_id,
		AMQP_TX_ROLLBACK_METHOD,
		&rollback_ok,
		&s
	);

	if (res.reply_type != AMQP_RESPONSE_NORMAL) {
		char str[256];
		char **pstr = (char **)&str;
		amqp_error(res, pstr);

		channel->is_connected = 0;
		zend_throw_exception(amqp_channel_exception_class_entry, *pstr, 0 TSRMLS_CC);
		amqp_maybe_release_buffers(connection->connection_resource->connection_state);
		return;
	}
	amqp_maybe_release_buffers(connection->connection_resource->connection_state);

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
