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

void free_field_value(struct amqp_field_value_t_ value) {
	switch (value.kind) {
		case AMQP_FIELD_KIND_ARRAY:
			{
				int i;
				for (i=0; i<value.value.array.num_entries; ++i) {
					free_field_value(value.value.array.entries[i]);
				}
				efree(value.value.array.entries);
			}
			break;
		case AMQP_FIELD_KIND_TABLE:
			{
				int i;
				for (i=0; i<value.value.table.num_entries; ++i) {
					free_field_value(value.value.table.entries[i].value);
				}
				efree(value.value.table.entries);
			}
			break;
	}
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

	*return_value = *exchange->arguments;
	zval_copy_ctor(return_value);

	/* Increment the ref count */
	Z_ADDREF_P(exchange->arguments);
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

	arguments = convert_zval_to_arguments(exchange->arguments);
	
#if AMQP_VERSION_MAJOR == 0 && AMQP_VERSION_MINOR >= 5 && AMQP_VERSION_PATCH > 2
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

	AMQP_EFREE_ARGUMENTS(arguments);

	/* handle any errors that occured outside of signals */
	if (res.reply_type != AMQP_RESPONSE_NORMAL) {
		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, message, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_exchange_exception_class_entry, *message, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

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

		php_amqp_error(res, message, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_exchange_exception_class_entry, *message, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);
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
	zval** zdata;
	zval *ztmp;

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

	ALLOC_ZVAL(ztmp);

	{
	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "content_type", sizeof("content_type"), (void*)&zdata)) {
		MAKE_COPY_ZVAL(zdata, ztmp);
		convert_to_string(ztmp);

		if (Z_STRLEN_P(ztmp) > 0) {
			props.content_type = amqp_cstring_bytes((char *)Z_STRVAL_P(ztmp));
			props._flags |= AMQP_BASIC_CONTENT_TYPE_FLAG;
		}
	}

	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "content_encoding", sizeof("content_encoding"), (void*)&zdata)) {
		MAKE_COPY_ZVAL(zdata, ztmp);
		convert_to_string(ztmp);

		if (Z_STRLEN_P(ztmp) > 0) {
			props.content_encoding = amqp_cstring_bytes((char *)Z_STRVAL_P(ztmp));
			props._flags |= AMQP_BASIC_CONTENT_ENCODING_FLAG;
		}
	}

	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "message_id", sizeof("message_id"), (void*)&zdata)) {
		MAKE_COPY_ZVAL(zdata, ztmp);
		convert_to_string(ztmp);

		if (Z_STRLEN_P(ztmp) > 0) {
			props.message_id = amqp_cstring_bytes((char *)Z_STRVAL_P(ztmp));
			props._flags |= AMQP_BASIC_MESSAGE_ID_FLAG;
		}
	}

	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "user_id", sizeof("user_id"), (void*)&zdata)) {
		MAKE_COPY_ZVAL(zdata, ztmp);
		convert_to_string(ztmp);

		if (Z_STRLEN_P(ztmp) > 0) {
			props.user_id = amqp_cstring_bytes((char *)Z_STRVAL_P(ztmp));
			props._flags |= AMQP_BASIC_USER_ID_FLAG;
		}
	}

	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "app_id", sizeof("app_id"), (void*)&zdata)) {
		MAKE_COPY_ZVAL(zdata, ztmp);
		convert_to_string(ztmp);

		if (Z_STRLEN_P(ztmp) > 0) {
			props.app_id = amqp_cstring_bytes((char *)Z_STRVAL_P(ztmp));
			props._flags |= AMQP_BASIC_APP_ID_FLAG;
		}
	}

	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "delivery_mode", sizeof("delivery_mode"), (void*)&zdata)) {
		MAKE_COPY_ZVAL(zdata, ztmp);
		convert_to_long(ztmp);

		props.delivery_mode = (uint8_t)Z_LVAL_P(ztmp);
		props._flags |= AMQP_BASIC_DELIVERY_MODE_FLAG;
	}

	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "priority", sizeof("priority"), (void*)&zdata)) {
		MAKE_COPY_ZVAL(zdata, ztmp);
		convert_to_long(ztmp);

		props.priority = (uint8_t)Z_LVAL_P(ztmp);
		props._flags |= AMQP_BASIC_PRIORITY_FLAG;
	}

	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "timestamp", sizeof("timestamp"), (void*)&zdata)) {
		MAKE_COPY_ZVAL(zdata, ztmp);
		convert_to_long(ztmp);

		props.timestamp = (uint64_t)Z_LVAL_P(ztmp);
		props._flags |= AMQP_BASIC_TIMESTAMP_FLAG;
	}

	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "expiration", sizeof("expiration"), (void*)&zdata)) {
		MAKE_COPY_ZVAL(zdata, ztmp);
		convert_to_string(ztmp);

		if (Z_STRLEN_P(ztmp) > 0) {
			props.expiration = amqp_cstring_bytes((char *)Z_STRVAL_P(ztmp));
			props._flags |= AMQP_BASIC_EXPIRATION_FLAG;
		}
	}

	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "type", sizeof("type"), (void*)&zdata)) {
		MAKE_COPY_ZVAL(zdata, ztmp);
		convert_to_string(ztmp);

		if (Z_STRLEN_P(ztmp) > 0) {
			props.type = amqp_cstring_bytes((char *)Z_STRVAL_P(ztmp));
			props._flags |= AMQP_BASIC_TYPE_FLAG;
		}
	}

	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "reply_to", sizeof("reply_to"), (void*)&zdata)) {
		MAKE_COPY_ZVAL(zdata, ztmp);
		convert_to_string(ztmp);

		if (Z_STRLEN_P(ztmp) > 0) {
			props.reply_to = amqp_cstring_bytes((char *)Z_STRVAL_P(ztmp));
			props._flags |= AMQP_BASIC_REPLY_TO_FLAG;
		}
	}

	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "correlation_id", sizeof("correlation_id"), (void*)&zdata)) {
		MAKE_COPY_ZVAL(zdata, ztmp);
		convert_to_string(ztmp);

		if (Z_STRLEN_P(ztmp) > 0) {
			props.correlation_id = amqp_cstring_bytes((char *)Z_STRVAL_P(ztmp));
			props._flags |= AMQP_BASIC_CORRELATION_ID_FLAG;
		}
	}

	}

	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF(ini_arr), "headers", sizeof("headers"), (void*)&zdata)) {
		HashTable *headers;
		HashPosition pos;
		zval **z;

		MAKE_COPY_ZVAL(zdata, ztmp);

		convert_to_array(ztmp);

		headers = HASH_OF(ztmp);
		zend_hash_internal_pointer_reset_ex(headers, &pos);

		props._flags |= AMQP_BASIC_HEADERS_FLAG;
		props.headers.entries = emalloc(sizeof(struct amqp_table_entry_t_) * zend_hash_num_elements(headers));
		props.headers.num_entries = 0;

		/* TODO: merge codebase with convert_zval_to_arguments function (then also merge AMQP_EFREE_ARGUMENTS macro and free_field_value function) */
		while (zend_hash_get_current_data_ex(headers, (void **)&z, &pos) == SUCCESS) {
			char *string_key;
			uint  string_key_len;

			int	type;
			ulong  num_key;

			type = zend_hash_get_current_key_ex(headers, &string_key, &string_key_len, &num_key, 0, &pos);

			zend_hash_move_forward_ex(headers, &pos);

			if (HASH_KEY_IS_STRING == type) {
				props.headers.entries[props.headers.num_entries].key.bytes = string_key;
				props.headers.entries[props.headers.num_entries].key.len = string_key_len - 1;
			} else {
				/* previous version ignored non-string keys, so we just make notice about it */
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Ignoring non-string header field '%lu'", num_key);

				continue;
			}

			if (Z_TYPE_PP(z) == IS_STRING) {
				props.headers.entries[props.headers.num_entries].value.kind              = AMQP_FIELD_KIND_UTF8;
				props.headers.entries[props.headers.num_entries].value.value.bytes.bytes = Z_STRVAL_PP(z);
				props.headers.entries[props.headers.num_entries].value.value.bytes.len   = Z_STRLEN_PP(z);
				props.headers.num_entries++;
			} else if (Z_TYPE_PP(z) == IS_LONG) {
				props.headers.entries[props.headers.num_entries].value.kind      = AMQP_FIELD_KIND_I64;
				props.headers.entries[props.headers.num_entries].value.value.i64 = (int64_t) Z_LVAL_PP(z);
				props.headers.num_entries++;
			} else if (Z_TYPE_PP(z) == IS_DOUBLE) {
				props.headers.entries[props.headers.num_entries].value.kind      = AMQP_FIELD_KIND_F64;
				props.headers.entries[props.headers.num_entries].value.value.f64 = (double)Z_DVAL_PP(z);
				props.headers.num_entries++;
			} else if (Z_TYPE_PP(z) == IS_BOOL) {
				props.headers.entries[props.headers.num_entries].value.kind      = AMQP_FIELD_KIND_BOOLEAN;
				props.headers.entries[props.headers.num_entries].value.value.boolean = (amqp_boolean_t)Z_BVAL_PP(z);
				props.headers.num_entries++;
			} else if (Z_TYPE_PP(z) == IS_ARRAY) {
				zval **arr_data;
				amqp_array_t array;
				HashPosition arr_pos;

				array.entries     = emalloc(sizeof(struct amqp_field_value_t_) * zend_hash_num_elements(Z_ARRVAL_PP(z)));
				array.num_entries = 0;
				for(
					zend_hash_internal_pointer_reset_ex(Z_ARRVAL_PP(z), &arr_pos);
					zend_hash_get_current_data_ex(Z_ARRVAL_PP(z), (void**) &arr_data, &arr_pos) == SUCCESS;
					zend_hash_move_forward_ex(Z_ARRVAL_PP(z), &arr_pos)
				) {
					if (Z_TYPE_PP(arr_data) == IS_STRING) {
						array.entries[array.num_entries].kind              = AMQP_FIELD_KIND_UTF8;
						array.entries[array.num_entries].value.bytes.bytes = Z_STRVAL_PP(arr_data);
						array.entries[array.num_entries].value.bytes.len   = Z_STRLEN_PP(arr_data);
						array.num_entries ++;
					} else {
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Ignoring non-string header nested array member type %d for field '%s'", Z_TYPE_PP(arr_data), string_key);
					}
				}

				props.headers.entries[props.headers.num_entries].value.kind = AMQP_FIELD_KIND_ARRAY;
				props.headers.entries[props.headers.num_entries].value.value.array = array;
				props.headers.num_entries++;
			} else {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Ignoring header field '%s' due to unsupported value type (NULL, array or resource)", string_key);
			}
		}
	} else {
		props.headers.entries = 0;
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
		(msg_len > 0 ? amqp_cstring_bytes(msg) : amqp_empty_bytes) /* message body */
	);

	if (props.headers.entries) {
		int i;
		for (i=0; i < props.headers.num_entries; ++i) {
			free_field_value(props.headers.entries[i].value);
		}
		efree(props.headers.entries);
	}

	FREE_ZVAL(ztmp);

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

		php_amqp_error(res, message, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_queue_exception_class_entry, *message, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

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
		arguments = convert_zval_to_arguments(zvalArguments);
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
		AMQP_EFREE_ARGUMENTS(arguments);
	}

	amqp_rpc_reply_t res = amqp_get_rpc_reply(connection->connection_resource->connection_state);

	if (res.reply_type != AMQP_RESPONSE_NORMAL) {
		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, message, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_exchange_exception_class_entry, *message, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

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
		arguments = convert_zval_to_arguments(zvalArguments);
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
		AMQP_EFREE_ARGUMENTS(arguments);
	}

	amqp_rpc_reply_t res = amqp_get_rpc_reply(connection->connection_resource->connection_state);

	if (res.reply_type != AMQP_RESPONSE_NORMAL) {
		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, message, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_exchange_exception_class_entry, *message, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

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
