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
#include "amqp_connection.h"
#include "amqp_channel.h"
#include "amqp_exchange.h"
#include "amqp_type.h"

zend_class_entry *amqp_exchange_class_entry;
#define this_ce amqp_exchange_class_entry

/* {{{ proto AMQPExchange::__construct(AMQPChannel channel);
create Exchange   */
static PHP_METHOD(amqp_exchange_class, __construct)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;

	PHP5to7_zval_t arguments PHP5to7_MAYBE_SET_TO_NULL;

	zval *channelObj;
	amqp_channel_resource *channel_resource;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &channelObj, amqp_channel_class_entry) == FAILURE) {
		return;
	}

	PHP5to7_MAYBE_INIT(arguments);
	PHP5to7_ARRAY_INIT(arguments);
	zend_update_property(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("arguments"), PHP5to7_MAYBE_PTR(arguments) TSRMLS_CC);
	PHP5to7_MAYBE_DESTROY(arguments);

	channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(channelObj);
	PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not create exchange.");

	zend_update_property(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("channel"), channelObj TSRMLS_CC);
	zend_update_property(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("connection"), PHP_AMQP_READ_OBJ_PROP(amqp_channel_class_entry, channelObj, "connection") TSRMLS_CC);
}
/* }}} */


/* {{{ proto AMQPExchange::getName()
Get the exchange name */
static PHP_METHOD(amqp_exchange_class, getName)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;

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
static PHP_METHOD(amqp_exchange_class, setName)
{
	char *name = NULL;
	PHP5to7_param_str_len_type_t name_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len) == FAILURE) {
		return;
	}

	/* Verify that the name is not null and not an empty string */
	if (name_len > 255) {
		zend_throw_exception(amqp_exchange_exception_class_entry, "Invalid exchange name given, must be less than 255 characters long.", 0 TSRMLS_CC);
		return;
	}

	/* Set the exchange name */
	zend_update_property_stringl(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("name"), name, name_len TSRMLS_CC);
}
/* }}} */


/* {{{ proto AMQPExchange::getFlags()
Get the exchange parameters */
static PHP_METHOD(amqp_exchange_class, getFlags)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;

	PHP5to7_param_long_type_t flagBitmask = 0;

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
static PHP_METHOD(amqp_exchange_class, setFlags)
{
	PHP5to7_param_long_type_t flagBitmask;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &flagBitmask) == FAILURE) {
		return;
	}

	/* Set the flags based on the bitmask we were given */
	flagBitmask = flagBitmask ? flagBitmask & PHP_AMQP_EXCHANGE_FLAGS : flagBitmask;

	zend_update_property_bool(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("passive"), IS_PASSIVE(flagBitmask) TSRMLS_CC);
	zend_update_property_bool(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("durable"), IS_DURABLE(flagBitmask) TSRMLS_CC);
	zend_update_property_bool(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("auto_delete"), IS_AUTODELETE(flagBitmask) TSRMLS_CC);
	zend_update_property_bool(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("internal"), IS_INTERNAL(flagBitmask) TSRMLS_CC);
}
/* }}} */


/* {{{ proto AMQPExchange::getType()
Get the exchange type */
static PHP_METHOD(amqp_exchange_class, getType)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;

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
static PHP_METHOD(amqp_exchange_class, setType)
{
	char *type = NULL;	PHP5to7_param_str_len_type_t type_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &type, &type_len) == FAILURE) {
		return;
	}

	zend_update_property_stringl(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("type"), type, type_len TSRMLS_CC);
}
/* }}} */


/* {{{ proto AMQPExchange::getArgument(string key)
Get the exchange argument referenced by key */
static PHP_METHOD(amqp_exchange_class, getArgument)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;

	PHP5to7_zval_t *tmp = NULL;

	char *key;	PHP5to7_param_str_len_type_t key_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &key_len) == FAILURE) {
		return;
	}

	if (!PHP5to7_ZEND_HASH_FIND(PHP_AMQP_READ_THIS_PROP_ARR("arguments"), key, key_len + 1, tmp)) {
		RETURN_FALSE;
	}

	RETURN_ZVAL(PHP5to7_MAYBE_DEREF(tmp), 1, 0);
}
/* }}} */

/* {{{ proto AMQPExchange::hasArgument(string key) */
static PHP_METHOD(amqp_exchange_class, hasArgument)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;

	PHP5to7_zval_t *tmp = NULL;

	char *key;	PHP5to7_param_str_len_type_t key_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &key_len) == FAILURE) {
		return;
	}

	if (!PHP5to7_ZEND_HASH_FIND(PHP_AMQP_READ_THIS_PROP_ARR("arguments"), key, (unsigned)(key_len + 1), tmp)) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto AMQPExchange::getArguments
Get the exchange arguments */
static PHP_METHOD(amqp_exchange_class, getArguments)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;
	PHP_AMQP_NOPARAMS();
	PHP_AMQP_RETURN_THIS_PROP("arguments");
}
/* }}} */


/* {{{ proto AMQPExchange::setArguments(array args)
Overwrite all exchange arguments with given args */
static PHP_METHOD(amqp_exchange_class, setArguments)
{
	zval *zvalArguments;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a/", &zvalArguments) == FAILURE) {
		return;
	}

	zend_update_property(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("arguments"), zvalArguments TSRMLS_CC);

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto AMQPExchange::setArgument(key, value) */
static PHP_METHOD(amqp_exchange_class, setArgument)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;

	char *key= NULL;    PHP5to7_param_str_len_type_t key_len = 0;
	zval *value = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz",
							  &key, &key_len,
							  &value) == FAILURE) {
		return;
	}

	switch (Z_TYPE_P(value)) {
		case IS_NULL:
			PHP5to7_ZEND_HASH_DEL(PHP_AMQP_READ_THIS_PROP_ARR("arguments"), key, (unsigned) (key_len + 1));
			break;
		PHP5to7_CASE_IS_BOOL:
		case IS_LONG:
		case IS_DOUBLE:
		case IS_STRING:
			PHP5to7_ZEND_HASH_ADD(PHP_AMQP_READ_THIS_PROP_ARR("arguments"), key, (unsigned) (key_len + 1), value, sizeof(zval *));
			Z_TRY_ADDREF_P(value);
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
static PHP_METHOD(amqp_exchange_class, declareExchange)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;

	amqp_channel_resource *channel_resource;
	amqp_table_t *arguments;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(PHP_AMQP_READ_THIS_PROP("channel"));
	PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not declare exchange.");

	/* Check that the exchange has a name */
	if (PHP_AMQP_READ_THIS_PROP_STRLEN("name") < 1) {
		zend_throw_exception(amqp_exchange_exception_class_entry, "Could not declare exchange. Exchanges must have a name.", 0 TSRMLS_CC);
		return;
	}

	/* Check that the exchange has a name */
	if (PHP_AMQP_READ_THIS_PROP_STRLEN("type") < 1) {
		zend_throw_exception(amqp_exchange_exception_class_entry, "Could not declare exchange. Exchanges must have a type.", 0 TSRMLS_CC);
		return;
	}

	arguments = php_amqp_type_convert_zval_to_amqp_table(PHP_AMQP_READ_THIS_PROP("arguments") TSRMLS_CC);

	amqp_exchange_declare(
		channel_resource->connection_resource->connection_state,
		channel_resource->channel_id,
		amqp_cstring_bytes(PHP_AMQP_READ_THIS_PROP_STR("name")),
		amqp_cstring_bytes(PHP_AMQP_READ_THIS_PROP_STR("type")),
		PHP_AMQP_READ_THIS_PROP_BOOL("passive"),
		PHP_AMQP_READ_THIS_PROP_BOOL("durable"),
		PHP_AMQP_READ_THIS_PROP_BOOL("auto_delete"),
		PHP_AMQP_READ_THIS_PROP_BOOL("internal"),
		*arguments
	);

	amqp_rpc_reply_t res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

	php_amqp_type_free_amqp_table(arguments);
	php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

	if (PHP_AMQP_MAYBE_ERROR(res, channel_resource)) {
		php_amqp_zend_throw_exception_short(res, amqp_exchange_exception_class_entry TSRMLS_CC);
		return;
	}

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto AMQPExchange::delete([string name[, long params]]);
delete Exchange
*/
static PHP_METHOD(amqp_exchange_class, delete)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;

	amqp_channel_resource *channel_resource;

	char *name = NULL;  PHP5to7_param_str_len_type_t name_len = 0;
	PHP5to7_param_long_type_t flags = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|sl",
							  &name, &name_len,
							  &flags) == FAILURE) {
		return;
	}

	channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(PHP_AMQP_READ_THIS_PROP("channel"));
	PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not delete exchange.");

 	amqp_exchange_delete(
		channel_resource->connection_resource->connection_state,
		channel_resource->channel_id,
		amqp_cstring_bytes(name_len ? name : PHP_AMQP_READ_THIS_PROP_STR("name")),
		(AMQP_IFUNUSED & flags) ? 1 : 0
	);

	amqp_rpc_reply_t res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

	if (PHP_AMQP_MAYBE_ERROR(res, channel_resource)) {
		php_amqp_zend_throw_exception_short(res, amqp_exchange_exception_class_entry TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
		return;
	}

	php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto AMQPExchange::publish(string msg, [string key, [int flags, [array headers]]]);
publish into Exchange
*/
static PHP_METHOD(amqp_exchange_class, publish)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;

	zval *ini_arr = NULL;
	PHP5to7_zval_t *tmp = NULL;

	amqp_channel_resource *channel_resource;

	char *key_name = NULL;  PHP5to7_param_str_len_type_t key_len = 0;
	char *msg 	   = NULL;  PHP5to7_param_str_len_type_t msg_len = 0;
	PHP5to7_param_long_type_t flags = AMQP_NOPARAM;

#ifndef PHP_WIN32
	/* Storage for previous signal handler during SIGPIPE override */
	void * old_handler;
#endif

	amqp_basic_properties_t props;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s!la/",
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
	if (ini_arr && PHP5to7_ZEND_HASH_FIND(HASH_OF (ini_arr), "content_type", sizeof("content_type"), tmp)) {
		SEPARATE_ZVAL(tmp);
		convert_to_string(PHP5to7_MAYBE_DEREF(tmp));

		if (Z_STRLEN_P(PHP5to7_MAYBE_DEREF(tmp)) > 0) {
			props.content_type = amqp_cstring_bytes(Z_STRVAL_P(PHP5to7_MAYBE_DEREF(tmp)));
			props._flags |= AMQP_BASIC_CONTENT_TYPE_FLAG;
		}
	}

	if (ini_arr && PHP5to7_ZEND_HASH_FIND(HASH_OF (ini_arr), "content_encoding", sizeof("content_encoding"), tmp)) {
		SEPARATE_ZVAL(tmp);
		convert_to_string(PHP5to7_MAYBE_DEREF(tmp));

		if (Z_STRLEN_P(PHP5to7_MAYBE_DEREF(tmp)) > 0) {
			props.content_encoding = amqp_cstring_bytes(Z_STRVAL_P(PHP5to7_MAYBE_DEREF(tmp)));
			props._flags |= AMQP_BASIC_CONTENT_ENCODING_FLAG;
		}
	}

	if (ini_arr && PHP5to7_ZEND_HASH_FIND(HASH_OF (ini_arr), "message_id", sizeof("message_id"), tmp)) {
		SEPARATE_ZVAL(tmp);
		convert_to_string(PHP5to7_MAYBE_DEREF(tmp));

		if (Z_STRLEN_P(PHP5to7_MAYBE_DEREF(tmp)) > 0) {
			props.message_id = amqp_cstring_bytes(Z_STRVAL_P(PHP5to7_MAYBE_DEREF(tmp)));
			props._flags |= AMQP_BASIC_MESSAGE_ID_FLAG;
		}
	}

	if (ini_arr && PHP5to7_ZEND_HASH_FIND(HASH_OF (ini_arr), "user_id", sizeof("user_id"), tmp)) {
		SEPARATE_ZVAL(tmp);
		convert_to_string(PHP5to7_MAYBE_DEREF(tmp));

		if (Z_STRLEN_P(PHP5to7_MAYBE_DEREF(tmp)) > 0) {
			props.user_id = amqp_cstring_bytes(Z_STRVAL_P(PHP5to7_MAYBE_DEREF(tmp)));
			props._flags |= AMQP_BASIC_USER_ID_FLAG;
		}
	}

	if (ini_arr && PHP5to7_ZEND_HASH_FIND(HASH_OF (ini_arr), "app_id", sizeof("app_id"), tmp)) {
		SEPARATE_ZVAL(tmp);
		convert_to_string(PHP5to7_MAYBE_DEREF(tmp));

		if (Z_STRLEN_P(PHP5to7_MAYBE_DEREF(tmp)) > 0) {
			props.app_id = amqp_cstring_bytes(Z_STRVAL_P(PHP5to7_MAYBE_DEREF(tmp)));
			props._flags |= AMQP_BASIC_APP_ID_FLAG;
		}
	}

	if (ini_arr && PHP5to7_ZEND_HASH_FIND(HASH_OF (ini_arr), "delivery_mode", sizeof("delivery_mode"), tmp)) {
		SEPARATE_ZVAL(tmp);
		convert_to_long(PHP5to7_MAYBE_DEREF(tmp));

		props.delivery_mode = (uint8_t)Z_LVAL_P(PHP5to7_MAYBE_DEREF(tmp));
		props._flags |= AMQP_BASIC_DELIVERY_MODE_FLAG;
	}

	if (ini_arr && PHP5to7_ZEND_HASH_FIND(HASH_OF (ini_arr), "priority", sizeof("priority"), tmp)) {
		SEPARATE_ZVAL(tmp);
		convert_to_long(PHP5to7_MAYBE_DEREF(tmp));

		props.priority = (uint8_t)Z_LVAL_P(PHP5to7_MAYBE_DEREF(tmp));
		props._flags |= AMQP_BASIC_PRIORITY_FLAG;
	}

	if (ini_arr && PHP5to7_ZEND_HASH_FIND(HASH_OF (ini_arr), "timestamp", sizeof("timestamp"), tmp)) {
		SEPARATE_ZVAL(tmp);
		convert_to_long(PHP5to7_MAYBE_DEREF(tmp));

		props.timestamp = (uint64_t)Z_LVAL_P(PHP5to7_MAYBE_DEREF(tmp));
		props._flags |= AMQP_BASIC_TIMESTAMP_FLAG;
	}

	if (ini_arr && PHP5to7_ZEND_HASH_FIND(HASH_OF (ini_arr), "expiration", sizeof("expiration"), tmp)) {
		SEPARATE_ZVAL(tmp);
		convert_to_string(PHP5to7_MAYBE_DEREF(tmp));

		if (Z_STRLEN_P(PHP5to7_MAYBE_DEREF(tmp)) > 0) {
			props.expiration = amqp_cstring_bytes(Z_STRVAL_P(PHP5to7_MAYBE_DEREF(tmp)));
			props._flags |= AMQP_BASIC_EXPIRATION_FLAG;
		}
	}

	if (ini_arr && PHP5to7_ZEND_HASH_FIND(HASH_OF (ini_arr), "type", sizeof("type"), tmp)) {
		SEPARATE_ZVAL(tmp);
		convert_to_string(PHP5to7_MAYBE_DEREF(tmp));

		if (Z_STRLEN_P(PHP5to7_MAYBE_DEREF(tmp)) > 0) {
			props.type = amqp_cstring_bytes(Z_STRVAL_P(PHP5to7_MAYBE_DEREF(tmp)));
			props._flags |= AMQP_BASIC_TYPE_FLAG;
		}
	}

	if (ini_arr && PHP5to7_ZEND_HASH_FIND(HASH_OF (ini_arr), "reply_to", sizeof("reply_to"), tmp)) {
		SEPARATE_ZVAL(tmp);
		convert_to_string(PHP5to7_MAYBE_DEREF(tmp));

		if (Z_STRLEN_P(PHP5to7_MAYBE_DEREF(tmp)) > 0) {
			props.reply_to = amqp_cstring_bytes(Z_STRVAL_P(PHP5to7_MAYBE_DEREF(tmp)));
			props._flags |= AMQP_BASIC_REPLY_TO_FLAG;
		}
	}
	if (ini_arr && PHP5to7_ZEND_HASH_FIND(HASH_OF (ini_arr), "correlation_id", sizeof("correlation_id"), tmp)) {
		SEPARATE_ZVAL(tmp);
		convert_to_string(PHP5to7_MAYBE_DEREF(tmp));

		if (Z_STRLEN_P(PHP5to7_MAYBE_DEREF(tmp)) > 0) {
			props.correlation_id = amqp_cstring_bytes(Z_STRVAL_P(PHP5to7_MAYBE_DEREF(tmp)));
			props._flags |= AMQP_BASIC_CORRELATION_ID_FLAG;
		}
	}

	}

	amqp_table_t *headers = NULL;

	if (ini_arr && PHP5to7_ZEND_HASH_FIND(HASH_OF(ini_arr), "headers", sizeof("headers"), tmp)) {
		SEPARATE_ZVAL(tmp);
		convert_to_array(PHP5to7_MAYBE_DEREF(tmp));

		headers = php_amqp_type_convert_zval_to_amqp_table(PHP5to7_MAYBE_DEREF(tmp) TSRMLS_CC);

		props._flags |= AMQP_BASIC_HEADERS_FLAG;
		props.headers = *headers;
	}

	channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(PHP_AMQP_READ_THIS_PROP("channel"));
	PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not publish to exchange.");

#ifndef PHP_WIN32
	/* Start ignoring SIGPIPE */
	old_handler = signal(SIGPIPE, SIG_IGN);
#endif

	zval *exchange_name = PHP_AMQP_READ_THIS_PROP("name");

	/* NOTE: basic.publish is asynchronous and thus will not indicate failure if something goes wrong on the broker */
	int status = amqp_basic_publish(
		channel_resource->connection_resource->connection_state,
		channel_resource->channel_id,
		(Z_TYPE_P(exchange_name) == IS_STRING && Z_STRLEN_P(exchange_name) > 0 ? amqp_cstring_bytes(Z_STRVAL_P(exchange_name)) : amqp_empty_bytes),	/* exchange */
		(key_len > 0 ? amqp_cstring_bytes(key_name) : amqp_empty_bytes), /* routing key */
		(AMQP_MANDATORY & flags) ? 1 : 0, /* mandatory */
		(AMQP_IMMEDIATE & flags) ? 1 : 0, /* immediate */
		&props,
		php_amqp_type_char_to_amqp_long(msg, msg_len) /* message body */
	);

	if (headers) {
		php_amqp_type_free_amqp_table(headers);
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

		php_amqp_error(res, &PHP_AMQP_G(error_message), channel_resource->connection_resource, channel_resource TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_exchange_exception_class_entry, PHP_AMQP_G(error_message), PHP_AMQP_G(error_code) TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
		return;
	}

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto int exchange::bind(string srcExchangeName[, string routingKey, array arguments]);
bind exchange to exchange by routing key
*/
static PHP_METHOD(amqp_exchange_class, bind)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;

	zval *zvalArguments = NULL;

	amqp_channel_resource *channel_resource;

	char *src_name;		PHP5to7_param_str_len_type_t src_name_len = 0;
	char *keyname;		PHP5to7_param_str_len_type_t keyname_len = 0;

	amqp_table_t *arguments = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|sa",
							  &src_name, &src_name_len,
							  &keyname, &keyname_len,
							  &zvalArguments) == FAILURE) {
		return;
	}

	channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(PHP_AMQP_READ_THIS_PROP("channel"));
	PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not bind to exchange.");

	if (zvalArguments) {
		arguments = php_amqp_type_convert_zval_to_amqp_table(zvalArguments TSRMLS_CC);
	}

	amqp_exchange_bind(
		channel_resource->connection_resource->connection_state,
		channel_resource->channel_id,
		amqp_cstring_bytes(PHP_AMQP_READ_THIS_PROP_STR("name")),
		(src_name_len > 0 ? amqp_cstring_bytes(src_name) : amqp_empty_bytes),
		(keyname_len  > 0 ? amqp_cstring_bytes(keyname)  : amqp_empty_bytes),
		(arguments ? *arguments : amqp_empty_table)
	);

	if (arguments) {
		php_amqp_type_free_amqp_table(arguments);
	}

	amqp_rpc_reply_t res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

	if (PHP_AMQP_MAYBE_ERROR(res, channel_resource)) {
		php_amqp_zend_throw_exception_short(res, amqp_exchange_exception_class_entry TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
		return;
	}

	php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int exchange::unbind(string srcExchangeName[, string routingKey, array arguments]);
remove exchange to exchange binding by routing key
*/
static PHP_METHOD(amqp_exchange_class, unbind)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;

	zval *zvalArguments = NULL;

	amqp_channel_resource *channel_resource;

	char *src_name; 	PHP5to7_param_str_len_type_t src_name_len = 0;
	char *keyname;		PHP5to7_param_str_len_type_t keyname_len = 0;

	amqp_table_t *arguments = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|sa",
							  &src_name, &src_name_len,
							  &keyname, &keyname_len,
							  &zvalArguments) == FAILURE) {
		return;
	}

	channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(PHP_AMQP_READ_THIS_PROP("channel"));
	PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not unbind from exchange.");

	if (zvalArguments) {
		arguments = php_amqp_type_convert_zval_to_amqp_table(zvalArguments TSRMLS_CC);
	}

	amqp_exchange_unbind(
		channel_resource->connection_resource->connection_state,
		channel_resource->channel_id,
		amqp_cstring_bytes(PHP_AMQP_READ_THIS_PROP_STR("name")),
		(src_name_len > 0 ? amqp_cstring_bytes(src_name) : amqp_empty_bytes),
		(keyname_len  > 0 ? amqp_cstring_bytes(keyname)  : amqp_empty_bytes),
		(arguments ? *arguments : amqp_empty_table)
	);

	if (arguments) {
		php_amqp_type_free_amqp_table(arguments);
	}

	amqp_rpc_reply_t res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

	if (PHP_AMQP_MAYBE_ERROR(res, channel_resource)) {
		php_amqp_zend_throw_exception_short(res, amqp_exchange_exception_class_entry TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
		return;
	}

	php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto AMQPExchange::getChannel()
Get the AMQPChannel object in use */
static PHP_METHOD(amqp_exchange_class, getChannel)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;
	PHP_AMQP_NOPARAMS();
	PHP_AMQP_RETURN_THIS_PROP("channel");
}
/* }}} */

/* {{{ proto AMQPExchange::getConnection()
Get the AMQPConnection object in use */
static PHP_METHOD(amqp_exchange_class, getConnection)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;
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

		{NULL, NULL, NULL}
};

PHP_MINIT_FUNCTION(amqp_exchange)
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "AMQPExchange", amqp_exchange_class_functions);
	this_ce = zend_register_internal_class(&ce TSRMLS_CC);

	zend_declare_property_null(this_ce, ZEND_STRL("connection"), ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(this_ce, ZEND_STRL("channel"), ZEND_ACC_PRIVATE TSRMLS_CC);

	zend_declare_property_stringl(this_ce, ZEND_STRL("name"), "", 0, ZEND_ACC_PRIVATE TSRMLS_CC);
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
