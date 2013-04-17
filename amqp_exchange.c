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
	ZEND_INIT_SYMTABLE_EX(debug_info, 5 + 1, 0);

	/* Start adding values */
	MAKE_STD_ZVAL(value);
	ZVAL_STRINGL(value, exchange->name, strlen(exchange->name), 1);
	zend_hash_add(debug_info, "name", sizeof("name"), &value, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(value);
	ZVAL_STRINGL(value, exchange->type, strlen(exchange->type), 1);
	zend_hash_add(debug_info, "type", sizeof("type"), &value, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(value);
	ZVAL_LONG(value, exchange->passive);
	zend_hash_add(debug_info, "passive", sizeof("passive"), &value, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(value);
	ZVAL_LONG(value, exchange->durable);
	zend_hash_add(debug_info, "durable", sizeof("durable"), &value, sizeof(zval *), NULL);

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
		zend_throw_exception(amqp_exchange_exception_class_entry, "Parameter must be an instance of AMQPConnection.", 0 TSRMLS_CC);
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

	/* We have a valid connection: */
	exchange->is_connected = '\1';
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
	if (name_len < 1 || name_len > 255) {
		zend_throw_exception(amqp_exchange_exception_class_entry, "Invalid exchange name given, must be between 1 and 255 characters long.", 0 TSRMLS_CC);
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
	long flagBitmask = 0;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_exchange_class_entry) == FAILURE) {
		return;
	}

	exchange = (amqp_exchange_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Set the bitmask based on what is set in the exchange */
	flagBitmask |= (exchange->passive ? AMQP_PASSIVE : 0);
	flagBitmask |= (exchange->durable ? AMQP_DURABLE : 0);

	RETURN_LONG(flagBitmask);
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
	exchange->passive = IS_PASSIVE(flagBitmask);
	exchange->durable = IS_DURABLE(flagBitmask);
    exchange->auto_delete = IS_AUTODELETE(flagBitmask);
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
	AMQP_SET_TYPE(exchange, type)
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

	amqp_rpc_reply_t res;

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
	amqp_exchange_declare(
		connection->connection_resource->connection_state,
		channel->channel_id,
		amqp_cstring_bytes(exchange->name),
		amqp_cstring_bytes(exchange->type),
		exchange->passive,
		exchange->durable,
		*arguments
	);

	res = AMQP_RPC_REPLY_T_CAST amqp_get_rpc_reply(connection->connection_resource->connection_state);

	AMQP_EFREE_ARGUMENTS(arguments);

	/* handle any errors that occured outside of signals */
	if (res.reply_type != AMQP_RESPONSE_NORMAL) {
		char str[256];
		char ** pstr = (char **) &str;
		amqp_error(res, pstr);
		zend_throw_exception(amqp_exchange_exception_class_entry, *pstr, 0 TSRMLS_CC);
		efree(*pstr);
		return;
	}

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
	int name_len = 0;
	long flags = 0;

	amqp_rpc_reply_t res;
	amqp_exchange_delete_t s;
	amqp_method_number_t method_ok = AMQP_EXCHANGE_DELETE_OK_METHOD;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O|sl", &id, amqp_exchange_class_entry, &name, &name_len, &flags) == FAILURE) {
		return;
	}

	exchange = (amqp_exchange_object *)zend_object_store_get_object(id TSRMLS_CC);

	if (name_len) {
		AMQP_SET_NAME(exchange, name);
		s.ticket = 0;
		s.exchange.len = name_len;
		s.exchange.bytes = name;
		s.if_unused = (AMQP_IFUNUSED & flags) ? 1 : 0;
		s.nowait = 0;
	} else {
		s.ticket = 0;
		s.exchange.len = exchange->name_len;
		s.exchange.bytes = exchange->name;
		s.if_unused = (AMQP_IFUNUSED & flags) ? 1 : 0;
		s.nowait = 0;
	}

	channel = AMQP_GET_CHANNEL(exchange);
	AMQP_VERIFY_CHANNEL(channel, "Could not declare exchange.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not declare exchange.");

	res = amqp_simple_rpc(
		connection->connection_resource->connection_state,
		channel->channel_id,
		AMQP_EXCHANGE_DELETE_METHOD,
		&method_ok, &s
	);

	if (res.reply_type != AMQP_RESPONSE_NORMAL) {
		char str[256];
		char ** pstr = (char **) &str;
		amqp_error(res, pstr);
		zend_throw_exception(amqp_exchange_exception_class_entry, *pstr, 0 TSRMLS_CC);
		amqp_maybe_release_buffers(connection->connection_resource->connection_state);
		return;
	}
	amqp_maybe_release_buffers(connection->connection_resource->connection_state);

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
	amqp_exchange_object *exchange;
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	char *key_name = NULL;
	int key_len = 0;

	char *msg;
	int msg_len= 0;

	long flags = AMQP_NOPARAM;

#ifndef PHP_WIN32
	/* Storage for previous signal handler during SIGPIPE override */
	void * old_handler;
#endif

	amqp_basic_properties_t props;

	int r;
	amqp_bytes_t abt0, abt1, abt2;


	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os|sla", &id, amqp_exchange_class_entry, &msg, &msg_len, &key_name, &key_len, &flags, &ini_arr) == FAILURE) {
		return;
	}

	exchange = (amqp_exchange_object *)zend_object_store_get_object(id TSRMLS_CC);

	if (exchange->name_len < 0) {
		zend_throw_exception(amqp_exchange_exception_class_entry, "Could not publish to exchange. Exchange name not set.", 0 TSRMLS_CC);
		return;
	}

	props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG;

	zdata = NULL;
	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "content_type", sizeof("content_type"), (void*)&zdata)) {
		convert_to_string(*zdata);
	}
	if (zdata && strlen(Z_STRVAL_PP(zdata)) > 0) {
		props.content_type = amqp_cstring_bytes((char *)Z_STRVAL_PP(zdata));
	} else {
		props.content_type = amqp_cstring_bytes("text/plain");
	}

	zdata = NULL;
	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "content_encoding", sizeof("content_encoding"), (void*)&zdata)) {
		convert_to_string(*zdata);
	}
	if (zdata && strlen(Z_STRVAL_PP(zdata)) > 0) {
		props.content_encoding = amqp_cstring_bytes((char *)Z_STRVAL_PP(zdata));
		props._flags += AMQP_BASIC_CONTENT_ENCODING_FLAG;
	}

	zdata = NULL;
	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "message_id", sizeof("message_id"), (void*)&zdata)) {
		convert_to_string(*zdata);
	}
	if (zdata && strlen(Z_STRVAL_PP(zdata)) > 0) {
		props.message_id = amqp_cstring_bytes((char *)Z_STRVAL_PP(zdata));
		props._flags += AMQP_BASIC_MESSAGE_ID_FLAG;
	}

	zdata = NULL;
	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "user_id", sizeof("user_id"), (void*)&zdata)) {
		convert_to_string(*zdata);
	}
	if (zdata && strlen(Z_STRVAL_PP(zdata)) > 0) {
		props.user_id = amqp_cstring_bytes((char *)Z_STRVAL_PP(zdata));
		props._flags += AMQP_BASIC_USER_ID_FLAG;
	}

	zdata = NULL;
	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "app_id", sizeof("app_id"), (void*)&zdata)) {
		convert_to_string(*zdata);
	}
	if (zdata && strlen(Z_STRVAL_PP(zdata)) > 0) {
		props.app_id = amqp_cstring_bytes((char *)Z_STRVAL_PP(zdata));
		props._flags += AMQP_BASIC_APP_ID_FLAG;
	}

	zdata = NULL;
	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "delivery_mode", sizeof("delivery_mode"), (void*)&zdata)) {
		convert_to_long(*zdata);
	}
	if (zdata) {
		props.delivery_mode = (uint8_t)Z_LVAL_PP(zdata);
		props._flags += AMQP_BASIC_DELIVERY_MODE_FLAG;
	}

	zdata = NULL;
	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "priority", sizeof("priority"), (void*)&zdata)) {
		convert_to_long(*zdata);
	}
	if (zdata) {
		props.priority = (uint8_t)Z_LVAL_PP(zdata);
		props._flags += AMQP_BASIC_PRIORITY_FLAG;
	}

	zdata = NULL;
	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "timestamp", sizeof("timestamp"), (void*)&zdata)) {
		convert_to_long(*zdata);
	}
	if (zdata) {
		props.timestamp = (uint64_t)Z_LVAL_PP(zdata);
		props._flags += AMQP_BASIC_TIMESTAMP_FLAG;
	}

	zdata = NULL;
	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "expiration", sizeof("expiration"), (void*)&zdata)) {
		convert_to_string(*zdata);
	}
	if (zdata && strlen(Z_STRVAL_PP(zdata)) > 0) {
		props.expiration =	amqp_cstring_bytes((char *)Z_STRVAL_PP(zdata));
		props._flags += AMQP_BASIC_EXPIRATION_FLAG;
	}

	zdata = NULL;
	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "type", sizeof("type"), (void*)&zdata)) {
		convert_to_string(*zdata);
	}
	if (zdata && strlen(Z_STRVAL_PP(zdata)) > 0) {
		props.type =  amqp_cstring_bytes((char *)Z_STRVAL_PP(zdata));
		props._flags += AMQP_BASIC_TYPE_FLAG;
	}

	zdata = NULL;
	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "reply_to", sizeof("reply_to"), (void*)&zdata)) {
		convert_to_string(*zdata);
	}
	if (zdata && strlen(Z_STRVAL_PP(zdata)) > 0) {
		props.reply_to = amqp_cstring_bytes((char *)Z_STRVAL_PP(zdata));
		props._flags += AMQP_BASIC_REPLY_TO_FLAG;
	}

	zdata = NULL;
	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "correlation_id", sizeof("correlation_id"), (void*)&zdata)) {
		convert_to_string(*zdata);
	}
	if (zdata && strlen(Z_STRVAL_PP(zdata)) > 0) {
		props.correlation_id = amqp_cstring_bytes((char *)Z_STRVAL_PP(zdata));
		props._flags += AMQP_BASIC_CORRELATION_ID_FLAG;
	}

	zdata = NULL;
	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF(ini_arr), "headers", sizeof("headers"), (void*)&zdata)) {
		HashTable *headers;
		HashPosition pos;

		convert_to_array(*zdata);
		headers = HASH_OF(*zdata);
		zend_hash_internal_pointer_reset_ex(headers, &pos);

		props._flags += AMQP_BASIC_HEADERS_FLAG;
		props.headers.entries = emalloc(sizeof(struct amqp_table_entry_t_) * zend_hash_num_elements(headers));
		props.headers.num_entries = 0;

		while (zend_hash_get_current_data_ex(headers, (void **)&zdata, &pos) == SUCCESS) {
			char *string_key;
			uint string_key_len;
			int	type;
			ulong  num_key;

			type = zend_hash_get_current_key_ex(headers, &string_key, &string_key_len, &num_key, 0, &pos);

			props.headers.entries[props.headers.num_entries].key.bytes = string_key;
			props.headers.entries[props.headers.num_entries].key.len = string_key_len - 1;

			if (Z_TYPE_P(*zdata) == IS_STRING) {
				convert_to_string(*zdata);
				props.headers.entries[props.headers.num_entries].value.kind = AMQP_FIELD_KIND_UTF8;
				props.headers.entries[props.headers.num_entries].value.value.bytes.bytes = Z_STRVAL_P(*zdata);
				props.headers.entries[props.headers.num_entries].value.value.bytes.len = Z_STRLEN_P(*zdata);
				props.headers.num_entries++;
			} else if (Z_TYPE_P(*zdata) == IS_LONG) {
				convert_to_long(*zdata);
				props.headers.entries[props.headers.num_entries].value.kind = AMQP_FIELD_KIND_I32;
				props.headers.entries[props.headers.num_entries].value.value.i32 = Z_LVAL_P(*zdata);
				props.headers.num_entries++;
			} else if (Z_TYPE_P(*zdata) == IS_DOUBLE) {
				convert_to_double(*zdata);
				props.headers.entries[props.headers.num_entries].value.kind = AMQP_FIELD_KIND_F32;
				props.headers.entries[props.headers.num_entries].value.value.f32 = (double)Z_DVAL_P(*zdata);
				props.headers.num_entries++;
			} else if (Z_TYPE_PP(zdata) == IS_ARRAY) {
				zval **arr_data;
				amqp_array_t array;
				HashPosition arr_pos;
				array.entries = emalloc(sizeof(struct amqp_field_value_t_) * zend_hash_num_elements(Z_ARRVAL_PP(zdata)));
				array.num_entries = 0;
				for(
					zend_hash_internal_pointer_reset_ex(Z_ARRVAL_PP(zdata), &arr_pos);
					zend_hash_get_current_data_ex(Z_ARRVAL_PP(zdata), (void**) &arr_data, &arr_pos) == SUCCESS;
					zend_hash_move_forward_ex(Z_ARRVAL_PP(zdata), &arr_pos)
				) {
					if (Z_TYPE_PP(arr_data) == IS_STRING) {
						array.entries[array.num_entries].kind = AMQP_FIELD_KIND_UTF8;
						array.entries[array.num_entries].value.bytes.bytes = Z_STRVAL_PP(arr_data);
						array.entries[array.num_entries].value.bytes.len = Z_STRLEN_PP(arr_data);
						array.num_entries ++;
					} else {
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Ignoring non-string array member type %d for field '%s'", Z_TYPE_PP(arr_data), string_key);
					}
				}

				props.headers.entries[props.headers.num_entries].value.kind = AMQP_FIELD_KIND_ARRAY;
				props.headers.entries[props.headers.num_entries].value.value.array = array;
				props.headers.num_entries++;
			}

			zend_hash_move_forward_ex(headers, &pos);
		}
	} else {
		props.headers.entries = 0;
	}

	channel = AMQP_GET_CHANNEL(exchange);
	AMQP_VERIFY_CHANNEL(channel, "Could not publish to exchange exchange.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not publish to exchange exchange.");

#ifndef PHP_WIN32
	/* Start ignoring SIGPIPE */
	old_handler = signal(SIGPIPE, SIG_IGN);
#endif

	abt0.len = exchange->name_len;
	abt0.bytes = exchange->name;
	abt1.len = key_len;
	abt1.bytes = key_name;
	abt2.len = msg_len;
	abt2.bytes = msg;

	r = amqp_basic_publish(
		connection->connection_resource->connection_state,
		channel->channel_id,
		abt0,
		abt1,
		(AMQP_MANDATORY & flags) ? 1 : 0, /* mandatory */
		(AMQP_IMMEDIATE & flags) ? 1 : 0, /* immediate */
		&props,
		abt2
	);

	if (props.headers.entries) {
		int i;
		for (i=0; i < props.headers.num_entries; ++i) {
			free_field_value(props.headers.entries[i].value);
		}
		efree(props.headers.entries);
	}

#ifndef PHP_WIN32
	/* End ignoring of SIGPIPEs */
	signal(SIGPIPE, old_handler);
#endif

	/* handle any errors that occured outside of signals */
	if (r < 0) {
		char str[256];
		char ** pstr = (char **) &str;
        spprintf(pstr, 0, "Socket error: %s", amqp_error_string(-r));
        zend_throw_exception(amqp_exchange_exception_class_entry, *pstr, 0 TSRMLS_CC);
		return;
	}

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto int exchange::bind(string srcExchangeName, string routingKey[, int flags]);
bind exchange to exchange by routing key
*/
PHP_METHOD(amqp_exchange_class, bind)
{
	zval *id;
	amqp_exchange_object *exchange;
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	char *src_name;
	int src_name_len;
	char *keyname;
	int keyname_len;
	int flags;

	amqp_rpc_reply_t res;
	amqp_exchange_bind_t s;
	amqp_method_number_t method_ok = AMQP_EXCHANGE_BIND_OK_METHOD;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Oss|l", &id, amqp_exchange_class_entry, &src_name, &src_name_len, &keyname, &keyname_len, &flags) == FAILURE) {
		return;
	}

	exchange = (amqp_exchange_object *)zend_object_store_get_object(id TSRMLS_CC);

	channel = AMQP_GET_CHANNEL(exchange);
	AMQP_VERIFY_CHANNEL(channel, "Could not bind to exchange.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not bind to exchanges.");

	if (!keyname_len) {
		zend_throw_exception(amqp_exchange_exception_class_entry, "Could not bind exchange. No routing key given.", 0 TSRMLS_CC);
		return;
	}

	s.ticket				= 0;
	s.destination.len		= exchange->name_len;
	s.destination.bytes		= exchange->name;
	s.source.len			= src_name_len;
	s.source.bytes			= src_name;
	s.routing_key.len		= keyname_len;
	s.routing_key.bytes		= keyname;
	s.nowait				= IS_NOWAIT(flags);
	s.arguments.num_entries = 0;
	s.arguments.entries		= NULL;

	res = amqp_simple_rpc(
		connection->connection_resource->connection_state,
		channel->channel_id,
		AMQP_EXCHANGE_BIND_METHOD,
		&method_ok,
		&s
	);

	if (res.reply_type != AMQP_RESPONSE_NORMAL) {
		char str[256];
		char ** pstr = (char **) &str;
		amqp_error(res, pstr);
		zend_throw_exception(amqp_exchange_exception_class_entry, *pstr, 0 TSRMLS_CC);
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
