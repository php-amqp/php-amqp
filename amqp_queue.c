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

/* $Id: amqp_queue.c 327551 2012-09-09 03:49:34Z pdezwart $ */

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
# include <signal.h>
# include <stdint.h>
#endif
#include <amqp.h>
#include <amqp_framing.h>

#ifdef PHP_WIN32
# include "win32/unistd.h"
#else
# include <unistd.h>
#endif

#include "php_amqp.h"
#include "amqp_envelope.h"
#include "amqp_queue.h"

zend_class_entry *amqp_queue_class_entry;
#define this_ce amqp_queue_class_entry


/* {{{ proto AMQPQueue::__construct(AMQPChannel channel)
AMQPQueue constructor
*/
PHP_METHOD(amqp_queue_class, __construct)
{
	zval *arguments;
	zval *channelObj;
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &channelObj) == FAILURE) {
		return;
	}

	MAKE_STD_ZVAL(arguments);
	array_init(arguments);
	zend_update_property(this_ce, getThis(), ZEND_STRL("arguments"), arguments TSRMLS_CC);
	zval_ptr_dtor(&arguments);

	/* Pull the channel out */
	channel = PHP_AMQP_GET_CHANNEL(channelObj);
	AMQP_VERIFY_CHANNEL(channel, "Could not create queue.");

	zend_update_property(this_ce, getThis(), ZEND_STRL("channel"), channelObj TSRMLS_CC);

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not create queue.");

	zend_update_property(this_ce, getThis(), ZEND_STRL("connection"), channel->connection TSRMLS_CC);

}
/* }}} */


/* {{{ proto AMQPQueue::getName()
Get the queue name */
PHP_METHOD(amqp_queue_class, getName)
{
	PHP_AMQP_NOPARAMS();

	if (PHP_AMQP_READ_THIS_PROP_STRLEN("name") > 0) {
		PHP_AMQP_RETURN_THIS_PROP("name");
	} else {
		/* BC */
		RETURN_FALSE;
	}
}
/* }}} */


/* {{{ proto AMQPQueue::setName(string name)
Set the queue name */
PHP_METHOD(amqp_queue_class, setName)
{
	char *name = NULL;
	int name_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len) == FAILURE) {
		return;
	}

	if (name_len < 1 || name_len > 255) {
		/* Verify that the name is not null and not an empty string */
		zend_throw_exception(amqp_queue_exception_class_entry, "Invalid queue name given, must be between 1 and 255 characters long.", 0 TSRMLS_CC);
		return;
	}

	/* Set the queue name */
	zend_update_property_stringl(this_ce, getThis(), ZEND_STRL("name"), name, name_len TSRMLS_CC);
}
/* }}} */



/* {{{ proto AMQPQueue::getFlags()
Get the queue parameters */
PHP_METHOD(amqp_queue_class, getFlags)
{
	long flagBitmask = 0;

	PHP_AMQP_NOPARAMS();

	if (PHP_AMQP_READ_THIS_PROP_BOOL("passive")) {
		flagBitmask |= AMQP_PASSIVE;
	}

	if (PHP_AMQP_READ_THIS_PROP_BOOL("durable")) {
		flagBitmask |= AMQP_DURABLE;
	}

	if (PHP_AMQP_READ_THIS_PROP_BOOL("exclusive")) {
		flagBitmask |= AMQP_EXCLUSIVE;
	}

	if (PHP_AMQP_READ_THIS_PROP_BOOL("auto_delete")) {
		flagBitmask |= AMQP_AUTODELETE;
	}

	RETURN_LONG(flagBitmask);
}
/* }}} */


/* {{{ proto AMQPQueue::setFlags(long bitmask)
Set the queue parameters */
PHP_METHOD(amqp_queue_class, setFlags)
{
	long flagBitmask;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &flagBitmask) == FAILURE) {
		return;
	}

	/* Set the flags based on the bitmask we were given */
	flagBitmask = flagBitmask ? flagBitmask & PHP_AMQP_QUEUE_FLAGS : flagBitmask;

	zend_update_property_bool(this_ce, getThis(), ZEND_STRL("passive"), IS_PASSIVE(flagBitmask) TSRMLS_CC);
	zend_update_property_bool(this_ce, getThis(), ZEND_STRL("durable"), IS_DURABLE(flagBitmask) TSRMLS_CC);
	zend_update_property_bool(this_ce, getThis(), ZEND_STRL("exclusive"), IS_EXCLUSIVE(flagBitmask) TSRMLS_CC);
	zend_update_property_bool(this_ce, getThis(), ZEND_STRL("auto_delete"), IS_AUTODELETE(flagBitmask) TSRMLS_CC);

	/* BC */
	RETURN_TRUE;
}
/* }}} */


/* {{{ proto AMQPQueue::getArgument(string key)
Get the queue argument referenced by key */
PHP_METHOD(amqp_queue_class, getArgument)
{
	zval **tmp;
	char *key;
	int key_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &key_len) == FAILURE) {
		return;
	}

	if (zend_hash_find(PHP_AMQP_READ_THIS_PROP_ARR("arguments"), key, (uint)(key_len + 1), (void **)&tmp) == FAILURE) {
		RETURN_FALSE;
	}

	*return_value = **tmp;

	zval_copy_ctor(return_value);
	INIT_PZVAL(return_value);
}
/* }}} */

/* {{{ proto AMQPQueue::hasArgument(string key) */
PHP_METHOD(amqp_queue_class, hasArgument)
{
	zval **tmp;
	char *key;
	int key_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &key_len) == FAILURE) {
		return;
	}

	if (zend_hash_find(PHP_AMQP_READ_THIS_PROP_ARR("arguments"), key, (uint)(key_len + 1), (void **)&tmp) == FAILURE) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto AMQPQueue::getArguments
Get the queue arguments */
PHP_METHOD(amqp_queue_class, getArguments)
{
	PHP_AMQP_NOPARAMS();
	PHP_AMQP_RETURN_THIS_PROP("arguments");
}
/* }}} */

/* {{{ proto AMQPQueue::setArguments(array args)
Overwrite all queue arguments with given args */
PHP_METHOD(amqp_queue_class, setArguments)
{
	zval *zvalArguments;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &zvalArguments) == FAILURE) {
		return;
	}

	zend_update_property(this_ce, getThis(), ZEND_STRL("arguments"), zvalArguments TSRMLS_CC);

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto AMQPQueue::setArgument(key, value)
Get the queue name */
PHP_METHOD(amqp_queue_class, setArgument)
{
	char *key= NULL;    int key_len = 0;
	zval *value = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz",
							  &key, &key_len,
							  &value) == FAILURE) {
		return;
	}

	switch (Z_TYPE_P(value)) {
		case IS_NULL:
			zend_hash_del_key_or_index(PHP_AMQP_READ_THIS_PROP_ARR("arguments"), key, (uint) (key_len + 1), 0, HASH_DEL_KEY);
			break;
		case IS_BOOL:
		case IS_LONG:
		case IS_DOUBLE:
		case IS_STRING:
			zend_hash_add(PHP_AMQP_READ_THIS_PROP_ARR("arguments"), key, (uint) (key_len + 1), &value, sizeof(zval *), NULL);
			Z_ADDREF_P(value);
			break;
		default:
			zend_throw_exception(amqp_exchange_exception_class_entry, "The value parameter must be of type NULL, int, double or string.", 0 TSRMLS_CC);
			return;
	}

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto int AMQPQueue::declareQueue();
declare queue
*/
PHP_METHOD(amqp_queue_class, declareQueue)
{
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	char *name;
	amqp_table_t *arguments;
	long message_count;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	channel = PHP_AMQP_GET_CHANNEL(PHP_AMQP_READ_THIS_PROP("channel"));
	AMQP_VERIFY_CHANNEL(channel, "Could not declare queue.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not declare queue.");

	arguments = convert_zval_to_amqp_table(PHP_AMQP_READ_THIS_PROP("arguments") TSRMLS_CC);

	amqp_queue_declare_ok_t *r = amqp_queue_declare(
		connection->connection_resource->connection_state,
		channel->channel_id,
		amqp_cstring_bytes(PHP_AMQP_READ_THIS_PROP_STR("name")),
		PHP_AMQP_READ_THIS_PROP_BOOL("passive"),
		PHP_AMQP_READ_THIS_PROP_BOOL("durable"),
		PHP_AMQP_READ_THIS_PROP_BOOL("exclusive"),
		PHP_AMQP_READ_THIS_PROP_BOOL("auto_delete"),
		*arguments
	);

	php_amqp_free_amqp_table(arguments);

	if (!r) {
		amqp_rpc_reply_t res = amqp_get_rpc_reply(connection->connection_resource->connection_state);

		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_queue_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		PHP_AMQP_DESTROY_ERROR_MESSAGE();
		return;
	}

	message_count = r->message_count;

	/* Set the queue name, in case it is an autogenerated queue name */
	name = stringify_bytes(r->queue);
	zend_update_property_string(this_ce, getThis(), ZEND_STRL("name"), name TSRMLS_CC);
	efree(name);

	php_amqp_maybe_release_buffers_on_channel(connection, channel);

	RETURN_LONG(message_count);
}
/* }}} */


/* {{{ proto int AMQPQueue::bind(string exchangeName, [string routingKey, array arguments]);
bind queue to exchange by routing key
*/
PHP_METHOD(amqp_queue_class, bind)
{
	zval *zvalArguments = NULL;

	amqp_channel_object *channel;
	amqp_connection_object *connection;

	char *exchange_name;
	int   exchange_name_len;

	char *keyname     = NULL;
	int   keyname_len = 0;

	amqp_table_t *arguments = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|sa",
							  &exchange_name, &exchange_name_len,
							  &keyname, &keyname_len,
							  &zvalArguments) == FAILURE) {
		return;
	}


	channel = PHP_AMQP_GET_CHANNEL(PHP_AMQP_READ_THIS_PROP("channel"));
	AMQP_VERIFY_CHANNEL(channel, "Could not bind queue.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not bind queue.");

	if (zvalArguments) {
		arguments = convert_zval_to_amqp_table(zvalArguments TSRMLS_CC);
	}

	amqp_queue_bind(
		connection->connection_resource->connection_state,
		channel->channel_id,
		amqp_cstring_bytes(PHP_AMQP_READ_THIS_PROP_STR("name")),
		(exchange_name_len > 0 ? amqp_cstring_bytes(exchange_name) : amqp_empty_bytes),
		(keyname_len > 0 ? amqp_cstring_bytes(keyname) : amqp_empty_bytes),
		(arguments ? *arguments : amqp_empty_table)
	);

	if (arguments) {
		php_amqp_free_amqp_table(arguments);
	}

	amqp_rpc_reply_t res = amqp_get_rpc_reply(connection->connection_resource->connection_state);

	if (res.reply_type != AMQP_RESPONSE_NORMAL) {
		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_queue_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		PHP_AMQP_DESTROY_ERROR_MESSAGE();
		return;
	}

	php_amqp_maybe_release_buffers_on_channel(connection, channel);

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto int AMQPQueue::get([bit flags=AMQP_NOPARAM]);
read messages from queue
return array (messages)
*/
PHP_METHOD(amqp_queue_class, get)
{
	amqp_channel_object *channel;
	amqp_connection_object *connection;
	zval *message;

	long flags = INI_INT("amqp.auto_ack") ? AMQP_AUTOACK : AMQP_NOPARAM;

	/* Parse out the method parameters */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &flags) == FAILURE) {
		return;
	}

	channel = PHP_AMQP_GET_CHANNEL(PHP_AMQP_READ_THIS_PROP("channel"));
	AMQP_VERIFY_CHANNEL(channel, "Could not get messages from queue.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not get messages from queue.");

	amqp_rpc_reply_t res = amqp_basic_get(
		connection->connection_resource->connection_state,
		channel->channel_id,
		amqp_cstring_bytes(PHP_AMQP_READ_THIS_PROP_STR("name")),
		(AMQP_AUTOACK & flags) ? 1 : 0
	);

	if (res.reply_type != AMQP_RESPONSE_NORMAL) {
		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_queue_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		PHP_AMQP_DESTROY_ERROR_MESSAGE();
		return;
	}

	if (AMQP_BASIC_GET_EMPTY_METHOD == res.reply.id) {
		RETURN_FALSE;
	}

	assert(AMQP_BASIC_GET_OK_METHOD == res.reply.id);

	/* Fill the envelope from response */
	amqp_basic_get_ok_t *get_ok_method = res.reply.decoded;

	amqp_envelope_t envelope;

	envelope.channel      = channel->channel_id;
	envelope.consumer_tag = amqp_empty_bytes;
	envelope.delivery_tag = get_ok_method->delivery_tag;
	envelope.redelivered  = get_ok_method->redelivered;
	envelope.exchange     = amqp_bytes_malloc_dup(get_ok_method->exchange);
	envelope.routing_key  = amqp_bytes_malloc_dup(get_ok_method->routing_key);

	php_amqp_maybe_release_buffers_on_channel(connection, channel);

  	res = amqp_read_message(
		connection->connection_resource->connection_state,
		channel->channel_id,
		&envelope.message,
		0
	);

	if (AMQP_RESPONSE_NORMAL != res.reply_type) {
		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_queue_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		amqp_destroy_envelope(&envelope);
		PHP_AMQP_DESTROY_ERROR_MESSAGE();
		return;
	}

	MAKE_STD_ZVAL(message);
	convert_amqp_envelope_to_zval(&envelope, message TSRMLS_CC);

	php_amqp_maybe_release_buffers_on_channel(connection, channel);
	amqp_destroy_envelope(&envelope);

	COPY_PZVAL_TO_ZVAL(*return_value, message);
}
/* }}} */


/* {{{ proto array AMQPQueue::consume([callback, flags = <bitmask>, consumer_tag]);
consume the message
*/
PHP_METHOD(amqp_queue_class, consume)
{
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	zend_fcall_info fci = empty_fcall_info;
	zend_fcall_info_cache fci_cache = empty_fcall_info_cache;

	amqp_table_t *arguments;

	char *consumer_tag = NULL;  int consumer_tag_len = 0;
	long flags = INI_INT("amqp.auto_ack") ? AMQP_AUTOACK : AMQP_NOPARAM;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|f!ls",
							  &fci, &fci_cache,
							  &flags,
							  &consumer_tag, &consumer_tag_len) == FAILURE) {
		return;
	}

	channel = PHP_AMQP_GET_CHANNEL(PHP_AMQP_READ_THIS_PROP("channel"));
	AMQP_VERIFY_CHANNEL(channel, "Could not get channel.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not get connection.");

	if (!(AMQP_JUST_CONSUME & flags)) {
		/* Setup the consume */
		arguments = convert_zval_to_amqp_table(PHP_AMQP_READ_THIS_PROP("arguments") TSRMLS_CC);

		amqp_basic_consume_ok_t *r = amqp_basic_consume(
				connection->connection_resource->connection_state,
				channel->channel_id,
				amqp_cstring_bytes(PHP_AMQP_READ_THIS_PROP_STR("name")),
				(consumer_tag_len > 0 ? amqp_cstring_bytes(consumer_tag) : amqp_empty_bytes), /* Consumer tag */
				(AMQP_NOLOCAL & flags) ? 1 : 0, /* No local */
				(AMQP_AUTOACK & flags) ? 1 : 0,    /* no_ack, aka AUTOACK */
				PHP_AMQP_READ_THIS_PROP_BOOL("exclusive"),
				*arguments
		);

		php_amqp_free_amqp_table(arguments);

		if (!r) {
			amqp_rpc_reply_t res = amqp_get_rpc_reply(connection->connection_resource->connection_state);

			PHP_AMQP_INIT_ERROR_MESSAGE();

			php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

			zend_throw_exception(amqp_queue_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
			php_amqp_maybe_release_buffers_on_channel(connection, channel);

			PHP_AMQP_DESTROY_ERROR_MESSAGE();
			return;
		}

		/* Set the consumer tag name, in case it is an autogenerated consumer tag name */
		zend_update_property_stringl(this_ce, getThis(), ZEND_STRL("consumer_tag"), (const char *) r->consumer_tag.bytes, (int) r->consumer_tag.len TSRMLS_CC);
	}

	if (!ZEND_FCI_INITIALIZED(fci)) {
		/* Callback not set, we have nothing to do - real consuming may happens later */
		return;
	}

	struct timeval tv = {0};
	struct timeval *tv_ptr = &tv;

	if (connection->read_timeout > 0) {
		tv.tv_sec  = (long int) connection->read_timeout;
		tv.tv_usec = (long int) ((connection->read_timeout - tv.tv_sec) * 1000000);
	} else {
		tv_ptr = NULL;
	}

	while(1) {
		/* Initialize the message */
		zval *message;

		amqp_envelope_t envelope;

		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		amqp_rpc_reply_t res = amqp_consume_message(connection->connection_resource->connection_state, &envelope, tv_ptr, 0);

		if (AMQP_RESPONSE_LIBRARY_EXCEPTION == res.reply_type && AMQP_STATUS_TIMEOUT == res.library_error) {
			PHP_AMQP_INIT_ERROR_MESSAGE();

			zend_throw_exception(amqp_queue_exception_class_entry, "Consumer timeout exceed", 0 TSRMLS_CC);

			amqp_destroy_envelope(&envelope);
			php_amqp_maybe_release_buffers_on_channel(connection, channel);

			PHP_AMQP_DESTROY_ERROR_MESSAGE();
			return;
		}

		if (AMQP_RESPONSE_NORMAL != res.reply_type) {
			PHP_AMQP_INIT_ERROR_MESSAGE();

			php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

			php_amqp_zend_throw_exception(res, amqp_queue_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);

			amqp_destroy_envelope(&envelope);
			php_amqp_maybe_release_buffers_on_channel(connection, channel);

			PHP_AMQP_DESTROY_ERROR_MESSAGE();
			return;
		}

		MAKE_STD_ZVAL(message);
		convert_amqp_envelope_to_zval(&envelope, message TSRMLS_CC);

		amqp_destroy_envelope(&envelope);

		/* Make the callback */
		zval *params;
		zval *retval_ptr = NULL;

		/* Build the parameter array */
		MAKE_STD_ZVAL(params);
		array_init(params);

		/* Dump it into the params array */
		add_index_zval(params, 0, message);
		Z_ADDREF_P(message);

		/* Add a pointer to the queue: */
		add_index_zval(params, 1, getThis());
		Z_ADDREF_P(getThis());

		/* Convert everything to be callable */
		zend_fcall_info_args(&fci, params TSRMLS_CC);
		/* Initialize the return value pointer */
		fci.retval_ptr_ptr = &retval_ptr;

		/* Call the function, and track the return value */
		if (zend_call_function(&fci, &fci_cache TSRMLS_CC) == SUCCESS && fci.retval_ptr_ptr && *fci.retval_ptr_ptr) {
			COPY_PZVAL_TO_ZVAL(*return_value, *fci.retval_ptr_ptr);
  		}

		/* Clean up our mess */
		zend_fcall_info_args_clear(&fci, 1);
		zval_ptr_dtor(&params);
		zval_ptr_dtor(&message);

		/* Check if user land function wants to bail */
		if (EG(exception) || (Z_TYPE_P(return_value) == IS_BOOL && !Z_BVAL_P(return_value))) {
			break;
		}
	}

	php_amqp_maybe_release_buffers_on_channel(connection, channel);
	return;
}
/* }}} */


/* {{{ proto int AMQPQueue::ack(long deliveryTag, [bit flags=AMQP_NOPARAM]);
	acknowledge the message
*/
PHP_METHOD(amqp_queue_class, ack)
{
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	long deliveryTag = 0;
	long flags = AMQP_NOPARAM;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|l", &deliveryTag, &flags ) == FAILURE) {
		return;
	}

	channel = PHP_AMQP_GET_CHANNEL(PHP_AMQP_READ_THIS_PROP("channel"));
	AMQP_VERIFY_CHANNEL(channel, "Could not ack message.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not ack message.");

	/* NOTE: basic.ack is asynchronous and thus will not indicate failure if something goes wrong on the broker */
	int status = amqp_basic_ack(
		connection->connection_resource->connection_state,
		channel->channel_id,
		(uint64_t) deliveryTag,
		(AMQP_MULTIPLE & flags) ? 1 : 0
	);

	if (status != AMQP_STATUS_OK) {
		/* Emulate library error */
		amqp_rpc_reply_t res;
		res.reply_type 	  = AMQP_RESPONSE_LIBRARY_EXCEPTION;
		res.library_error = status;

		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_queue_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		PHP_AMQP_DESTROY_ERROR_MESSAGE();
		return;
	}

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto int AMQPQueue::nack(long deliveryTag, [bit flags=AMQP_NOPARAM]);
	acknowledge the message
*/
PHP_METHOD(amqp_queue_class, nack)
{
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	long deliveryTag = 0;
	long flags = AMQP_NOPARAM;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|l", &deliveryTag, &flags ) == FAILURE) {
		return;
	}

	channel = PHP_AMQP_GET_CHANNEL(PHP_AMQP_READ_THIS_PROP("channel"));
	AMQP_VERIFY_CHANNEL(channel, "Could not nack message.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not nack message.");

	/* NOTE: basic.nack is asynchronous and thus will not indicate failure if something goes wrong on the broker */
	int status = amqp_basic_nack(
		connection->connection_resource->connection_state,
		channel->channel_id,
		(uint64_t) deliveryTag,
		(AMQP_MULTIPLE & flags) ? 1 : 0,
		(AMQP_REQUEUE & flags) ? 1 : 0
	);

	if (status != AMQP_STATUS_OK) {
		/* Emulate library error */
		amqp_rpc_reply_t res;
		res.reply_type 	  = AMQP_RESPONSE_LIBRARY_EXCEPTION;
		res.library_error = status;

		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_queue_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		PHP_AMQP_DESTROY_ERROR_MESSAGE();
		return;
	}

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto int AMQPQueue::reject(long deliveryTag, [bit flags=AMQP_NOPARAM]);
	acknowledge the message
*/
PHP_METHOD(amqp_queue_class, reject)
{
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	long deliveryTag = 0;
	long flags = AMQP_NOPARAM;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|l", &deliveryTag, &flags) == FAILURE) {
		return;
	}

	channel = PHP_AMQP_GET_CHANNEL(PHP_AMQP_READ_THIS_PROP("channel"));
	AMQP_VERIFY_CHANNEL(channel, "Could not reject message.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not reject message.");

	/* NOTE: basic.reject is asynchronous and thus will not indicate failure if something goes wrong on the broker */
	int status = amqp_basic_reject(
		connection->connection_resource->connection_state,
		channel->channel_id,
		(uint64_t) deliveryTag,
		(AMQP_REQUEUE & flags) ? 1 : 0
	);

	if (status != AMQP_STATUS_OK) {
		/* Emulate library error */
		amqp_rpc_reply_t res;
		res.reply_type 	  = AMQP_RESPONSE_LIBRARY_EXCEPTION;
		res.library_error = status;

		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_queue_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		PHP_AMQP_DESTROY_ERROR_MESSAGE();
		return;
	}

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto int AMQPQueue::purge();
purge queue
*/
PHP_METHOD(amqp_queue_class, purge)
{
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	channel = PHP_AMQP_GET_CHANNEL(PHP_AMQP_READ_THIS_PROP("channel"));
	AMQP_VERIFY_CHANNEL(channel, "Could not purge queue.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not purge queue.");

	amqp_queue_purge_ok_t *r = amqp_queue_purge(
		connection->connection_resource->connection_state,
		channel->channel_id,
		amqp_cstring_bytes(PHP_AMQP_READ_THIS_PROP_STR("name"))
	);

	if (!r) {
		amqp_rpc_reply_t res = amqp_get_rpc_reply(connection->connection_resource->connection_state);

		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_queue_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		PHP_AMQP_DESTROY_ERROR_MESSAGE();
		return;
	}

	/* long message_count = r->message_count; */

	php_amqp_maybe_release_buffers_on_channel(connection, channel);

	/* RETURN_LONG(message_count) */;

	/* BC */
	RETURN_TRUE;
}
/* }}} */


/* {{{ proto int AMQPQueue::cancel([string consumer_tag]);
cancel queue to consumer
*/
PHP_METHOD(amqp_queue_class, cancel)
{
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	char *consumer_tag = NULL;  int consumer_tag_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &consumer_tag, &consumer_tag_len) == FAILURE) {
		return;
	}

	channel = PHP_AMQP_GET_CHANNEL(PHP_AMQP_READ_THIS_PROP("channel"));
	AMQP_VERIFY_CHANNEL(channel, "Could not cancel queue.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not cancel queue.");

    if (!consumer_tag_len && !PHP_AMQP_READ_THIS_PROP_STRLEN("consumer_tag")) {
        return;
    }

	amqp_basic_cancel_ok_t *r = amqp_basic_cancel(
		connection->connection_resource->connection_state,
		channel->channel_id,
		consumer_tag_len > 0 ? amqp_cstring_bytes(consumer_tag) : amqp_cstring_bytes(PHP_AMQP_READ_THIS_PROP_STR("consumer_tag"))
	);

	if (!r) {
		amqp_rpc_reply_t res = amqp_get_rpc_reply(connection->connection_resource->connection_state);

		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_queue_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		PHP_AMQP_DESTROY_ERROR_MESSAGE();
		return;
	}

	if (!consumer_tag_len || strcmp(consumer_tag, PHP_AMQP_READ_THIS_PROP_STR("consumer_tag")) != 0) {
		zend_update_property_null(this_ce, getThis(), ZEND_STRL("consumer_tag") TSRMLS_CC);
	}

	php_amqp_maybe_release_buffers_on_channel(connection, channel);

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto int AMQPQueue::unbind(string exchangeName, [string routingKey, array arguments]);
unbind queue from exchange
*/
PHP_METHOD(amqp_queue_class, unbind)
{
	zval *zvalArguments = NULL;
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	char *exchange_name;
	int   exchange_name_len;
	char *keyname     = NULL;
	int   keyname_len = 0;

	amqp_table_t *arguments = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|sa",
							  &exchange_name, &exchange_name_len,
							  &keyname, &keyname_len,
							  &zvalArguments) == FAILURE) {
		return;
	}

	channel = PHP_AMQP_GET_CHANNEL(PHP_AMQP_READ_THIS_PROP("channel"));
	AMQP_VERIFY_CHANNEL(channel, "Could not unbind queue.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not unbind queue.");

	if (zvalArguments) {
		arguments = convert_zval_to_amqp_table(zvalArguments TSRMLS_CC);
	}

	amqp_queue_unbind(
		connection->connection_resource->connection_state,
		channel->channel_id,
		amqp_cstring_bytes(PHP_AMQP_READ_THIS_PROP_STR("name")),
		(exchange_name_len > 0 ? amqp_cstring_bytes(exchange_name) : amqp_empty_bytes),
		(keyname_len > 0 ? amqp_cstring_bytes(keyname) : amqp_empty_bytes),
		(arguments ? *arguments : amqp_empty_table)
	);

	if (arguments) {
		php_amqp_free_amqp_table(arguments);
	}

	amqp_rpc_reply_t res = amqp_get_rpc_reply(connection->connection_resource->connection_state);

	if (res.reply_type != AMQP_RESPONSE_NORMAL) {
		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_queue_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		PHP_AMQP_DESTROY_ERROR_MESSAGE();
		return;
	}

	php_amqp_maybe_release_buffers_on_channel(connection, channel);

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto int AMQPQueue::delete([long flags = AMQP_NOPARAM]]);
delete queue and return the number of messages deleted in it
*/
PHP_METHOD(amqp_queue_class, delete)
{
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	long flags = AMQP_NOPARAM;

	long message_count;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &flags) == FAILURE) {
		return;
	}

	channel = PHP_AMQP_GET_CHANNEL(PHP_AMQP_READ_THIS_PROP("channel"));
	AMQP_VERIFY_CHANNEL(channel, "Could not delete queue.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not delete queue.");

	amqp_queue_delete_ok_t * r = amqp_queue_delete(
		connection->connection_resource->connection_state,
		channel->channel_id,
		amqp_cstring_bytes(PHP_AMQP_READ_THIS_PROP_STR("name")),
		(AMQP_IFUNUSED & flags) ? 1 : 0,
		(AMQP_IFEMPTY & flags) ? 1 : 0
	);

	if (!r) {
		amqp_rpc_reply_t res = amqp_get_rpc_reply(connection->connection_resource->connection_state);

		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_queue_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		PHP_AMQP_DESTROY_ERROR_MESSAGE();
		return;
	}

	message_count = r->message_count;

	php_amqp_maybe_release_buffers_on_channel(connection, channel);

	RETURN_LONG(message_count);
}
/* }}} */

/* {{{ proto AMQPChannel::getChannel()
Get the AMQPChannel object in use */
PHP_METHOD(amqp_queue_class, getChannel)
{
	PHP_AMQP_NOPARAMS();
	PHP_AMQP_RETURN_THIS_PROP("channel");
}
/* }}} */

/* {{{ proto AMQPChannel::getConnection()
Get the AMQPConnection object in use */
PHP_METHOD(amqp_queue_class, getConnection)
{
	PHP_AMQP_NOPARAMS();
	PHP_AMQP_RETURN_THIS_PROP("connection");
}
/* }}} */

/* {{{ proto string AMQPChannel::getConsumerTag()
Get latest consumer tag*/
PHP_METHOD(amqp_queue_class, getConsumerTag)
{
	PHP_AMQP_NOPARAMS();
	PHP_AMQP_RETURN_THIS_PROP("consumer_tag");
}

/* }}} */
/* amqp_queue_class ARG_INFO definition */
ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_queue_class__construct, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
				ZEND_ARG_OBJ_INFO(0, amqp_channel, AMQPChannel, 0)
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

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_queue_class_hasArgument, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
				ZEND_ARG_INFO(0, key)
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
		PHP_ME(amqp_queue_class, hasArgument,		arginfo_amqp_queue_class_hasArgument,		ZEND_ACC_PUBLIC)

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

PHP_MINIT_FUNCTION(amqp_queue)
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "AMQPQueue", amqp_queue_class_functions);
	this_ce = zend_register_internal_class(&ce TSRMLS_CC);

	zend_declare_property_null(this_ce, ZEND_STRL("connection"), ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(this_ce, ZEND_STRL("channel"), ZEND_ACC_PRIVATE TSRMLS_CC);

	zend_declare_property_stringl(this_ce, ZEND_STRL("name"), "", 0, ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(this_ce, ZEND_STRL("consumer_tag"), ZEND_ACC_PRIVATE TSRMLS_CC);

	zend_declare_property_bool(this_ce, ZEND_STRL("passive"), 0, ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_bool(this_ce, ZEND_STRL("durable"), 0, ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_bool(this_ce, ZEND_STRL("exclusive"), 0, ZEND_ACC_PRIVATE TSRMLS_CC);
	/* By default, the auto_delete flag should be set */
	zend_declare_property_bool(this_ce, ZEND_STRL("auto_delete"), 1, ZEND_ACC_PRIVATE TSRMLS_CC);



	zend_declare_property_null(this_ce, ZEND_STRL("arguments"), ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}

/*
*Local variables:
*tab-width: 4
*tabstop: 4
*c-basic-offset: 4
*End:
*vim600: noet sw=4 ts=4 fdm=marker
*vim<600: noet sw=4 ts=4
*/
