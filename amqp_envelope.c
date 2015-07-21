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
	amqp_envelope_object *envelope = (amqp_envelope_object *)Z_OBJ_P(object TSRMLS_CC);

	/* Let zend clean up for us: */
	*is_temp = 1;

	/* Keep the # 18 matching the number of entries in this table*/
	ALLOC_HASHTABLE(debug_info);
	ZEND_INIT_SYMTABLE_EX(debug_info, 18 + 1, 0);

	/* Start adding values */
	ZVAL_STRINGL(&value, envelope->body, envelope->body_len);
	zend_hash_add(debug_info, "body", &value);

	ZVAL_STRINGL(&value, envelope->content_type, strlen(envelope->content_type));
	zend_hash_add(debug_info, "content_type", &value);

	ZVAL_STRINGL(&value, envelope->routing_key, strlen(envelope->routing_key));
	zend_hash_add(debug_info, "routing_key", &value);

	ZVAL_LONG(&value, envelope->delivery_tag);
	zend_hash_add(debug_info, "delivery_tag", &value);

	ZVAL_LONG(&value, envelope->delivery_mode);
	zend_hash_add(debug_info, "delivery_mode", &value);

	ZVAL_STRINGL(&value, envelope->exchange_name, strlen(envelope->exchange_name));
	zend_hash_add(debug_info, "exchange_name", &value);

	ZVAL_LONG(&value, envelope->is_redelivery);
	zend_hash_add(debug_info, "is_redelivery", &value);

	ZVAL_STRINGL(&value, envelope->content_encoding, strlen(envelope->content_encoding));
	zend_hash_add(debug_info, "content_encoding", &value);

	ZVAL_STRINGL(&value, envelope->type, strlen(envelope->type));
	zend_hash_add(debug_info, "type", &value);

	ZVAL_LONG(&value, envelope->timestamp);
	zend_hash_add(debug_info, "timestamp", &value);

	ZVAL_LONG(&value, envelope->priority);
	zend_hash_add(debug_info, "priority", &value);

	ZVAL_STRINGL(&value, envelope->expiration, strlen(envelope->expiration));
	zend_hash_add(debug_info, "expiration", &value);

	ZVAL_STRINGL(&value, envelope->user_id, strlen(envelope->user_id));
	zend_hash_add(debug_info, "user_id", &value);

	ZVAL_STRINGL(&value, envelope->app_id, strlen(envelope->app_id));
	zend_hash_add(debug_info, "app_id", &value);

	ZVAL_STRINGL(&value, envelope->message_id, strlen(envelope->message_id));
	zend_hash_add(debug_info, "message_id", &value);

	ZVAL_STRINGL(&value, envelope->reply_to, strlen(envelope->reply_to));
	zend_hash_add(debug_info, "reply_to", &value);

	ZVAL_STRINGL(&value, envelope->correlation_id, strlen(envelope->correlation_id));
	zend_hash_add(debug_info, "correlation_id", &value);

	Z_ADDREF_P(envelope->headers);
	zend_hash_add(debug_info, "headers", envelope->headers);

	return debug_info;
}

void amqp_envelope_dtor(void *object TSRMLS_DC)
{
	amqp_envelope_object *envelope = (amqp_envelope_object*)object;

	if (envelope->headers) {
		zval_ptr_dtor(&envelope->headers);
	}

	if (envelope->body) {
		efree(envelope->body);
	}

	zend_object_std_dtor(&envelope->zo TSRMLS_CC);

	efree(object);
}

zend_object amqp_envelope_ctor(zend_class_entry *ce TSRMLS_DC)
{
	zend_object new_value;
	amqp_envelope_object *envelope = (amqp_envelope_object*)emalloc(sizeof(amqp_envelope_object));

	memset(envelope, 0, sizeof(amqp_envelope_object));

	MAKE_STD_ZVAL(envelope->headers);
	array_init(envelope->headers);

	zend_object_std_init(&envelope->zo, ce TSRMLS_CC);
	AMQP_OBJECT_PROPERTIES_INIT(envelope->zo, ce);

	new_value.handle = zend_objects_store_put(envelope, (zend_objects_store_dtor_t)zend_objects_destroy_object, (zend_objects_free_object_storage_t)amqp_envelope_dtor, NULL TSRMLS_CC);

	memcpy((void *)&amqp_envelope_object_handlers, (void *)zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	amqp_envelope_object_handlers.get_debug_info = amqp_envelope_object_get_debug_info;
	new_value.handlers = &amqp_envelope_object_handlers;

	return new_value;
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
	amqp_envelope_object *envelope;

	/* Try to pull amqp object out of method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &envelope, amqp_envelope_class_entry) == FAILURE) {
		return;
	}

	if (envelope->body == 0) {
		RETURN_FALSE;
	}

	RETURN_STR(envelope->body);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getRoutingKey()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getRoutingKey)
{
	amqp_envelope_object *envelope;

	/* Try to pull amqp object out of method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &envelope, amqp_envelope_class_entry) == FAILURE) {
		return;
	}

	RETURN_STR(envelope->routing_key);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getDeliveryMode()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getDeliveryMode)
{
	amqp_envelope_object *envelope;

	/* Try to pull amqp object out of method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &envelope, amqp_envelope_class_entry) == FAILURE) {
		return;
	}

	RETURN_LONG(envelope->delivery_mode);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getDeliveryTag()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getDeliveryTag)
{
	amqp_envelope_object *envelope;

	/* Try to pull amqp object out of method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &envelope, amqp_envelope_class_entry) == FAILURE) {
		return;
	}

	RETURN_LONG(envelope->delivery_tag);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getExchangeName()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getExchangeName)
{
	amqp_envelope_object *envelope;

	/* Try to pull amqp object out of method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &envelope, amqp_envelope_class_entry) == FAILURE) {
		return;
	}

	RETURN_STR(envelope->exchange_name);
}
/* }}} */

/* {{{ proto AMQPEnvelope::isRedelivery()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, isRedelivery)
{
	amqp_envelope_object *envelope;

	/* Try to pull amqp object out of method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &envelope, amqp_envelope_class_entry) == FAILURE) {
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
	amqp_envelope_object *envelope;

	/* Try to pull amqp object out of method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &envelope, amqp_envelope_class_entry) == FAILURE) {
		return;
	}

	RETURN_STR(envelope->content_type);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getContentEncoding()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getContentEncoding)
{
	amqp_envelope_object *envelope;

	/* Try to pull amqp object out of method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &envelope, amqp_envelope_class_entry) == FAILURE) {
		return;
	}

	RETURN_STR(envelope->content_encoding);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getType()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getType)
{
	amqp_envelope_object *envelope;

	/* Try to pull amqp object out of method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &envelope, amqp_envelope_class_entry) == FAILURE) {
		return;
	}

	RETURN_STR(envelope->type);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getTimestamp()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getTimestamp)
{
	amqp_envelope_object *envelope;

	/* Try to pull amqp object out of method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &envelope, amqp_envelope_class_entry) == FAILURE) {
		return;
	}

	RETURN_LONG(envelope->timestamp);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getPriority()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getPriority)
{
	amqp_envelope_object *envelope;

	/* Try to pull amqp object out of method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &envelope, amqp_envelope_class_entry) == FAILURE) {
		return;
	}

	RETURN_LONG(envelope->priority);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getExpiration()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getExpiration)
{
	amqp_envelope_object *envelope;

	/* Try to pull amqp object out of method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &envelope, amqp_envelope_class_entry) == FAILURE) {
		return;
	}

	RETURN_STR(envelope->expiration);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getUserId()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getUserId)
{
	amqp_envelope_object *envelope;

	/* Try to pull amqp object out of method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &envelope, amqp_envelope_class_entry) == FAILURE) {
		return;
	}

	RETURN_STR(envelope->user_id);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getAppId()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getAppId)
{
	amqp_envelope_object *envelope;

	/* Try to pull amqp object out of method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &envelope, amqp_envelope_class_entry) == FAILURE) {
		return;
	}

	RETURN_STR(envelope->app_id);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getMessageId()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getMessageId)
{
	amqp_envelope_object *envelope;

	/* Try to pull amqp object out of method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &envelope, amqp_envelope_class_entry) == FAILURE) {
		return;
	}

	RETURN_STR(envelope->message_id);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getReplyTo()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getReplyTo)
{
	amqp_envelope_object *envelope;

	/* Try to pull amqp object out of method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &envelope, amqp_envelope_class_entry) == FAILURE) {
		return;
	}

	RETURN_STR(envelope->reply_to);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getCorrelationId()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getCorrelationId)
{
	amqp_envelope_object *envelope;

	/* Try to pull amqp object out of method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &envelope, amqp_envelope_class_entry) == FAILURE) {
		return;
	}

	RETURN_STR(envelope->correlation_id);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getHeader(string headerName)
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getHeader)
{
	zval *tmp;
	amqp_envelope_object *envelope;
	char *key;
	int key_len;

	/* Try to pull amqp object out of method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os", &envelope, amqp_envelope_class_entry, &key, &key_len) == FAILURE) {
		return;
	}

	/* Look for the hash key */
	if ((tmp = zend_hash_str_find(HASH_OF(envelope->headers), key, key_len)) == NULL) {
		RETURN_FALSE;
	}

	*return_value = *tmp;
	zval_copy_ctor(return_value);
	INIT_PZVAL(return_value);

}
/* }}} */

/* {{{ proto AMQPEnvelope::getHeaders()
check amqp envelope */
PHP_METHOD(amqp_envelope_class, getHeaders)
{
	amqp_envelope_object *envelope;

	/* Try to pull amqp object out of method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &envelope, amqp_envelope_class_entry) == FAILURE) {
		return;
	}

	zval_dtor(return_value);
	MAKE_COPY_ZVAL(&envelope->headers, return_value);
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
