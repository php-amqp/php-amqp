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

/* $Id: amqp_envelope.c 327551 2012-09-09 03:49:34Z pdezwart $ */

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

zend_class_entry *amqp_envelope_class_entry;
#define this_ce amqp_envelope_class_entry

void parse_amqp_table(amqp_table_t *table, zval *result);

void convert_amqp_envelope_to_zval(amqp_envelope_t *amqp_envelope, zval *envelope TSRMLS_DC)
{
    PHP5to7_zval_t headers PHP5to7_MAYBE_SET_TO_NULL;

    /* Build the envelope */
    object_init_ex(envelope, this_ce);

    PHP5to7_MAYBE_INIT(headers);
    PHP5to7_ARRAY_INIT(headers);

    amqp_basic_properties_t *p = &amqp_envelope->message.properties;
    amqp_message_t *message = &amqp_envelope->message;

    zend_update_property_stringl(this_ce, envelope, ZEND_STRL("body"), (const char *) message->body.bytes, (PHP5to7_param_str_len_type_t) message->body.len TSRMLS_CC);

    zend_update_property_long(this_ce, envelope, ZEND_STRL("delivery_tag"), (PHP5to7_param_long_type_t) amqp_envelope->delivery_tag TSRMLS_CC);
    zend_update_property_bool(this_ce, envelope, ZEND_STRL("is_redelivery"), (PHP5to7_param_long_type_t) amqp_envelope->redelivered TSRMLS_CC);
    zend_update_property_stringl(this_ce, envelope, ZEND_STRL("exchange_name"), (const char *) amqp_envelope->exchange.bytes, (PHP5to7_param_str_len_type_t) amqp_envelope->exchange.len TSRMLS_CC);
    zend_update_property_stringl(this_ce, envelope, ZEND_STRL("routing_key"), (const char *) amqp_envelope->routing_key.bytes, (PHP5to7_param_str_len_type_t) amqp_envelope->routing_key.len TSRMLS_CC);



    if (p->_flags & AMQP_BASIC_CONTENT_TYPE_FLAG) {
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("content_type"), (const char *) p->content_type.bytes, (PHP5to7_param_str_len_type_t) p->content_type.len TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("content_type"), "", 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_CONTENT_ENCODING_FLAG) {
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("content_encoding"), (const char *) p->content_encoding.bytes, (PHP5to7_param_str_len_type_t) p->content_encoding.len TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("content_encoding"), "", 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_HEADERS_FLAG) {
        parse_amqp_table(&(p->headers), PHP5to7_MAYBE_PTR(headers));
    }

    zend_update_property(this_ce, envelope, ZEND_STRL("headers"), PHP5to7_MAYBE_PTR(headers) TSRMLS_CC);

    if (p->_flags & AMQP_BASIC_DELIVERY_MODE_FLAG) {
        zend_update_property_long(this_ce, envelope, ZEND_STRL("delivery_mode"), (PHP5to7_param_long_type_t) p->delivery_mode TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_long(this_ce, envelope, ZEND_STRL("delivery_mode"), AMQP_DELIVERY_NONPERSISTENT TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_PRIORITY_FLAG) {
        zend_update_property_long(this_ce, envelope, ZEND_STRL("priority"), (PHP5to7_param_long_type_t) p->priority TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_long(this_ce, envelope, ZEND_STRL("priority"), 0 TSRMLS_CC);
    }


    if (p->_flags & AMQP_BASIC_CORRELATION_ID_FLAG) {
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("correlation_id"), (const char *) p->correlation_id.bytes, (PHP5to7_param_str_len_type_t) p->correlation_id.len TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("correlation_id"), "", 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_REPLY_TO_FLAG) {
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("reply_to"), (const char *) p->reply_to.bytes, (PHP5to7_param_str_len_type_t) p->reply_to.len TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("reply_to"), "", 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_EXPIRATION_FLAG) {
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("expiration"), (const char *) p->expiration.bytes, (PHP5to7_param_str_len_type_t) p->expiration.len TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("expiration"), "", 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_MESSAGE_ID_FLAG) {
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("message_id"), (const char *) p->message_id.bytes, (PHP5to7_param_str_len_type_t) p->message_id.len TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("message_id"), "", 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_TIMESTAMP_FLAG) {
        zend_update_property_long(this_ce, envelope, ZEND_STRL("timestamp"), (PHP5to7_param_long_type_t) p->timestamp TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_long(this_ce, envelope, ZEND_STRL("timestamp"), 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_TYPE_FLAG) {
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("type"), (const char *) p->type.bytes, (PHP5to7_param_str_len_type_t) p->type.len TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("type"), "", 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_USER_ID_FLAG) {
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("user_id"), (const char *) p->user_id.bytes, (PHP5to7_param_str_len_type_t) p->user_id.len TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("user_id"), "", 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_APP_ID_FLAG) {
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("app_id"), (const char *) p->app_id.bytes, (PHP5to7_param_str_len_type_t) p->app_id.len TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("app_id"), "", 0 TSRMLS_CC);
    }

    PHP5to7_MAYBE_DESTROY(headers);
}


/* {{{ proto AMQPEnvelope::__construct() */
PHP_METHOD (amqp_envelope_class, __construct) {
    PHP_AMQP_NOPARAMS();

    /* BC */
    PHP5to7_zval_t headers PHP5to7_MAYBE_SET_TO_NULL;
    PHP5to7_MAYBE_INIT(headers);
    PHP5to7_ARRAY_INIT(headers);

    zend_update_property_stringl(this_ce, getThis(), ZEND_STRL("body"), "", 0 TSRMLS_CC);

    zend_update_property_long(this_ce, getThis(), ZEND_STRL("delivery_tag"), 0 TSRMLS_CC);
    zend_update_property_bool(this_ce, getThis(), ZEND_STRL("is_redelivery"), 0 TSRMLS_CC);
    zend_update_property_stringl(this_ce, getThis(), ZEND_STRL("exchange_name"), "", 0 TSRMLS_CC);
    zend_update_property_stringl(this_ce, getThis(), ZEND_STRL("routing_key"), "", 0 TSRMLS_CC);

    zend_update_property_stringl(this_ce, getThis(), ZEND_STRL("content_type"), "", 0 TSRMLS_CC);
    zend_update_property_stringl(this_ce, getThis(), ZEND_STRL("content_encoding"), "", 0 TSRMLS_CC);
    zend_update_property(this_ce, getThis(), ZEND_STRL("headers"), PHP5to7_MAYBE_PTR(headers) TSRMLS_CC);
    zend_update_property_long(this_ce, getThis(), ZEND_STRL("delivery_mode"), AMQP_DELIVERY_NONPERSISTENT TSRMLS_CC);
    zend_update_property_long(this_ce, getThis(), ZEND_STRL("priority"), 0 TSRMLS_CC);
    zend_update_property_stringl(this_ce, getThis(), ZEND_STRL("correlation_id"), "", 0 TSRMLS_CC);
    zend_update_property_stringl(this_ce, getThis(), ZEND_STRL("reply_to"), "", 0 TSRMLS_CC);
    zend_update_property_stringl(this_ce, getThis(), ZEND_STRL("expiration"), "", 0 TSRMLS_CC);
    zend_update_property_stringl(this_ce, getThis(), ZEND_STRL("message_id"), "", 0 TSRMLS_CC);
    zend_update_property_long(this_ce, getThis(), ZEND_STRL("timestamp"), 0 TSRMLS_CC);
    zend_update_property_stringl(this_ce, getThis(), ZEND_STRL("type"), "", 0 TSRMLS_CC);
    zend_update_property_stringl(this_ce, getThis(), ZEND_STRL("user_id"), "", 0 TSRMLS_CC);
    zend_update_property_stringl(this_ce, getThis(), ZEND_STRL("app_id"), "", 0 TSRMLS_CC);

    PHP5to7_MAYBE_DESTROY(headers);
}
/* }}} */


/* {{{ proto AMQPEnvelope::getBody()*/
PHP_METHOD (amqp_envelope_class, getBody) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;

    PHP_AMQP_NOPARAMS();

    zval* zv = PHP_AMQP_READ_THIS_PROP("body");

    if (Z_STRLEN_P(zv) == 0) {
        /* BC */
        RETURN_FALSE;
    }

    RETURN_ZVAL(zv, 1, 0);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getRoutingKey() */
PHP_METHOD (amqp_envelope_class, getRoutingKey) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("routing_key");
}
/* }}} */

/* {{{ proto AMQPEnvelope::getDeliveryMode() */
PHP_METHOD (amqp_envelope_class, getDeliveryMode) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("delivery_mode");
}
/* }}} */


/* {{{ proto AMQPEnvelope::getDeliveryTag() */
PHP_METHOD (amqp_envelope_class, getDeliveryTag) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("delivery_tag");
}
/* }}} */

/* {{{ proto AMQPEnvelope::getExchangeName() */
PHP_METHOD (amqp_envelope_class, getExchangeName) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("exchange_name");
}
/* }}} */

/* {{{ proto AMQPEnvelope::isRedelivery() */
PHP_METHOD (amqp_envelope_class, isRedelivery) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("is_redelivery");
}
/* }}} */

/* {{{ proto AMQPEnvelope::getContentType() */
PHP_METHOD (amqp_envelope_class, getContentType) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("content_type");
}
/* }}} */

/* {{{ proto AMQPEnvelope::getContentEncoding() */
PHP_METHOD (amqp_envelope_class, getContentEncoding) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("content_encoding");
}
/* }}} */

/* {{{ proto AMQPEnvelope::getType() */
PHP_METHOD (amqp_envelope_class, getType) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("type");
}
/* }}} */

/* {{{ proto AMQPEnvelope::getTimestamp() */
PHP_METHOD (amqp_envelope_class, getTimestamp) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("timestamp");
}
/* }}} */

/* {{{ proto AMQPEnvelope::getPriority() */
PHP_METHOD (amqp_envelope_class, getPriority) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("priority");
}
/* }}} */

/* {{{ proto AMQPEnvelope::getExpiration()
check amqp envelope */
PHP_METHOD (amqp_envelope_class, getExpiration) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("expiration");
}
/* }}} */

/* {{{ proto AMQPEnvelope::getUserId() */
PHP_METHOD (amqp_envelope_class, getUserId) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("user_id");
}
/* }}} */

/* {{{ proto AMQPEnvelope::getAppId() */
PHP_METHOD (amqp_envelope_class, getAppId) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("app_id");
}
/* }}} */

/* {{{ proto AMQPEnvelope::getMessageId() */
PHP_METHOD (amqp_envelope_class, getMessageId) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("message_id");
}
/* }}} */

/* {{{ proto AMQPEnvelope::getReplyTo() */
PHP_METHOD (amqp_envelope_class, getReplyTo) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("reply_to");
}
/* }}} */

/* {{{ proto AMQPEnvelope::getCorrelationId() */
PHP_METHOD (amqp_envelope_class, getCorrelationId) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("correlation_id");
}
/* }}} */


/* {{{ proto AMQPEnvelope::getHeader(string name) */
PHP_METHOD (amqp_envelope_class, getHeader) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;

    char *key;  PHP5to7_param_str_len_type_t key_len;
    PHP5to7_zval_t *tmp = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &key_len) == FAILURE) {
        return;
    }

	zval* zv = PHP_AMQP_READ_THIS_PROP("headers");

    /* Look for the hash key */
    if (!PHP5to7_ZEND_HASH_FIND(HASH_OF(zv), key, key_len + 1, tmp)) {
        RETURN_FALSE;
    }

    RETURN_ZVAL(PHP5to7_MAYBE_DEREF(tmp), 1, 0);
}
/* }}} */


/* {{{ proto AMQPEnvelope::hasHeader(string name) */
PHP_METHOD (amqp_envelope_class, hasHeader) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;

    char *key;  PHP5to7_param_str_len_type_t key_len;
    PHP5to7_zval_t *tmp = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &key_len) == FAILURE) {
        return;
    }

    zval* zv = PHP_AMQP_READ_THIS_PROP("headers");

    /* Look for the hash key */
    if (!PHP5to7_ZEND_HASH_FIND(HASH_OF(zv), key, key_len + 1, tmp)) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}
/* }}} */

/* {{{ proto AMQPEnvelope::getHeaders() */
PHP_METHOD (amqp_envelope_class, getHeaders) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("headers");
}
/* }}} */

/* amqp_envelope_class ARG_INFO definition */
ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class__construct, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getBody, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getRoutingKey, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getDeliveryTag, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getDeliveryMode, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getExchangeName, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_isRedelivery, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getContentType, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getContentEncoding, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getType, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getTimestamp, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getPriority, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getExpiration, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getUserId, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getAppId, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getMessageId, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getReplyTo, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getCorrelationId, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getHeaders, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_getHeader, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
                ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class_hasHeader, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
                ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

zend_function_entry amqp_envelope_class_functions[] = {
        PHP_ME(amqp_envelope_class, __construct, arginfo_amqp_envelope_class__construct, ZEND_ACC_PUBLIC)
        PHP_ME(amqp_envelope_class, getBody, arginfo_amqp_envelope_class_getBody, ZEND_ACC_PUBLIC)
        PHP_ME(amqp_envelope_class, getRoutingKey, arginfo_amqp_envelope_class_getRoutingKey, ZEND_ACC_PUBLIC)
        PHP_ME(amqp_envelope_class, getDeliveryTag, arginfo_amqp_envelope_class_getDeliveryTag, ZEND_ACC_PUBLIC)
        PHP_ME(amqp_envelope_class, getDeliveryMode, arginfo_amqp_envelope_class_getDeliveryMode, ZEND_ACC_PUBLIC)
        PHP_ME(amqp_envelope_class, getExchangeName, arginfo_amqp_envelope_class_getExchangeName, ZEND_ACC_PUBLIC)
        PHP_ME(amqp_envelope_class, isRedelivery, arginfo_amqp_envelope_class_isRedelivery, ZEND_ACC_PUBLIC)
        PHP_ME(amqp_envelope_class, getContentType, arginfo_amqp_envelope_class_getContentType, ZEND_ACC_PUBLIC)
        PHP_ME(amqp_envelope_class, getContentEncoding, arginfo_amqp_envelope_class_getContentEncoding, ZEND_ACC_PUBLIC)
        PHP_ME(amqp_envelope_class, getType, arginfo_amqp_envelope_class_getType, ZEND_ACC_PUBLIC)
        PHP_ME(amqp_envelope_class, getTimestamp, arginfo_amqp_envelope_class_getTimestamp, ZEND_ACC_PUBLIC)
        PHP_ME(amqp_envelope_class, getPriority, arginfo_amqp_envelope_class_getPriority, ZEND_ACC_PUBLIC)
        PHP_ME(amqp_envelope_class, getExpiration, arginfo_amqp_envelope_class_getExpiration, ZEND_ACC_PUBLIC)
        PHP_ME(amqp_envelope_class, getUserId, arginfo_amqp_envelope_class_getUserId, ZEND_ACC_PUBLIC)
        PHP_ME(amqp_envelope_class, getAppId, arginfo_amqp_envelope_class_getAppId, ZEND_ACC_PUBLIC)
        PHP_ME(amqp_envelope_class, getMessageId, arginfo_amqp_envelope_class_getMessageId, ZEND_ACC_PUBLIC)
        PHP_ME(amqp_envelope_class, getReplyTo, arginfo_amqp_envelope_class_getReplyTo, ZEND_ACC_PUBLIC)
        PHP_ME(amqp_envelope_class, getCorrelationId, arginfo_amqp_envelope_class_getCorrelationId, ZEND_ACC_PUBLIC)
        PHP_ME(amqp_envelope_class, getHeaders, arginfo_amqp_envelope_class_getHeaders, ZEND_ACC_PUBLIC)
        PHP_ME(amqp_envelope_class, getHeader, arginfo_amqp_envelope_class_getHeader, ZEND_ACC_PUBLIC)
        PHP_ME(amqp_envelope_class, hasHeader, arginfo_amqp_envelope_class_hasHeader, ZEND_ACC_PUBLIC)

        {NULL, NULL, NULL}
};


PHP_MINIT_FUNCTION (amqp_envelope) {
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "AMQPEnvelope", amqp_envelope_class_functions);
    this_ce = zend_register_internal_class(&ce TSRMLS_CC);


    zend_declare_property_null(this_ce, ZEND_STRL("body"), ZEND_ACC_PRIVATE TSRMLS_CC);

    /*
    deliver(consumer-tag consumer-tag,
            delivery-tag delivery-tag,
            redelivered redelivered,
            exchange-name exchange,
            shortstr routing-key)

    get-ok(delivery-tag delivery-tag,
           redelivered redelivered,
           exchange-name exchange,
           shortstr routing-key,
           message-count message-count)
     */
    zend_declare_property_null(this_ce, ZEND_STRL("delivery_tag"), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(this_ce, ZEND_STRL("is_redelivery"), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(this_ce, ZEND_STRL("exchange_name"), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(this_ce, ZEND_STRL("routing_key"), ZEND_ACC_PRIVATE TSRMLS_CC);

    /*
    shortstr content-type	MIME content type.
    shortstr content-encoding	MIME content encoding.
    table headers	Message header field table.
    octet delivery-mode	Non-persistent (1) or persistent (2).
    octet priority	Message priority, 0 to 9.
    shortstr correlation-id	Application correlation identifier.
    shortstr reply-to	Address to reply to.
    shortstr expiration	Message expiration specification.
    shortstr message-id	Application message identifier.
    timestamp timestamp	Message timestamp.
    shortstr type	Message type name.
    shortstr user-id	Creating user id.
    shortstr app-id	Creating application id.
    */
    zend_declare_property_null(this_ce, ZEND_STRL("content_type"), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(this_ce, ZEND_STRL("content_encoding"), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(this_ce, ZEND_STRL("headers"), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(this_ce, ZEND_STRL("delivery_mode"), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(this_ce, ZEND_STRL("priority"), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(this_ce, ZEND_STRL("correlation_id"), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(this_ce, ZEND_STRL("reply_to"), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(this_ce, ZEND_STRL("expiration"), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(this_ce, ZEND_STRL("message_id"), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(this_ce, ZEND_STRL("timestamp"), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(this_ce, ZEND_STRL("type"), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(this_ce, ZEND_STRL("user_id"), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(this_ce, ZEND_STRL("app_id"), ZEND_ACC_PRIVATE TSRMLS_CC);

    return SUCCESS;
}


void parse_amqp_table(amqp_table_t *table, zval *result) {
    int i;
    PHP5to7_zval_t value PHP5to7_MAYBE_SET_TO_NULL;

    assert(Z_TYPE_P(result) == IS_ARRAY);

    for (i = 0; i < table->num_entries; i++) {
        PHP5to7_MAYBE_INIT(value);

        amqp_table_entry_t *entry = &(table->entries[i]);
        switch (entry->value.kind) {
            case AMQP_FIELD_KIND_BOOLEAN:
                ZVAL_BOOL(PHP5to7_MAYBE_PTR(value), entry->value.value.boolean);
                break;
            case AMQP_FIELD_KIND_I8: ZVAL_LONG(PHP5to7_MAYBE_PTR(value), entry->value.value.i8);
                break;
            case AMQP_FIELD_KIND_U8: ZVAL_LONG(PHP5to7_MAYBE_PTR(value), entry->value.value.u8);
                break;
            case AMQP_FIELD_KIND_I16: ZVAL_LONG(PHP5to7_MAYBE_PTR(value), entry->value.value.i16);
                break;
            case AMQP_FIELD_KIND_U16: ZVAL_LONG(PHP5to7_MAYBE_PTR(value), entry->value.value.u16);
                break;
            case AMQP_FIELD_KIND_I32: ZVAL_LONG(PHP5to7_MAYBE_PTR(value), entry->value.value.i32);
                break;
            case AMQP_FIELD_KIND_U32: ZVAL_LONG(PHP5to7_MAYBE_PTR(value), entry->value.value.u32);
                break;
            case AMQP_FIELD_KIND_I64: ZVAL_LONG(PHP5to7_MAYBE_PTR(value), entry->value.value.i64);
                break;
            case AMQP_FIELD_KIND_U64: ZVAL_LONG(PHP5to7_MAYBE_PTR(value), entry->value.value.i64);
                break;
            case AMQP_FIELD_KIND_F32: ZVAL_DOUBLE(PHP5to7_MAYBE_PTR(value), entry->value.value.f32);
                break;
            case AMQP_FIELD_KIND_F64: ZVAL_DOUBLE(PHP5to7_MAYBE_PTR(value), entry->value.value.f64);
                break;
            case AMQP_FIELD_KIND_UTF8:
            case AMQP_FIELD_KIND_BYTES:
                PHP5to7_ZVAL_STRINGL_DUP(PHP5to7_MAYBE_PTR(value), entry->value.value.bytes.bytes, entry->value.value.bytes.len);
                break;
            case AMQP_FIELD_KIND_ARRAY: {
                int j;
                array_init(PHP5to7_MAYBE_PTR(value));
                for (j = 0; j < entry->value.value.array.num_entries; ++j) {
                    switch (entry->value.value.array.entries[j].kind) {
                        case AMQP_FIELD_KIND_UTF8:
                            PHP5to7_ADD_NEXT_INDEX_STRINGL_DUP(
                                    PHP5to7_MAYBE_PTR(value),
                                    entry->value.value.array.entries[j].value.bytes.bytes,
                                    (uint) entry->value.value.array.entries[j].value.bytes.len
                            );
                            break;
                        case AMQP_FIELD_KIND_TABLE: {
                            PHP5to7_zval_t subtable PHP5to7_MAYBE_SET_TO_NULL;
                            PHP5to7_MAYBE_INIT(subtable);
                            PHP5to7_ARRAY_INIT(subtable);

                            parse_amqp_table(
                                    &(entry->value.value.array.entries[j].value.table),
                                    PHP5to7_MAYBE_PTR(subtable)
                            );
                            add_next_index_zval(PHP5to7_MAYBE_PTR(value), PHP5to7_MAYBE_PTR(subtable));
                        }
                            break;
                        default:
                            break;
                    }
                }
            }
                break;
            case AMQP_FIELD_KIND_TABLE:
                PHP5to7_ARRAY_INIT(value);
                parse_amqp_table(&(entry->value.value.table), PHP5to7_MAYBE_PTR(value));
                break;
            case AMQP_FIELD_KIND_TIMESTAMP: ZVAL_DOUBLE(PHP5to7_MAYBE_PTR(value), entry->value.value.u64);
                break;
            case AMQP_FIELD_KIND_VOID:
            case AMQP_FIELD_KIND_DECIMAL:
            default: ZVAL_NULL(PHP5to7_MAYBE_PTR(value));
                break;
        }

        if (Z_TYPE_P(PHP5to7_MAYBE_PTR(value)) != IS_NULL) {
            char *key = estrndup(entry->key.bytes, (uint) entry->key.len);
            add_assoc_zval(result, key, PHP5to7_MAYBE_PTR(value));
            efree(key);
        } else {
            PHP5to7_MAYBE_DESTROY(value);
        }
    }
    return;
}

/*
*Local variables:
*tab-width: 4
*c-basic-offset: 4
*End:
*vim600: noet sw=4 ts=4 fdm=marker
*vim<600: noet sw=4 ts=4
*/
