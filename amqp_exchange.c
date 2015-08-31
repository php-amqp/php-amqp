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

zend_object_handlers amqp_exchange_object_handlers;

HashTable *amqp_exchange_object_get_debug_info(zval *object, int *is_temp TSRMLS_DC) {
	zval value;
	HashTable *debug_info;

	/* Get the envelope object from which to read */
	amqp_exchange_object *exchange = AMQP_EXCHANGE_OBJ_P(object);

	/* Let zend clean up for us: */
	*is_temp = 1;

	/* Keep the first number matching the number of entries in this table*/
	ALLOC_HASHTABLE(debug_info);
	ZEND_INIT_SYMTABLE_EX(debug_info, 6 + 1, 0);

	/* Start adding values */
	ZVAL_STRINGL(&value, exchange->name, strlen(exchange->name));
	zend_hash_str_add(debug_info, "name", sizeof("name")-1, &value);

	ZVAL_STRINGL(&value, exchange->type, strlen(exchange->type));
	zend_hash_str_add(debug_info, "type", sizeof("type")-1, &value);

	ZVAL_BOOL(&value, IS_PASSIVE(exchange->flags));
	zend_hash_str_add(debug_info, "passive", sizeof("passive")-1, &value);

	ZVAL_BOOL(&value, IS_DURABLE(exchange->flags));
	zend_hash_str_add(debug_info, "durable", sizeof("durable")-1, &value);

	ZVAL_BOOL(&value, IS_AUTODELETE(exchange->flags));
	zend_hash_str_add(debug_info, "auto_delete", sizeof("auto_delete")-1, &value);

	ZVAL_BOOL(&value, IS_INTERNAL(exchange->flags));
	zend_hash_str_add(debug_info, "internal", sizeof("internal")-1, &value);

	Z_ADDREF(exchange->arguments);
	zend_hash_str_add(debug_info, "arguments", sizeof("arguments")-1, &exchange->arguments);

	/* Start adding values */
	return debug_info;
}

void amqp_exchange_free_obj(zend_object *object TSRMLS_DC)
{
	amqp_exchange_object *exchange = amqp_exchange_object_fetch_object(object);

	zval_ptr_dtor(&exchange->arguments);

	zend_object_std_dtor(&exchange->zo TSRMLS_CC);
	efree(exchange);
}

void amqp_exchange_dtor_obj(zend_object *object TSRMLS_DC)
{
	amqp_exchange_object *exchange = amqp_exchange_object_fetch_object(object);

	zval_ptr_dtor(&exchange->channel);
}

zend_object* amqp_exchange_ctor(zend_class_entry *ce)
{
	amqp_exchange_object* exchange = (amqp_exchange_object*)ecalloc(1,
			sizeof(amqp_exchange_object)
			+ zend_object_properties_size(ce));

	/* Initialize the arguments array: */
	array_init(&exchange->arguments);

	zend_object_std_init(&exchange->zo, ce);
	object_properties_init(&exchange->zo, ce);

	memcpy((void *)&amqp_exchange_object_handlers, (void *)zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	amqp_exchange_object_handlers.get_debug_info = amqp_exchange_object_get_debug_info;
	amqp_exchange_object_handlers.offset = XtOffsetOf(amqp_exchange_object, zo);
	amqp_exchange_object_handlers.free_obj = amqp_exchange_free_obj;
	amqp_exchange_object_handlers.dtor_obj = amqp_exchange_dtor_obj;

	exchange->zo.handlers = &amqp_exchange_object_handlers;

	return &exchange->zo;
}

/* {{{ proto AMQPExchange::__construct(AMQPChannel channel);
create Exchange   */
PHP_METHOD(amqp_exchange_class, __construct)
{
	amqp_exchange_object *exchange = AMQP_EXCHANGE_OBJ_P(getThis());
	amqp_channel_object *channel;
	zval* channel_param;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"O", &channel_param, amqp_channel_class_entry) == FAILURE) {
		zend_throw_exception(amqp_exchange_exception_class_entry, "Parameter must be an instance of AMQPChannel.", 0 TSRMLS_CC);
		RETURN_NULL();
	}

	ZVAL_COPY(&exchange->channel, channel_param);

	channel = AMQP_CHANNEL_OBJ(exchange->channel);

	/* Pull the channel out */
	AMQP_VERIFY_CHANNEL(channel, "Could not create exchange.");
}
/* }}} */

/* {{{ proto AMQPExchange::getName()
Get the exchange name */
PHP_METHOD(amqp_exchange_class, getName)
{
	amqp_exchange_object *exchange = AMQP_EXCHANGE_OBJ_P(getThis());

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	/* Check if there is a name to be had: */
	if (exchange->name_len) {
		RETURN_STRING(exchange->name);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto AMQPExchange::setName(string name)
Set the exchange name */
PHP_METHOD(amqp_exchange_class, setName)
{
	amqp_exchange_object *exchange = AMQP_EXCHANGE_OBJ_P(getThis());
	char *name = NULL;
	size_t name_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len) == FAILURE) {
		return;
	}

	/* Verify that the name is not null and not an empty string */
	if (name_len > 255) {
		zend_throw_exception(amqp_exchange_exception_class_entry, "Invalid exchange name given, must be less than 255 characters long.", 0 TSRMLS_CC);
		return;
	}

	/* Set the exchange name */
	AMQP_SET_NAME(exchange, name);
}
/* }}} */

/* {{{ proto AMQPExchange::getFlags()
Get the exchange parameters */
PHP_METHOD(amqp_exchange_class, getFlags)
{
	amqp_exchange_object *exchange = AMQP_EXCHANGE_OBJ_P(getThis());

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_LONG(exchange->flags);
}
/* }}} */

/* {{{ proto AMQPExchange::setFlags(long bitmask)
Set the exchange parameters */
PHP_METHOD(amqp_exchange_class, setFlags)
{
	amqp_exchange_object *exchange = AMQP_EXCHANGE_OBJ_P(getThis());
	long flagBitmask;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &flagBitmask) == FAILURE) {
		return;
	}

	/* Set the flags based on the bitmask we were given */
	exchange->flags = flagBitmask ? flagBitmask & PHP_AMQP_EXCHANGE_FLAGS : flagBitmask;
}
/* }}} */

/* {{{ proto AMQPExchange::getType()
Get the exchange type */
PHP_METHOD(amqp_exchange_class, getType)
{
	amqp_exchange_object *exchange = AMQP_EXCHANGE_OBJ_P(getThis());

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	/* Check if there is a type to be had: */
	if (exchange->type_len) {
		RETURN_STRING(exchange->type);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto AMQPExchange::setType(string type)
Set the exchange type */
PHP_METHOD(amqp_exchange_class, setType)
{
	amqp_exchange_object *exchange = AMQP_EXCHANGE_OBJ_P(getThis());
	char *type = NULL;
	size_t type_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s", &type, &type_len) == FAILURE) {
		return;
	}

	AMQP_SET_TYPE(exchange, type);
}
/* }}} */

/* {{{ proto AMQPExchange::getArgument(string key)
Get the exchange argument referenced by key */
PHP_METHOD(amqp_exchange_class, getArgument)
{
	zval *tmp;
	amqp_exchange_object *exchange = AMQP_EXCHANGE_OBJ_P(getThis());
	char *key;
	size_t key_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &key_len) == FAILURE) {
		return;
	}

	if ((tmp = zend_hash_str_find(Z_ARRVAL(exchange->arguments), key, key_len)) == NULL) {
		RETURN_FALSE;
	}

	RETURN_ZVAL(tmp, 1, 0);
}
/* }}} */

/* {{{ proto AMQPExchange::getArguments
Get the exchange arguments */
PHP_METHOD(amqp_exchange_class, getArguments)
{
	amqp_exchange_object *exchange = AMQP_EXCHANGE_OBJ_P(getThis());

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_ZVAL(&exchange->arguments, 1, 0);
}
/* }}} */

/* {{{ proto AMQPExchange::setArguments(array args)
Overwrite all exchange arguments with given args */
PHP_METHOD(amqp_exchange_class, setArguments)
{
	zval *zvalArguments;
	amqp_exchange_object *exchange = AMQP_EXCHANGE_OBJ_P(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &zvalArguments) == FAILURE) {
		return;
	}

	/* Destroy the arguments storage */
	if (Z_DELREF(exchange->arguments) == 0) {
		zval_dtor(&exchange->arguments);
	}

	ZVAL_COPY(&exchange->arguments, zvalArguments);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto AMQPExchange::setArgument(key, value)
Get the exchange name */
PHP_METHOD(amqp_exchange_class, setArgument)
{
	zval *value;
	amqp_exchange_object *exchange = AMQP_EXCHANGE_OBJ_P(getThis());
	char *key;
	size_t key_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz", &key, &key_len, &value) == FAILURE) {
		return;
	}

	switch (Z_TYPE_P(value)) {
		case IS_NULL:
			zend_hash_str_del(Z_ARRVAL(exchange->arguments), key, key_len + 1);
			break;
		case IS_TRUE:
		case IS_FALSE:
		case IS_LONG:
		case IS_DOUBLE:
		case IS_STRING:
			add_assoc_zval(&exchange->arguments, key, value);
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
	amqp_exchange_object *exchange = AMQP_EXCHANGE_OBJ_P(getThis());
	amqp_channel_object *channel = AMQP_CHANNEL_OBJ(exchange->channel);
	amqp_connection_object *connection = Z_AMQP_CONNECTION_OBJ(channel->connection);
	amqp_table_t *arguments;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	AMQP_VERIFY_CHANNEL(channel, "Could not declare exchange.");
	AMQP_VERIFY_CONNECTION(connection, "Could not declare exchange.");

	/* Check that the exchange has a name */
	if (exchange->name_len < 1) {
		zend_throw_exception(amqp_exchange_exception_class_entry, "Could not declare exchange. Exchanges must have a name.", 0 TSRMLS_CC);
		return;
	}

	/* Check that the exchange has a name */
	if (exchange->type_len < 1) {
		zend_throw_exception(amqp_exchange_exception_class_entry, "Could not declare exchange. Exchanges must have a type.", 0 TSRMLS_CC);
		return;
	}

	arguments = convert_zval_to_amqp_table(&exchange->arguments TSRMLS_CC);
	
#if AMQP_VERSION_MAJOR * 100 + AMQP_VERSION_MINOR * 10 + AMQP_VERSION_PATCH > 52
	amqp_exchange_declare(
		connection->connection_resource->connection_state,
		channel->channel_id,
		amqp_cstring_bytes(exchange->name),
		amqp_cstring_bytes(exchange->type),
		IS_PASSIVE(exchange->flags),
		IS_DURABLE(exchange->flags),
		IS_AUTODELETE(exchange->flags),
		IS_INTERNAL(exchange->flags),
		*arguments
	);
#else
	amqp_exchange_declare(
		connection->connection_resource->connection_state,
		channel->channel_id,
		amqp_cstring_bytes(exchange->name),
		amqp_cstring_bytes(exchange->type),
		IS_PASSIVE(exchange->flags),
		IS_DURABLE(exchange->flags),
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
	amqp_exchange_object *exchange = AMQP_EXCHANGE_OBJ_P(getThis());
	amqp_channel_object *channel = AMQP_CHANNEL_OBJ(exchange->channel);
	amqp_connection_object *connection = Z_AMQP_CONNECTION_OBJ(channel->connection);;

	char *name = 0;
	size_t   name_len = 0;

	zend_long flags = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|sl", &name, &name_len, &flags) == FAILURE) {
		return;
	}

	AMQP_VERIFY_CHANNEL(channel, "Could not delete exchange.");
	AMQP_VERIFY_CONNECTION(connection, "Could not delete exchange.");

 	amqp_exchange_delete(
		connection->connection_resource->connection_state,
		channel->channel_id,
		amqp_cstring_bytes(name_len ? name : exchange->name),
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
	zval *pztmp;

	amqp_exchange_object *exchange = AMQP_EXCHANGE_OBJ_P(getThis());
	amqp_channel_object *channel = AMQP_CHANNEL_OBJ(exchange->channel);
	amqp_connection_object *connection = Z_AMQP_CONNECTION_OBJ(channel->connection);;

	char *key_name = NULL;
	size_t   key_len  = 0;

	char *msg = NULL;
	size_t   msg_len= 0;

	zend_long flags = AMQP_NOPARAM;

#ifndef PHP_WIN32
	/* Storage for previous signal handler during SIGPIPE override */
	void * old_handler;
#endif

	amqp_basic_properties_t props;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|sla", &msg, &msg_len, &key_name, &key_len, &flags, &ini_arr) == FAILURE) {
		return;
	}

	/* By default (and for BC) content type is text/plain (may be skipped at all, then set props._flags to 0) */
	props.content_type = amqp_cstring_bytes("text/plain");
	props._flags       = AMQP_BASIC_CONTENT_TYPE_FLAG;

	props.headers.entries = 0;

	if (ini_arr && (pztmp = zend_hash_str_find(HASH_OF(ini_arr), "content_type", sizeof("content_type")-1)) != NULL) {
		convert_to_string(pztmp);

		if (Z_STRLEN_P(pztmp) > 0) {
			props.content_type = php_amqp_zend_string(Z_STR_P(pztmp));
			props._flags |= AMQP_BASIC_CONTENT_TYPE_FLAG;
		}
	}

	if (ini_arr && (pztmp = zend_hash_str_find(HASH_OF(ini_arr), "content_encoding", sizeof("content_encoding")-1)) != NULL) {
		convert_to_string(pztmp);

		if (Z_STRLEN_P(pztmp) > 0) {
			props.content_encoding = php_amqp_zend_string(Z_STR_P(pztmp));
			props._flags |= AMQP_BASIC_CONTENT_ENCODING_FLAG;
		}
	}

	if (ini_arr && (pztmp = zend_hash_str_find(HASH_OF(ini_arr), "message_id", sizeof("message_id")-1)) != NULL) {
		convert_to_string(pztmp);

		if (Z_STRLEN_P(pztmp) > 0) {
			props.message_id = php_amqp_zend_string(Z_STR_P(pztmp));
			props._flags |= AMQP_BASIC_MESSAGE_ID_FLAG;
		}
	}

	if (ini_arr && (pztmp = zend_hash_str_find(HASH_OF(ini_arr), "user_id", sizeof("user_id")-1)) != NULL) {
		convert_to_string(pztmp);

		if (Z_STRLEN_P(pztmp) > 0) {
			props.user_id = php_amqp_zend_string(Z_STR_P(pztmp));
			props._flags |= AMQP_BASIC_USER_ID_FLAG;
		}
	}

	if (ini_arr && (pztmp = zend_hash_str_find(HASH_OF(ini_arr), "app_id", sizeof("app_id")-1)) != NULL) {
		convert_to_string(pztmp);

		if (Z_STRLEN_P(pztmp) > 0) {
			props.app_id = php_amqp_zend_string(Z_STR_P(pztmp));
			props._flags |= AMQP_BASIC_APP_ID_FLAG;
		}
	}

	if (ini_arr && (pztmp = zend_hash_str_find(HASH_OF(ini_arr), "delivery_mode", sizeof("delivery_mode")-1)) != NULL) {
		convert_to_long(pztmp);

		props.delivery_mode = (uint8_t)Z_LVAL_P(pztmp);
		props._flags |= AMQP_BASIC_DELIVERY_MODE_FLAG;
	}

	if (ini_arr && (pztmp = zend_hash_str_find(HASH_OF(ini_arr), "priority", sizeof("priority")-1)) != NULL) {
		convert_to_long(pztmp);

		props.priority = (uint8_t)Z_LVAL_P(pztmp);
		props._flags |= AMQP_BASIC_PRIORITY_FLAG;
	}

	if (ini_arr && (pztmp = zend_hash_str_find(HASH_OF(ini_arr), "timestamp", sizeof("timestamp")-1)) != NULL) {
		convert_to_long(pztmp);

		props.timestamp = (uint64_t)Z_LVAL_P(pztmp);
		props._flags |= AMQP_BASIC_TIMESTAMP_FLAG;
	}

	if (ini_arr && (pztmp = zend_hash_str_find(HASH_OF(ini_arr), "expiration", sizeof("expiration")-1)) != NULL) {
		convert_to_string(pztmp);

		if (Z_STRLEN_P(pztmp) > 0) {
			props.expiration = php_amqp_zend_string(Z_STR_P(pztmp));
			props._flags |= AMQP_BASIC_EXPIRATION_FLAG;
		}
	}

	if (ini_arr && (pztmp = zend_hash_str_find(HASH_OF(ini_arr), "type", sizeof("type")-1)) != NULL) {
		convert_to_string(pztmp);

		if (Z_STRLEN_P(pztmp) > 0) {
			props.type = php_amqp_zend_string(Z_STR_P(pztmp));
			props._flags |= AMQP_BASIC_TYPE_FLAG;
		}
	}

	if (ini_arr && (pztmp = zend_hash_str_find(HASH_OF(ini_arr), "reply_to", sizeof("reply_to")-1)) != NULL) {
		convert_to_string(pztmp);

		if (Z_STRLEN_P(pztmp) > 0) {
			props.reply_to = php_amqp_zend_string(Z_STR_P(pztmp));
			props._flags |= AMQP_BASIC_REPLY_TO_FLAG;
		}
	}

	if (ini_arr && (pztmp = zend_hash_str_find(HASH_OF(ini_arr), "correlation_id", sizeof("correlation_id")-1)) != NULL) {
		convert_to_string(pztmp);

		if (Z_STRLEN_P(pztmp) > 0) {
			props.correlation_id = php_amqp_zend_string(Z_STR_P(pztmp));
			props._flags |= AMQP_BASIC_CORRELATION_ID_FLAG;
		}
	}

	amqp_table_t* headers = NULL;

	if (ini_arr && (pztmp = zend_hash_str_find(HASH_OF(ini_arr), "headers", sizeof("headers")-1)) != NULL) {
		convert_to_array(pztmp);

		headers = convert_zval_to_amqp_table(pztmp TSRMLS_CC);

		props._flags |= AMQP_BASIC_HEADERS_FLAG;

		props.headers = *headers;
	}

	AMQP_VERIFY_CHANNEL(channel, "Could not publish to exchange.");
	AMQP_VERIFY_CONNECTION(connection, "Could not publish to exchange.");

#ifndef PHP_WIN32
	/* Start ignoring SIGPIPE */
	old_handler = signal(SIGPIPE, SIG_IGN);
#endif

	/* NOTE: basic.publish is asynchronous and thus will not indicate failure if something goes wrong on the broker */
	int status = amqp_basic_publish(
		connection->connection_resource->connection_state,
		channel->channel_id,
		(exchange->name_len > 0 ? amqp_cstring_bytes(exchange->name) : amqp_empty_bytes),	/* exchange */
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
	amqp_exchange_object *exchange = AMQP_EXCHANGE_OBJ_P(getThis());
	amqp_channel_object *channel = AMQP_CHANNEL_OBJ(exchange->channel);
	amqp_connection_object *connection;

	char *src_name;
	size_t   src_name_len = 0;

	char *keyname;
	size_t   keyname_len = 0;

	int flags;

	amqp_table_t *arguments = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|sa", &src_name, &src_name_len, &keyname, &keyname_len, &zvalArguments) == FAILURE) {
		return;
	}

	AMQP_VERIFY_CHANNEL(channel, "Could not bind to exchange.");

	connection = Z_AMQP_CONNECTION_OBJ(channel->connection);

	AMQP_VERIFY_CONNECTION(connection, "Could not bind to exchanges.");

	if (zvalArguments) {
		arguments = convert_zval_to_amqp_table(zvalArguments TSRMLS_CC);
	}

	amqp_exchange_bind(
		connection->connection_resource->connection_state,
		channel->channel_id,
		amqp_cstring_bytes(exchange->name),
		(src_name_len > 0 ? amqp_cstring_bytes(src_name) : amqp_empty_bytes),
		(keyname_len  > 0 ? amqp_cstring_bytes(keyname)  : amqp_empty_bytes),
		(zvalArguments ? *arguments : amqp_empty_table)
	);

	if (zvalArguments) {
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
	amqp_exchange_object *exchange = AMQP_EXCHANGE_OBJ_P(getThis());
	amqp_channel_object *channel = AMQP_CHANNEL_OBJ(exchange->channel);
	amqp_connection_object *connection;

	char *src_name;
	size_t   src_name_len = 0;

	char *keyname;
	size_t   keyname_len = 0;

	amqp_table_t *arguments = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|sa", &src_name, &src_name_len, &keyname, &keyname_len, &zvalArguments) == FAILURE) {
		return;
	}

	AMQP_VERIFY_CHANNEL(channel, "Could not unbind from exchange.");

	connection = Z_AMQP_CONNECTION_OBJ(channel->connection);

	AMQP_VERIFY_CONNECTION(connection, "Could not unbind from exchanges.");

	if (zvalArguments) {
		arguments = convert_zval_to_amqp_table(zvalArguments TSRMLS_CC);
	}

	amqp_exchange_unbind(
		connection->connection_resource->connection_state,
		channel->channel_id,
		amqp_cstring_bytes(exchange->name),
		(src_name_len > 0 ? amqp_cstring_bytes(src_name) : amqp_empty_bytes),
		(keyname_len  > 0 ? amqp_cstring_bytes(keyname)  : amqp_empty_bytes),
		(zvalArguments ? *arguments : amqp_empty_table)
	);

	if (zvalArguments) {
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
	amqp_exchange_object *exchange = AMQP_EXCHANGE_OBJ_P(getThis());

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_ZVAL(&exchange->channel, 1, 0);
}
/* }}} */

/* {{{ proto AMQPExchange::getConnection()
Get the AMQPConnection object in use */
PHP_METHOD(amqp_exchange_class, getConnection)
{
	amqp_exchange_object *exchange = AMQP_EXCHANGE_OBJ_P(getThis());
	amqp_channel_object *channel = AMQP_CHANNEL_OBJ(exchange->channel);

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_ZVAL(&channel->connection, 1, 0);
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
