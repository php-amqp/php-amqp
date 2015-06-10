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


#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3
zend_object_handlers amqp_exchange_object_handlers;
HashTable *amqp_exchange_object_get_debug_info(zval *object, int *is_temp TSRMLS_DC) {
	zval *value;
	HashTable *debug_info;

	/* Get the envelope object from which to read */
	amqp_exchange_object *exchange = (amqp_exchange_object *)zend_object_store_get_object(object TSRMLS_CC);

	/* Let zend clean up for us: */
	*is_temp = 1;

	/* Keep the first number matching the number of entries in this table*/
	ALLOC_HASHTABLE(debug_info);
	ZEND_INIT_SYMTABLE_EX(debug_info, 6 + 1, 0);

	/* Start adding values */
	MAKE_STD_ZVAL(value);
	ZVAL_STRINGL(value, exchange->name, strlen(exchange->name), 1);
	zend_hash_add(debug_info, "name", sizeof("name"), &value, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(value);
	ZVAL_STRINGL(value, exchange->type, strlen(exchange->type), 1);
	zend_hash_add(debug_info, "type", sizeof("type"), &value, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(value);
	ZVAL_BOOL(value, IS_PASSIVE(exchange->flags));
	zend_hash_add(debug_info, "passive", sizeof("passive"), &value, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(value);
	ZVAL_BOOL(value, IS_DURABLE(exchange->flags));
	zend_hash_add(debug_info, "durable", sizeof("durable"), &value, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(value);
	ZVAL_BOOL(value, IS_AUTODELETE(exchange->flags));
	zend_hash_add(debug_info, "auto_delete", sizeof("auto_delete"), &value, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(value);
	ZVAL_BOOL(value, IS_INTERNAL(exchange->flags));
	zend_hash_add(debug_info, "internal", sizeof("internal"), &value, sizeof(zval *), NULL);

	Z_ADDREF_P(exchange->arguments);
	zend_hash_add(debug_info, "arguments", sizeof("arguments"), &exchange->arguments, sizeof(&exchange->arguments), NULL);

	/* Start adding values */
	return debug_info;
}
#endif

void amqp_exchange_dtor(void *object TSRMLS_DC)
{
	amqp_exchange_object *exchange = (amqp_exchange_object*)object;

	/* Destroy the connection object */
	if (exchange->channel) {
		zval_ptr_dtor(&exchange->channel);
	}

	if (exchange->arguments) {
		zval_ptr_dtor(&exchange->arguments);
	}

	zend_object_std_dtor(&exchange->zo TSRMLS_CC);

	efree(object);
}

zend_object_value amqp_exchange_ctor(zend_class_entry *ce TSRMLS_DC)
{
	zend_object_value new_value;
	amqp_exchange_object* exchange = (amqp_exchange_object*)emalloc(sizeof(amqp_exchange_object));

	memset(exchange, 0, sizeof(amqp_exchange_object));

	/* Initialize the arguments array: */
	MAKE_STD_ZVAL(exchange->arguments);
	array_init(exchange->arguments);

	zend_object_std_init(&exchange->zo, ce TSRMLS_CC);
	AMQP_OBJECT_PROPERTIES_INIT(exchange->zo, ce);

	new_value.handle = zend_objects_store_put(
		exchange,
		(zend_objects_store_dtor_t)zend_objects_destroy_object,
		(zend_objects_free_object_storage_t)amqp_exchange_dtor,
		NULL TSRMLS_CC
	);

#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3
	memcpy((void *)&amqp_exchange_object_handlers, (void *)zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	amqp_exchange_object_handlers.get_debug_info = amqp_exchange_object_get_debug_info;
	new_value.handlers = &amqp_exchange_object_handlers;
#else
	new_value.handlers = zend_get_std_object_handlers();
#endif

	return new_value;
}

/* {{{ proto AMQPExchange::__construct(AMQPChannel channel);
create Exchange   */
PHP_METHOD(amqp_exchange_class, __construct)
{
	zval *id;
	zval *channelObj;
	amqp_exchange_object *exchange;
	amqp_channel_object *channel;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "OO", &id, amqp_exchange_class_entry, &channelObj, amqp_channel_class_entry) == FAILURE) {
		zend_throw_exception(amqp_exchange_exception_class_entry, "Parameter must be an instance of AMQPChannel.", 0 TSRMLS_CC);
		RETURN_NULL();
	}

	if (!instanceof_function(Z_OBJCE_P(channelObj), amqp_channel_class_entry TSRMLS_CC)) {
		zend_throw_exception(amqp_exchange_exception_class_entry, "The first parameter must be and instance of AMQPChannel.", 0 TSRMLS_CC);
		return;
	}

	exchange = (amqp_exchange_object *)zend_object_store_get_object(id TSRMLS_CC);

	exchange->channel = channelObj;

	/* Increment the ref count */
	Z_ADDREF_P(channelObj);

	/* Pull the channel out */
	channel = AMQP_GET_CHANNEL(exchange);
	AMQP_VERIFY_CHANNEL(channel, "Could not create exchange.");
}
/* }}} */


/* {{{ proto AMQPExchange::getName()
Get the exchange name */
PHP_METHOD(amqp_exchange_class, getName)
{
	zval *id;
	amqp_exchange_object *exchange;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_exchange_class_entry) == FAILURE) {
		return;
	}

	exchange = (amqp_exchange_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Check if there is a name to be had: */
	if (exchange->name_len) {
		RETURN_STRING(exchange->name, 1);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */


/* {{{ proto AMQPExchange::setName(string name)
Set the exchange name */
PHP_METHOD(amqp_exchange_class, setName)
{
	zval *id;
	amqp_exchange_object *exchange;
	char *name = NULL;
	int name_len = 0;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os", &id, amqp_exchange_class_entry, &name, &name_len) == FAILURE) {
		return;
	}

	/* Pull the exchange off the object store */
	exchange = (amqp_exchange_object *)zend_object_store_get_object(id TSRMLS_CC);

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
	zval *id;
	amqp_exchange_object *exchange;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_exchange_class_entry) == FAILURE) {
		return;
	}

	exchange = (amqp_exchange_object *)zend_object_store_get_object(id TSRMLS_CC);

	RETURN_LONG(exchange->flags);
}
/* }}} */


/* {{{ proto AMQPExchange::setFlags(long bitmask)
Set the exchange parameters */
PHP_METHOD(amqp_exchange_class, setFlags)
{
	zval *id;
	amqp_exchange_object *exchange;
	long flagBitmask;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ol", &id, amqp_exchange_class_entry, &flagBitmask) == FAILURE) {
		return;
	}

	/* Pull the exchange off the object store */
	exchange = (amqp_exchange_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Set the flags based on the bitmask we were given */
	exchange->flags = flagBitmask ? flagBitmask & PHP_AMQP_EXCHANGE_FLAGS : flagBitmask;
}
/* }}} */


/* {{{ proto AMQPExchange::getType()
Get the exchange type */
PHP_METHOD(amqp_exchange_class, getType)
{
	zval *id;
	amqp_exchange_object *exchange;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_exchange_class_entry) == FAILURE) {
		return;
	}

	exchange = (amqp_exchange_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Check if there is a type to be had: */
	if (exchange->type_len) {
		RETURN_STRING(exchange->type, 1);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */


/* {{{ proto AMQPExchange::setType(string type)
Set the exchange type */
PHP_METHOD(amqp_exchange_class, setType)
{
	zval *id;
	amqp_exchange_object *exchange;
	char *type = NULL;
	int type_len = 0;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os", &id, amqp_exchange_class_entry, &type, &type_len) == FAILURE) {
		return;
	}

	/* Pull the exchange off the object store */
	exchange = (amqp_exchange_object *)zend_object_store_get_object(id TSRMLS_CC);

	AMQP_SET_TYPE(exchange, type);
}
/* }}} */


/* {{{ proto AMQPExchange::getArgument(string key)
Get the exchange argument referenced by key */
PHP_METHOD(amqp_exchange_class, getArgument)
{
	zval *id;
	zval **tmp;
	amqp_exchange_object *exchange;
	char *key;
	int key_len;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os", &id, amqp_exchange_class_entry, &key, &key_len) == FAILURE) {
		return;
	}

	exchange = (amqp_exchange_object *)zend_object_store_get_object(id TSRMLS_CC);

	if (zend_hash_find(Z_ARRVAL_P(exchange->arguments), key, key_len + 1, (void **)&tmp) == FAILURE) {
		RETURN_FALSE;
	}

	*return_value = **tmp;

	zval_copy_ctor(return_value);
	INIT_PZVAL(return_value);
}
/* }}} */


/* {{{ proto AMQPExchange::getArguments
Get the exchange arguments */
PHP_METHOD(amqp_exchange_class, getArguments)
{
	zval *id;
	amqp_exchange_object *exchange;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_exchange_class_entry) == FAILURE) {
		return;
	}

	exchange = (amqp_exchange_object *)zend_object_store_get_object(id TSRMLS_CC);

	zval_dtor(return_value);
	MAKE_COPY_ZVAL(&exchange->arguments, return_value);
}
/* }}} */


/* {{{ proto AMQPExchange::setArguments(array args)
Overwrite all exchange arguments with given args */
PHP_METHOD(amqp_exchange_class, setArguments)
{
	zval *id, *zvalArguments;
	amqp_exchange_object *exchange;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Oa", &id, amqp_exchange_class_entry, &zvalArguments) == FAILURE) {
		return;
	}

	/* Pull the exchange off the object store */
	exchange = (amqp_exchange_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Destroy the arguments storage */
	if (exchange->arguments) {
		zval_ptr_dtor(&exchange->arguments);
	}

	exchange->arguments = zvalArguments;

	/* Increment the ref count */
	Z_ADDREF_P(exchange->arguments);

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto AMQPExchange::setArgument(key, value)
Get the exchange name */
PHP_METHOD(amqp_exchange_class, setArgument)
{
	zval *id, *value;
	amqp_exchange_object *exchange;
	char *key;
	int key_len;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Osz", &id, amqp_exchange_class_entry, &key, &key_len, &value) == FAILURE) {
		return;
	}

	/* Pull the exchange off the object store */
	exchange = (amqp_exchange_object *)zend_object_store_get_object(id TSRMLS_CC);

	switch (Z_TYPE_P(value)) {
		case IS_NULL:
			zend_hash_del_key_or_index(Z_ARRVAL_P(exchange->arguments), key, key_len + 1, 0, HASH_DEL_KEY);
			break;
		case IS_BOOL:
		case IS_LONG:
		case IS_DOUBLE:
		case IS_STRING:
			add_assoc_zval(exchange->arguments, key, value);
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
	zval *id;

	amqp_exchange_object *exchange;
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	amqp_table_t *arguments;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_exchange_class_entry) == FAILURE) {
		return;
	}

	exchange = (amqp_exchange_object *)zend_object_store_get_object(id TSRMLS_CC);

	AMQP_ASSIGN_CHANNEL(channel, exchange);
	AMQP_VERIFY_CHANNEL(channel, "Could not declare exchange.");

	connection = AMQP_GET_CONNECTION(channel);
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

	arguments = convert_zval_to_amqp_table(exchange->arguments TSRMLS_CC);
	
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
	zval *id;

	amqp_exchange_object *exchange;
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	char *name = 0;
	int   name_len = 0;

	long flags = 0;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O|sl", &id, amqp_exchange_class_entry, &name, &name_len, &flags) == FAILURE) {
		return;
	}

	exchange = (amqp_exchange_object *)zend_object_store_get_object(id TSRMLS_CC);

	channel = AMQP_GET_CHANNEL(exchange);
	AMQP_VERIFY_CHANNEL(channel, "Could not delete exchange.");

	connection = AMQP_GET_CONNECTION(channel);
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
	zval *id;
	zval *ini_arr = NULL;
	zval** ppztmp;
	zval *ini_arr_tmp = NULL;


	amqp_exchange_object *exchange;
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	char *key_name = NULL;
	int   key_len  = 0;

	char *msg;
	int   msg_len= 0;

	long flags = AMQP_NOPARAM;

#ifndef PHP_WIN32
	/* Storage for previous signal handler during SIGPIPE override */
	void * old_handler;
#endif

	amqp_basic_properties_t props;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os|sla", &id, amqp_exchange_class_entry, &msg, &msg_len, &key_name, &key_len, &flags, &ini_arr) == FAILURE) {
		return;
	}

	exchange = (amqp_exchange_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* By default (and for BC) content type is text/plain (may be skipped at all, then set props._flags to 0) */
	props.content_type = amqp_cstring_bytes("text/plain");
	props._flags       = AMQP_BASIC_CONTENT_TYPE_FLAG;

	props.headers.entries = 0;

	if (ini_arr) {
		ALLOC_ZVAL(ini_arr_tmp);
		MAKE_COPY_ZVAL(&ini_arr, ini_arr_tmp);
	}

	{
	if (ini_arr_tmp && SUCCESS == zend_hash_find(HASH_OF (ini_arr_tmp), "content_type", sizeof("content_type"), (void*)&ppztmp)) {
		convert_to_string(*ppztmp);

		if (Z_STRLEN_PP(ppztmp) > 0) {
			props.content_type = amqp_cstring_bytes((char *)Z_STRVAL_PP(ppztmp));
			props._flags |= AMQP_BASIC_CONTENT_TYPE_FLAG;
		}
	}

	if (ini_arr_tmp && SUCCESS == zend_hash_find(HASH_OF (ini_arr_tmp), "content_encoding", sizeof("content_encoding"), (void*)&ppztmp)) {
		convert_to_string(*ppztmp);

		if (Z_STRLEN_PP(ppztmp) > 0) {
			props.content_encoding = amqp_cstring_bytes((char *)Z_STRVAL_PP(ppztmp));
			props._flags |= AMQP_BASIC_CONTENT_ENCODING_FLAG;
		}
	}

	if (ini_arr_tmp && SUCCESS == zend_hash_find(HASH_OF (ini_arr_tmp), "message_id", sizeof("message_id"), (void*)&ppztmp)) {
		convert_to_string(*ppztmp);

		if (Z_STRLEN_PP(ppztmp) > 0) {
			props.message_id = amqp_cstring_bytes((char *)Z_STRVAL_PP(ppztmp));
			props._flags |= AMQP_BASIC_MESSAGE_ID_FLAG;
		}
	}

	if (ini_arr_tmp && SUCCESS == zend_hash_find(HASH_OF (ini_arr_tmp), "user_id", sizeof("user_id"), (void*)&ppztmp)) {
		convert_to_string(*ppztmp);

		if (Z_STRLEN_PP(ppztmp) > 0) {
			props.user_id = amqp_cstring_bytes((char *)Z_STRVAL_PP(ppztmp));
			props._flags |= AMQP_BASIC_USER_ID_FLAG;
		}
	}

	if (ini_arr_tmp && SUCCESS == zend_hash_find(HASH_OF (ini_arr_tmp), "app_id", sizeof("app_id"), (void*)&ppztmp)) {
		convert_to_string(*ppztmp);

		if (Z_STRLEN_PP(ppztmp) > 0) {
			props.app_id = amqp_cstring_bytes((char *)Z_STRVAL_PP(ppztmp));
			props._flags |= AMQP_BASIC_APP_ID_FLAG;
		}
	}

	if (ini_arr_tmp && SUCCESS == zend_hash_find(HASH_OF (ini_arr_tmp), "delivery_mode", sizeof("delivery_mode"), (void*)&ppztmp)) {
		convert_to_long(*ppztmp);

		props.delivery_mode = (uint8_t)Z_LVAL_PP(ppztmp);
		props._flags |= AMQP_BASIC_DELIVERY_MODE_FLAG;
	}

	if (ini_arr_tmp && SUCCESS == zend_hash_find(HASH_OF (ini_arr_tmp), "priority", sizeof("priority"), (void*)&ppztmp)) {
		convert_to_long(*ppztmp);

		props.priority = (uint8_t)Z_LVAL_PP(ppztmp);
		props._flags |= AMQP_BASIC_PRIORITY_FLAG;
	}

	if (ini_arr_tmp && SUCCESS == zend_hash_find(HASH_OF (ini_arr_tmp), "timestamp", sizeof("timestamp"), (void*)&ppztmp)) {
		convert_to_long(*ppztmp);

		props.timestamp = (uint64_t)Z_LVAL_PP(ppztmp);
		props._flags |= AMQP_BASIC_TIMESTAMP_FLAG;
	}

	if (ini_arr_tmp && SUCCESS == zend_hash_find(HASH_OF (ini_arr_tmp), "expiration", sizeof("expiration"), (void*)&ppztmp)) {
		convert_to_string(*ppztmp);

		if (Z_STRLEN_PP(ppztmp) > 0) {
			props.expiration = amqp_cstring_bytes((char *)Z_STRVAL_PP(ppztmp));
			props._flags |= AMQP_BASIC_EXPIRATION_FLAG;
		}
	}

	if (ini_arr_tmp && SUCCESS == zend_hash_find(HASH_OF (ini_arr_tmp), "type", sizeof("type"), (void*)&ppztmp)) {
		convert_to_string(*ppztmp);

		if (Z_STRLEN_PP(ppztmp) > 0) {
			props.type = amqp_cstring_bytes((char *)Z_STRVAL_PP(ppztmp));
			props._flags |= AMQP_BASIC_TYPE_FLAG;
		}
	}

	if (ini_arr_tmp && SUCCESS == zend_hash_find(HASH_OF (ini_arr_tmp), "reply_to", sizeof("reply_to"), (void*)&ppztmp)) {
		convert_to_string(*ppztmp);

		if (Z_STRLEN_PP(ppztmp) > 0) {
			props.reply_to = amqp_cstring_bytes((char *)Z_STRVAL_PP(ppztmp));
			props._flags |= AMQP_BASIC_REPLY_TO_FLAG;
		}
	}

	if (ini_arr_tmp && SUCCESS == zend_hash_find(HASH_OF (ini_arr_tmp), "correlation_id", sizeof("correlation_id"), (void*)&ppztmp)) {
		convert_to_string(*ppztmp);

		if (Z_STRLEN_PP(ppztmp) > 0) {
			props.correlation_id = amqp_cstring_bytes((char *)Z_STRVAL_PP(ppztmp));
			props._flags |= AMQP_BASIC_CORRELATION_ID_FLAG;
		}
	}

	}

	amqp_table_t *headers = NULL;

	if (ini_arr_tmp && SUCCESS == zend_hash_find(HASH_OF(ini_arr_tmp), "headers", sizeof("headers"), (void*)&ppztmp)) {
		convert_to_array(*ppztmp);

		headers = convert_zval_to_amqp_table(*ppztmp TSRMLS_CC);

		props._flags |= AMQP_BASIC_HEADERS_FLAG;
		props.headers = *headers;
	}

	channel = AMQP_GET_CHANNEL(exchange);
	AMQP_VERIFY_CHANNEL(channel, "Could not publish to exchange.");

	connection = AMQP_GET_CONNECTION(channel);
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

	if (ini_arr_tmp) {
		zval_dtor(ini_arr_tmp);
		FREE_ZVAL(ini_arr_tmp);
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
	zval *id, *zvalArguments = NULL;
	amqp_exchange_object *exchange;
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	char *src_name;
	int   src_name_len = 0;

	char *keyname;
	int   keyname_len = 0;

	int flags;

	amqp_table_t *arguments;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os|sa", &id, amqp_exchange_class_entry, &src_name, &src_name_len, &keyname, &keyname_len, &zvalArguments) == FAILURE) {
		return;
	}

	exchange = (amqp_exchange_object *)zend_object_store_get_object(id TSRMLS_CC);

	channel = AMQP_GET_CHANNEL(exchange);
	AMQP_VERIFY_CHANNEL(channel, "Could not bind to exchange.");

	connection = AMQP_GET_CONNECTION(channel);
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
	zval *id, *zvalArguments = NULL;
	amqp_exchange_object *exchange;
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	char *src_name;
	int   src_name_len = 0;

	char *keyname;
	int   keyname_len = 0;

	int flags;

	amqp_table_t *arguments;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os|sa", &id, amqp_exchange_class_entry, &src_name, &src_name_len, &keyname, &keyname_len, &zvalArguments) == FAILURE) {
		return;
	}

	exchange = (amqp_exchange_object *)zend_object_store_get_object(id TSRMLS_CC);

	channel = AMQP_GET_CHANNEL(exchange);
	AMQP_VERIFY_CHANNEL(channel, "Could not unbind from exchange.");

	connection = AMQP_GET_CONNECTION(channel);
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
	zval *id;
	amqp_exchange_object *exchange;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_exchange_class_entry) == FAILURE) {
		return;
	}

	exchange = (amqp_exchange_object *)zend_object_store_get_object(id TSRMLS_CC);

	RETURN_ZVAL(exchange->channel, 1, 0);
}
/* }}} */

/* {{{ proto AMQPExchange::getConnection()
Get the AMQPConnection object in use */
PHP_METHOD(amqp_exchange_class, getConnection)
{
	zval *id;
	amqp_exchange_object *exchange;
	amqp_channel_object *channel;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_exchange_class_entry) == FAILURE) {
		return;
	}

	exchange = (amqp_exchange_object *)zend_object_store_get_object(id TSRMLS_CC);
	channel = AMQP_GET_CHANNEL(exchange);

	RETURN_ZVAL(channel->connection, 1, 0);
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
