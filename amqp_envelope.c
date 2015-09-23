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

zend_object_handlers amqp_envelope_object_handlers;

HashTable *amqp_envelope_object_get_debug_info(zval *object, int *is_temp TSRMLS_DC) {
	zval value;
	HashTable *debug_info;

	/* Get the envelope object from which to read */
	amqp_envelope_object *envelope = AMQP_ENVELOPE_OBJ_P(object);

	/* Let zend clean up for us: */
	*is_temp = 1;

	/* Keep the # 18 matching the number of entries in this table*/
	ALLOC_HASHTABLE(debug_info);
	ZEND_INIT_SYMTABLE_EX(debug_info, 18 + 1, 0);

	/* Start adding values */
	ZVAL_STR(&value, zend_string_copy(envelope->body));
	zend_hash_str_add(debug_info, "body", sizeof("body")-1, &value);

	ZVAL_STRINGL(&value, envelope->content_type, strlen(envelope->content_type));
	zend_hash_str_add(debug_info, "content_type", sizeof("content_type")-1, &value);

	ZVAL_STRINGL(&value, envelope->routing_key, strlen(envelope->routing_key));
	zend_hash_str_add(debug_info, "routing_key", sizeof("routing_key")-1, &value);

	ZVAL_LONG(&value, envelope->delivery_tag);
	zend_hash_str_add(debug_info, "delivery_tag", sizeof("delivery_tag")-1, &value);

	ZVAL_LONG(&value, envelope->delivery_mode);
	zend_hash_str_add(debug_info, "delivery_mode", sizeof("delivery_mode")-1, &value);

	ZVAL_STRINGL(&value, envelope->exchange_name, strlen(envelope->exchange_name));
	zend_hash_str_add(debug_info, "exchange_name", sizeof("exchange_name")-1, &value);

	ZVAL_LONG(&value, envelope->is_redelivery);
	zend_hash_str_add(debug_info, "is_redelivery", sizeof("is_redelivery")-1, &value);

	ZVAL_STRINGL(&value, envelope->content_encoding, strlen(envelope->content_encoding));
	zend_hash_str_add(debug_info, "content_encoding", sizeof("content_encoding")-1, &value);

	ZVAL_STRINGL(&value, envelope->type, strlen(envelope->type));
	zend_hash_str_add(debug_info, "type", sizeof("type")-1, &value);

	ZVAL_LONG(&value, envelope->timestamp);
	zend_hash_str_add(debug_info, "timestamp", sizeof("timestamp")-1, &value);

	ZVAL_LONG(&value, envelope->priority);
	zend_hash_str_add(debug_info, "priority", sizeof("priority")-1, &value);

	ZVAL_STRINGL(&value, envelope->expiration, strlen(envelope->expiration));
	zend_hash_str_add(debug_info, "expiration", sizeof("expiration")-1, &value);

	ZVAL_STRINGL(&value, envelope->user_id, strlen(envelope->user_id));
	zend_hash_str_add(debug_info, "user_id", sizeof("user_id")-1, &value);

	ZVAL_STRINGL(&value, envelope->app_id, strlen(envelope->app_id));
	zend_hash_str_add(debug_info, "app_id", sizeof("app_id")-1, &value);

	ZVAL_STRINGL(&value, envelope->message_id, strlen(envelope->message_id));
	zend_hash_str_add(debug_info, "message_id", sizeof("message_id")-1, &value);

	ZVAL_STRINGL(&value, envelope->reply_to, strlen(envelope->reply_to));
	zend_hash_str_add(debug_info, "reply_to", sizeof("reply_to")-1, &value);

	ZVAL_STRINGL(&value, envelope->correlation_id, strlen(envelope->correlation_id));
	zend_hash_str_add(debug_info, "correlation_id", sizeof("correlation_id")-1, &value);

	Z_ADDREF(envelope->headers);
	zend_hash_str_add(debug_info, "headers", sizeof("headers")-1, &envelope->headers);

	return debug_info;
}

void amqp_envelope_free_obj(zend_object *object TSRMLS_DC)
{
	amqp_envelope_object *envelope = amqp_envelope_object_fetch_object(object);

	if (envelope->body) {
		zend_string_release(envelope->body);
	}

	zend_object_std_dtor(&envelope->zo TSRMLS_CC);
}

void amqp_envelope_dtor_obj(zend_object *object TSRMLS_DC)
{
	amqp_envelope_object *envelope = amqp_envelope_object_fetch_object(object);

	zval_ptr_dtor(&envelope->headers);
}

zend_object* amqp_envelope_ctor(zend_class_entry *ce TSRMLS_DC)
{
	amqp_envelope_object *envelope = (amqp_envelope_object*)ecalloc(1,
			sizeof(amqp_envelope_object)
			+ zend_object_properties_size(ce));

	zend_object_std_init(&envelope->zo, ce TSRMLS_CC);
	object_properties_init(&envelope->zo, ce);

	array_init(&envelope->headers);

	memcpy((void *)&amqp_envelope_object_handlers, (void *)zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	amqp_envelope_object_handlers.get_debug_info = amqp_envelope_object_get_debug_info;
	amqp_envelope_object_handlers.free_obj = amqp_envelope_free_obj;
	amqp_envelope_object_handlers.dtor_obj = amqp_envelope_dtor_obj;
	amqp_envelope_object_handlers.offset = XtOffsetOf(amqp_envelope_object, zo);
	envelope->zo.handlers = &amqp_envelope_object_handlers;

	return &envelope->zo;
}

/* {{{ proto AMQPEnvelope::__construct()
 */
PHP_METHOD(amqp_envelope_class, __construct)
{
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
}
/* }}} */

/* {{{ proto AMQPEnvelope::getBody()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getBody)
{
	amqp_envelope_object *envelope = AMQP_ENVELOPE_OBJ_P(getThis());

	/* Try to pull amqp object out of method params */
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	if (ZSTR_LEN(envelope->body) == 0) {
		RETURN_FALSE;
	}

	RETURN_STR(zend_string_copy(envelope->body));
}
/* }}} */

/* {{{ proto AMQPEnvelope::getRoutingKey()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getRoutingKey)
{
	amqp_envelope_object *envelope = AMQP_ENVELOPE_OBJ_P(getThis());

	/* Try to pull amqp object out of method params */
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_STRING(envelope->routing_key);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getDeliveryMode()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getDeliveryMode)
{
	amqp_envelope_object *envelope = AMQP_ENVELOPE_OBJ_P(getThis());

	/* Try to pull amqp object out of method params */
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_LONG(envelope->delivery_mode);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getDeliveryTag()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getDeliveryTag)
{
	amqp_envelope_object *envelope = AMQP_ENVELOPE_OBJ_P(getThis());

	/* Try to pull amqp object out of method params */
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_LONG(envelope->delivery_tag);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getExchangeName()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getExchangeName)
{
	amqp_envelope_object *envelope = AMQP_ENVELOPE_OBJ_P(getThis());

	/* Try to pull amqp object out of method params */
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_STRING(envelope->exchange_name);
}
/* }}} */

/* {{{ proto AMQPEnvelope::isRedelivery()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, isRedelivery)
{
	amqp_envelope_object *envelope = AMQP_ENVELOPE_OBJ_P(getThis());

	/* Try to pull amqp object out of method params */
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	/* We have no envelope */
	RETURN_BOOL(envelope->is_redelivery);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getContentType()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getContentType)
{
	amqp_envelope_object *envelope = AMQP_ENVELOPE_OBJ_P(getThis());

	/* Try to pull amqp object out of method params */
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_STRING(envelope->content_type);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getContentEncoding()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getContentEncoding)
{
	amqp_envelope_object *envelope = AMQP_ENVELOPE_OBJ_P(getThis());

	/* Try to pull amqp object out of method params */
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_STRING(envelope->content_encoding);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getType()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getType)
{
	amqp_envelope_object *envelope = AMQP_ENVELOPE_OBJ_P(getThis());

	/* Try to pull amqp object out of method params */
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_STRING(envelope->type);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getTimestamp()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getTimestamp)
{
	amqp_envelope_object *envelope = AMQP_ENVELOPE_OBJ_P(getThis());

	/* Try to pull amqp object out of method params */
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_LONG(envelope->timestamp);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getPriority()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getPriority)
{
	amqp_envelope_object *envelope = AMQP_ENVELOPE_OBJ_P(getThis());

	/* Try to pull amqp object out of method params */
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_LONG(envelope->priority);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getExpiration()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getExpiration)
{
	amqp_envelope_object *envelope = AMQP_ENVELOPE_OBJ_P(getThis());

	/* Try to pull amqp object out of method params */
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_STRING(envelope->expiration);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getUserId()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getUserId)
{
	amqp_envelope_object *envelope = AMQP_ENVELOPE_OBJ_P(getThis());

	/* Try to pull amqp object out of method params */
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_STRING(envelope->user_id);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getAppId()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getAppId)
{
	amqp_envelope_object *envelope = AMQP_ENVELOPE_OBJ_P(getThis());

	/* Try to pull amqp object out of method params */
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_STRING(envelope->app_id);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getMessageId()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getMessageId)
{
	amqp_envelope_object *envelope = AMQP_ENVELOPE_OBJ_P(getThis());

	/* Try to pull amqp object out of method params */
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_STRING(envelope->message_id);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getReplyTo()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getReplyTo)
{
	amqp_envelope_object *envelope = AMQP_ENVELOPE_OBJ_P(getThis());

	/* Try to pull amqp object out of method params */
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_STRING(envelope->reply_to);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getCorrelationId()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getCorrelationId)
{
	amqp_envelope_object *envelope = AMQP_ENVELOPE_OBJ_P(getThis());

	/* Try to pull amqp object out of method params */
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_STRING(envelope->correlation_id);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getHeader(string headerName)
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getHeader)
{
	amqp_envelope_object *envelope = AMQP_ENVELOPE_OBJ_P(getThis());
	zval *tmp = 0;
	zend_string* key;

	/* Try to pull amqp object out of method params */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "S", &key) == FAILURE) {
		return;
	}

	/* Look for the hash key */
	if ((tmp = zend_hash_find(HASH_OF(&envelope->headers), key)) == NULL) {
		RETURN_FALSE;
	}

	RETURN_ZVAL(tmp, 1, 0);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getHeaders()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getHeaders)
{
	amqp_envelope_object *envelope = AMQP_ENVELOPE_OBJ_P(getThis());

	/* Try to pull amqp object out of method params */
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_ZVAL(&envelope->headers, 1, 0);
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
