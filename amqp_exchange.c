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

/* $Id: amqp_exchange.c 327551 2012-09-09 03:49:34Z pdezwart $ */

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
#include "amqp_exchange.h"

zend_class_entry *amqp_exchange_class_entry;
#define this_ce amqp_exchange_class_entry

/* {{{ proto AMQPExchange::__construct(AMQPChannel channel);
create Exchange   */
PHP_METHOD(amqp_exchange_class, __construct)
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
	AMQP_VERIFY_CHANNEL(channel, "Could not create exchange.");

	zend_update_property(this_ce, getThis(), ZEND_STRL("channel"), channelObj TSRMLS_CC);

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not create exchange.");

	zend_update_property(this_ce, getThis(), ZEND_STRL("connection"), channel->connection TSRMLS_CC);
}
/* }}} */


/* {{{ proto AMQPExchange::getName()
Get the exchange name */
PHP_METHOD(amqp_exchange_class, getName)
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


/* {{{ proto AMQPExchange::setName(string name)
Set the exchange name */
PHP_METHOD(amqp_exchange_class, setName)
{
	char *name = NULL;
	int name_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len) == FAILURE) {
		return;
	}

	/* Verify that the name is not null and not an empty string */
	if (name_len > 255) {
		zend_throw_exception(amqp_exchange_exception_class_entry, "Invalid exchange name given, must be less than 255 characters long.", 0 TSRMLS_CC);
		return;
	}

	/* Set the exchange name */
	zend_update_property_stringl(this_ce, getThis(), ZEND_STRL("name"), name, name_len TSRMLS_CC);
}
/* }}} */


/* {{{ proto AMQPExchange::getFlags()
Get the exchange parameters */
PHP_METHOD(amqp_exchange_class, getFlags)
{
	long flagBitmask = 0;

	PHP_AMQP_NOPARAMS();

	if (PHP_AMQP_READ_THIS_PROP_BOOL("passive")) {
		flagBitmask |= AMQP_PASSIVE;
	}

	if (PHP_AMQP_READ_THIS_PROP_BOOL("durable")) {
		flagBitmask |= AMQP_DURABLE;
	}

	if (PHP_AMQP_READ_THIS_PROP_BOOL("auto_delete")) {
		flagBitmask |= AMQP_AUTODELETE;
	}

	if (PHP_AMQP_READ_THIS_PROP_BOOL("internal")) {
		flagBitmask |= AMQP_INTERNAL;
	}

	RETURN_LONG(flagBitmask);
}
/* }}} */


/* {{{ proto AMQPExchange::setFlags(long bitmask)
Set the exchange parameters */
PHP_METHOD(amqp_exchange_class, setFlags)
{
	long flagBitmask;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &flagBitmask) == FAILURE) {
		return;
	}

	/* Set the flags based on the bitmask we were given */
	flagBitmask = flagBitmask ? flagBitmask & PHP_AMQP_EXCHANGE_FLAGS : flagBitmask;

	zend_update_property_bool(this_ce, getThis(), ZEND_STRL("passive"), IS_PASSIVE(flagBitmask) TSRMLS_CC);
	zend_update_property_bool(this_ce, getThis(), ZEND_STRL("durable"), IS_DURABLE(flagBitmask) TSRMLS_CC);
	zend_update_property_bool(this_ce, getThis(), ZEND_STRL("auto_delete"), IS_AUTODELETE(flagBitmask) TSRMLS_CC);
	zend_update_property_bool(this_ce, getThis(), ZEND_STRL("internal"), IS_INTERNAL(flagBitmask) TSRMLS_CC);
}
/* }}} */


/* {{{ proto AMQPExchange::getType()
Get the exchange type */
PHP_METHOD(amqp_exchange_class, getType)
{
	PHP_AMQP_NOPARAMS();

	if (PHP_AMQP_READ_THIS_PROP_STRLEN("type") > 0) {
		PHP_AMQP_RETURN_THIS_PROP("type");
	} else {
		/* BC */
		RETURN_FALSE;
	}
}
/* }}} */


/* {{{ proto AMQPExchange::setType(string type)
Set the exchange type */
PHP_METHOD(amqp_exchange_class, setType)
{
	char *type = NULL;	int type_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &type, &type_len) == FAILURE) {
		return;
	}

	zend_update_property_stringl(this_ce, getThis(), ZEND_STRL("type"), type, type_len TSRMLS_CC);
}
/* }}} */


/* {{{ proto AMQPExchange::getArgument(string key)
Get the exchange argument referenced by key */
PHP_METHOD(amqp_exchange_class, getArgument)
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

/* {{{ proto AMQPExchange::hasArgument(string key) */
PHP_METHOD(amqp_exchange_class, hasArgument)
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

/* {{{ proto AMQPExchange::getArguments
Get the exchange arguments */
PHP_METHOD(amqp_exchange_class, getArguments)
{
	PHP_AMQP_NOPARAMS();
	PHP_AMQP_RETURN_THIS_PROP("arguments");
}
/* }}} */


/* {{{ proto AMQPExchange::setArguments(array args)
Overwrite all exchange arguments with given args */
PHP_METHOD(amqp_exchange_class, setArguments)
{
	zval *zvalArguments;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &zvalArguments) == FAILURE) {
		return;
	}

	zend_update_property(this_ce, getThis(), ZEND_STRL("arguments"), zvalArguments TSRMLS_CC);

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto AMQPExchange::setArgument(key, value) */
PHP_METHOD(amqp_exchange_class, setArgument)
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


/* {{{ proto AMQPExchange::declareExchange();
declare Exchange
*/
PHP_METHOD(amqp_exchange_class, declareExchange)
{
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	amqp_table_t *arguments;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	channel = PHP_AMQP_GET_CHANNEL(PHP_AMQP_READ_THIS_PROP("channel"));
	AMQP_VERIFY_CHANNEL(channel, "Could not declare exchange.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not declare exchange.");

	/* Check that the exchange has a name */
	if (Z_STRLEN_P(PHP_AMQP_READ_THIS_PROP("name")) < 1) {
		zend_throw_exception(amqp_exchange_exception_class_entry, "Could not declare exchange. Exchanges must have a name.", 0 TSRMLS_CC);
		return;
	}

	/* Check that the exchange has a name */
	if (Z_STRLEN_P(PHP_AMQP_READ_THIS_PROP("type")) < 1) {
		zend_throw_exception(amqp_exchange_exception_class_entry, "Could not declare exchange. Exchanges must have a type.", 0 TSRMLS_CC);
		return;
	}

	arguments = convert_zval_to_amqp_table(PHP_AMQP_READ_THIS_PROP("arguments") TSRMLS_CC);
	
#if AMQP_VERSION_MAJOR * 100 + AMQP_VERSION_MINOR * 10 + AMQP_VERSION_PATCH > 52
	amqp_exchange_declare(
		connection->connection_resource->connection_state,
		channel->channel_id,
		amqp_cstring_bytes(PHP_AMQP_READ_THIS_PROP_STR("name")),
		amqp_cstring_bytes(PHP_AMQP_READ_THIS_PROP_STR("type")),
		PHP_AMQP_READ_THIS_PROP_BOOL("passive"),
		PHP_AMQP_READ_THIS_PROP_BOOL("durable"),
		PHP_AMQP_READ_THIS_PROP_BOOL("auto_delete"),
		PHP_AMQP_READ_THIS_PROP_BOOL("internal"),
		*arguments
	);
#else
	amqp_exchange_declare(
		connection->connection_resource->connection_state,
		channel->channel_id,
		amqp_cstring_bytes(PHP_AMQP_READ_THIS_PROP_STR("name")),
		amqp_cstring_bytes(PHP_AMQP_READ_THIS_PROP_STR("type")),
		PHP_AMQP_READ_THIS_PROP_BOOL("passive"),
		PHP_AMQP_READ_THIS_PROP_BOOL("durable"),
		*arguments
	);
#endif

	amqp_rpc_reply_t res = amqp_get_rpc_reply(connection->connection_resource->connection_state);

	php_amqp_free_amqp_table(arguments);

	/* handle any errors that occured outside of signals */
	if (res.reply_type != AMQP_RESPONSE_NORMAL) {
		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_exchange_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		PHP_AMQP_DESTROY_ERROR_MESSAGE();
		return;
	}

	php_amqp_maybe_release_buffers_on_channel(connection, channel);

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto AMQPExchange::delete([string name[, long params]]);
delete Exchange
*/
PHP_METHOD(amqp_exchange_class, delete)
{
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	char *name = NULL;  int name_len = 0;
	long flags = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|sl",
							  &name, &name_len,
							  &flags) == FAILURE) {
		return;
	}

	channel = PHP_AMQP_GET_CHANNEL(PHP_AMQP_READ_THIS_PROP("channel"));
	AMQP_VERIFY_CHANNEL(channel, "Could not delete exchange.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not delete exchange.");

 	amqp_exchange_delete(
		connection->connection_resource->connection_state,
		channel->channel_id,
		amqp_cstring_bytes(name_len ? name : PHP_AMQP_READ_THIS_PROP_STR("name")),
		(AMQP_IFUNUSED & flags) ? 1 : 0
	);

	amqp_rpc_reply_t res = amqp_get_rpc_reply(connection->connection_resource->connection_state);

	if (res.reply_type != AMQP_RESPONSE_NORMAL) {
		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_exchange_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		PHP_AMQP_DESTROY_ERROR_MESSAGE();
		return;
	}

	php_amqp_maybe_release_buffers_on_channel(connection, channel);

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto AMQPExchange::publish(string msg, [string key, [int flags, [array headers]]]);
publish into Exchange
*/
PHP_METHOD(amqp_exchange_class, publish)
{
	zval *ini_arr = NULL;
	zval** ppztmp;

	amqp_channel_object *channel;
	amqp_connection_object *connection;

	char *key_name = NULL;  int key_len  = 0;
	char *msg 	   = NULL;  int msg_len= 0;
	long flags = AMQP_NOPARAM;

#ifndef PHP_WIN32
	/* Storage for previous signal handler during SIGPIPE override */
	void * old_handler;
#endif

	amqp_basic_properties_t props;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|sla/",
							  &msg, &msg_len,
							  &key_name, &key_len,
							  &flags,
							  &ini_arr) == FAILURE) {
		return;
	}

	/* By default (and for BC) content type is text/plain (may be skipped at all, then set props._flags to 0) */
	props.content_type = amqp_cstring_bytes("text/plain");
	props._flags       = AMQP_BASIC_CONTENT_TYPE_FLAG;

	props.headers.entries = 0;

	{
	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "content_type", sizeof("content_type"), (void*)&ppztmp)) {
		SEPARATE_ZVAL(ppztmp);
		convert_to_string(*ppztmp);

		if (Z_STRLEN_PP(ppztmp) > 0) {
			props.content_type = amqp_cstring_bytes(Z_STRVAL_PP(ppztmp));
			props._flags |= AMQP_BASIC_CONTENT_TYPE_FLAG;
		}
	}

	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "content_encoding", sizeof("content_encoding"), (void*)&ppztmp)) {
		SEPARATE_ZVAL(ppztmp);
		convert_to_string(*ppztmp);

		if (Z_STRLEN_PP(ppztmp) > 0) {
			props.content_encoding = amqp_cstring_bytes(Z_STRVAL_PP(ppztmp));
			props._flags |= AMQP_BASIC_CONTENT_ENCODING_FLAG;
		}
	}

	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "message_id", sizeof("message_id"), (void*)&ppztmp)) {
		SEPARATE_ZVAL(ppztmp);
		convert_to_string(*ppztmp);

		if (Z_STRLEN_PP(ppztmp) > 0) {
			props.message_id = amqp_cstring_bytes(Z_STRVAL_PP(ppztmp));
			props._flags |= AMQP_BASIC_MESSAGE_ID_FLAG;
		}
	}

	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "user_id", sizeof("user_id"), (void*)&ppztmp)) {
		SEPARATE_ZVAL(ppztmp);
		convert_to_string(*ppztmp);

		if (Z_STRLEN_PP(ppztmp) > 0) {
			props.user_id = amqp_cstring_bytes(Z_STRVAL_PP(ppztmp));
			props._flags |= AMQP_BASIC_USER_ID_FLAG;
		}
	}

	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "app_id", sizeof("app_id"), (void*)&ppztmp)) {
		SEPARATE_ZVAL(ppztmp);
		convert_to_string(*ppztmp);

		if (Z_STRLEN_PP(ppztmp) > 0) {
			props.app_id = amqp_cstring_bytes(Z_STRVAL_PP(ppztmp));
			props._flags |= AMQP_BASIC_APP_ID_FLAG;
		}
	}

	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "delivery_mode", sizeof("delivery_mode"), (void*)&ppztmp)) {
		SEPARATE_ZVAL(ppztmp);
		convert_to_long(*ppztmp);

		props.delivery_mode = (uint8_t)Z_LVAL_PP(ppztmp);
		props._flags |= AMQP_BASIC_DELIVERY_MODE_FLAG;
	}

	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "priority", sizeof("priority"), (void*)&ppztmp)) {
		SEPARATE_ZVAL(ppztmp);
		convert_to_long(*ppztmp);

		props.priority = (uint8_t)Z_LVAL_PP(ppztmp);
		props._flags |= AMQP_BASIC_PRIORITY_FLAG;
	}

	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "timestamp", sizeof("timestamp"), (void*)&ppztmp)) {
		SEPARATE_ZVAL(ppztmp);
		convert_to_long(*ppztmp);

		props.timestamp = (uint64_t)Z_LVAL_PP(ppztmp);
		props._flags |= AMQP_BASIC_TIMESTAMP_FLAG;
	}

	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "expiration", sizeof("expiration"), (void*)&ppztmp)) {
		SEPARATE_ZVAL(ppztmp);
		convert_to_string(*ppztmp);

		if (Z_STRLEN_PP(ppztmp) > 0) {
			props.expiration = amqp_cstring_bytes(Z_STRVAL_PP(ppztmp));
			props._flags |= AMQP_BASIC_EXPIRATION_FLAG;
		}
	}

	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "type", sizeof("type"), (void*)&ppztmp)) {
		SEPARATE_ZVAL(ppztmp);
		convert_to_string(*ppztmp);

		if (Z_STRLEN_PP(ppztmp) > 0) {
			props.type = amqp_cstring_bytes(Z_STRVAL_PP(ppztmp));
			props._flags |= AMQP_BASIC_TYPE_FLAG;
		}
	}

	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "reply_to", sizeof("reply_to"), (void*)&ppztmp)) {
		SEPARATE_ZVAL(ppztmp);
		convert_to_string(*ppztmp);

		if (Z_STRLEN_PP(ppztmp) > 0) {
			props.reply_to = amqp_cstring_bytes(Z_STRVAL_PP(ppztmp));
			props._flags |= AMQP_BASIC_REPLY_TO_FLAG;
		}
	}

	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "correlation_id", sizeof("correlation_id"), (void*)&ppztmp)) {
		SEPARATE_ZVAL(ppztmp);
		convert_to_string(*ppztmp);

		if (Z_STRLEN_PP(ppztmp) > 0) {
			props.correlation_id = amqp_cstring_bytes(Z_STRVAL_PP(ppztmp));
			props._flags |= AMQP_BASIC_CORRELATION_ID_FLAG;
		}
	}

	}

	amqp_table_t *headers = NULL;

	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF(ini_arr), "headers", sizeof("headers"), (void*)&ppztmp)) {
		SEPARATE_ZVAL(ppztmp);
		convert_to_array(*ppztmp);

		headers = convert_zval_to_amqp_table(*ppztmp TSRMLS_CC);

		props._flags |= AMQP_BASIC_HEADERS_FLAG;
		props.headers = *headers;
	}

	channel = PHP_AMQP_GET_CHANNEL(PHP_AMQP_READ_THIS_PROP("channel"));
	AMQP_VERIFY_CHANNEL(channel, "Could not publish to exchange.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not publish to exchange.");

#ifndef PHP_WIN32
	/* Start ignoring SIGPIPE */
	old_handler = signal(SIGPIPE, SIG_IGN);
#endif

	zval *exchange_name = PHP_AMQP_READ_THIS_PROP("name");

	/* NOTE: basic.publish is asynchronous and thus will not indicate failure if something goes wrong on the broker */
	int status = amqp_basic_publish(
		connection->connection_resource->connection_state,
		channel->channel_id,
		(Z_STRLEN_P(exchange_name) > 0 ? amqp_cstring_bytes(Z_STRVAL_P(exchange_name)) : amqp_empty_bytes),	/* exchange */
		(key_len > 0 ? amqp_cstring_bytes(key_name) : amqp_empty_bytes), /* routing key */
		(AMQP_MANDATORY & flags) ? 1 : 0, /* mandatory */
		(AMQP_IMMEDIATE & flags) ? 1 : 0, /* immediate */
		&props,
		php_amqp_long_string(msg, msg_len) /* message body */
	);

	if (headers) {
		php_amqp_free_amqp_table(headers);
	}

#ifndef PHP_WIN32
	/* End ignoring of SIGPIPEs */
	signal(SIGPIPE, old_handler);
#endif

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


/* {{{ proto int exchange::bind(string srcExchangeName[, string routingKey, array arguments]);
bind exchange to exchange by routing key
*/
PHP_METHOD(amqp_exchange_class, bind)
{
	zval *zvalArguments = NULL;

	amqp_channel_object *channel;
	amqp_connection_object *connection;

	char *src_name;		int src_name_len = 0;
	char *keyname;		int keyname_len = 0;

	amqp_table_t *arguments = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|sa",
							  &src_name, &src_name_len,
							  &keyname, &keyname_len,
							  &zvalArguments) == FAILURE) {
		return;
	}

	channel = PHP_AMQP_GET_CHANNEL(PHP_AMQP_READ_THIS_PROP("channel"));
	AMQP_VERIFY_CHANNEL(channel, "Could not bind to exchange.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not bind to exchanges.");

	if (zvalArguments) {
		arguments = convert_zval_to_amqp_table(zvalArguments TSRMLS_CC);
	}

	amqp_exchange_bind(
		connection->connection_resource->connection_state,
		channel->channel_id,
		amqp_cstring_bytes(PHP_AMQP_READ_THIS_PROP_STR("name")),
		(src_name_len > 0 ? amqp_cstring_bytes(src_name) : amqp_empty_bytes),
		(keyname_len  > 0 ? amqp_cstring_bytes(keyname)  : amqp_empty_bytes),
		(arguments ? *arguments : amqp_empty_table)
	);

	if (arguments) {
		php_amqp_free_amqp_table(arguments);
	}

	amqp_rpc_reply_t res = amqp_get_rpc_reply(connection->connection_resource->connection_state);

	if (res.reply_type != AMQP_RESPONSE_NORMAL) {
		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_exchange_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		PHP_AMQP_DESTROY_ERROR_MESSAGE();
		return;
	}

	php_amqp_maybe_release_buffers_on_channel(connection, channel);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int exchange::unbind(string srcExchangeName[, string routingKey, array arguments]);
remove exchange to exchange binding by routing key
*/
PHP_METHOD(amqp_exchange_class, unbind)
{
	zval *zvalArguments = NULL;

	amqp_channel_object *channel;
	amqp_connection_object *connection;

	char *src_name; 	int src_name_len = 0;
	char *keyname;		int keyname_len = 0;

	amqp_table_t *arguments = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|sa",
							  &src_name, &src_name_len,
							  &keyname, &keyname_len,
							  &zvalArguments) == FAILURE) {
		return;
	}

	channel = PHP_AMQP_GET_CHANNEL(PHP_AMQP_READ_THIS_PROP("channel"));
	AMQP_VERIFY_CHANNEL(channel, "Could not unbind from exchange.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not unbind from exchanges.");

	if (zvalArguments) {
		arguments = convert_zval_to_amqp_table(zvalArguments TSRMLS_CC);
	}

	amqp_exchange_unbind(
		connection->connection_resource->connection_state,
		channel->channel_id,
		amqp_cstring_bytes(PHP_AMQP_READ_THIS_PROP_STR("name")),
		(src_name_len > 0 ? amqp_cstring_bytes(src_name) : amqp_empty_bytes),
		(keyname_len  > 0 ? amqp_cstring_bytes(keyname)  : amqp_empty_bytes),
		(arguments ? *arguments : amqp_empty_table)
	);

	if (arguments) {
		php_amqp_free_amqp_table(arguments);
	}

	amqp_rpc_reply_t res = amqp_get_rpc_reply(connection->connection_resource->connection_state);

	if (res.reply_type != AMQP_RESPONSE_NORMAL) {
		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_exchange_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		PHP_AMQP_DESTROY_ERROR_MESSAGE();
		return;
	}

	php_amqp_maybe_release_buffers_on_channel(connection, channel);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto AMQPExchange::getChannel()
Get the AMQPChannel object in use */
PHP_METHOD(amqp_exchange_class, getChannel)
{
	PHP_AMQP_NOPARAMS();
	PHP_AMQP_RETURN_THIS_PROP("channel");
}
/* }}} */

/* {{{ proto AMQPExchange::getConnection()
Get the AMQPConnection object in use */
PHP_METHOD(amqp_exchange_class, getConnection)
{
	PHP_AMQP_NOPARAMS();
	PHP_AMQP_RETURN_THIS_PROP("connection");
}
/* }}} */

/* amqp_exchange ARG_INFO definition */
ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_exchange_class__construct, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
				ZEND_ARG_OBJ_INFO(0, amqp_channel, AMQPChannel, 0)
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

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_exchange_class_hasArgument, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
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
		PHP_ME(amqp_exchange_class, hasArgument,	arginfo_amqp_exchange_class_hasArgument,	ZEND_ACC_PUBLIC)

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

PHP_MINIT_FUNCTION(amqp_exchange)
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "AMQPExchange", amqp_exchange_class_functions);
	this_ce = zend_register_internal_class(&ce TSRMLS_CC);

	zend_declare_property_null(this_ce, ZEND_STRL("connection"), ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(this_ce, ZEND_STRL("channel"), ZEND_ACC_PRIVATE TSRMLS_CC);

	zend_declare_property_null(this_ce, ZEND_STRL("name"), ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(this_ce, ZEND_STRL("type"), ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_bool(this_ce, ZEND_STRL("passive"), 0, ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_bool(this_ce, ZEND_STRL("durable"), 0, ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_bool(this_ce, ZEND_STRL("auto_delete"), 0, ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_bool(this_ce, ZEND_STRL("internal"), 0, ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(this_ce, ZEND_STRL("arguments"), ZEND_ACC_PRIVATE TSRMLS_CC);

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
