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

void convert_amqp_envelope_to_zval(amqp_envelope_t *amqp_envelope, zval *envelope TSRMLS_DC) {
    zval *headers;

    /* Build the envelope */
    object_init_ex(envelope, this_ce);

    MAKE_STD_ZVAL(headers);
    array_init(headers);

    amqp_basic_properties_t *p = &amqp_envelope->message.properties;
    amqp_message_t *message = &amqp_envelope->message;

    zend_update_property_stringl(this_ce, envelope, ZEND_STRL("body"), (const char *) message->body.bytes, (int) message->body.len TSRMLS_CC);

    zend_update_property_long(this_ce, envelope, ZEND_STRL("delivery_tag"), (long) amqp_envelope->delivery_tag TSRMLS_CC);
    zend_update_property_bool(this_ce, envelope, ZEND_STRL("is_redelivery"), (long) amqp_envelope->redelivered TSRMLS_CC);
    zend_update_property_stringl(this_ce, envelope, ZEND_STRL("exchange_name"), (const char *) amqp_envelope->exchange.bytes, (int) amqp_envelope->exchange.len TSRMLS_CC);
    zend_update_property_stringl(this_ce, envelope, ZEND_STRL("routing_key"), (const char *) amqp_envelope->routing_key.bytes, (int) amqp_envelope->routing_key.len TSRMLS_CC);



    if (p->_flags & AMQP_BASIC_CONTENT_TYPE_FLAG) {
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("content_type"), (const char *) p->content_type.bytes, (int) p->content_type.len TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("content_type"), "", 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_CONTENT_ENCODING_FLAG) {
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("content_encoding"), (const char *) p->content_encoding.bytes, (int) p->content_encoding.len TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("content_encoding"), "", 0 TSRMLS_CC);
    }

    zend_update_property(this_ce, envelope, ZEND_STRL("headers"), headers TSRMLS_CC);

    if (p->_flags & AMQP_BASIC_HEADERS_FLAG) {
        parse_amqp_table(&(p->headers), headers);
    }

    if (p->_flags & AMQP_BASIC_DELIVERY_MODE_FLAG) {
        zend_update_property_long(this_ce, envelope, ZEND_STRL("delivery_mode"), (long) p->delivery_mode TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_long(this_ce, envelope, ZEND_STRL("delivery_mode"), AMQP_DELIVERY_NONPERSISTENT TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_PRIORITY_FLAG) {
        zend_update_property_long(this_ce, envelope, ZEND_STRL("priority"), (long) p->priority TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_long(this_ce, envelope, ZEND_STRL("priority"), 0 TSRMLS_CC);
    }


    if (p->_flags & AMQP_BASIC_CORRELATION_ID_FLAG) {
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("correlation_id"), (const char *) p->correlation_id.bytes, (int) p->correlation_id.len TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("correlation_id"), "", 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_REPLY_TO_FLAG) {
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("reply_to"), (const char *) p->reply_to.bytes, (int) p->reply_to.len TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("reply_to"), "", 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_EXPIRATION_FLAG) {
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("expiration"), (const char *) p->expiration.bytes, (int) p->expiration.len TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("expiration"), "", 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_MESSAGE_ID_FLAG) {
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("message_id"), (const char *) p->message_id.bytes, (int) p->message_id.len TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("message_id"), "", 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_TIMESTAMP_FLAG) {
        zend_update_property_long(this_ce, envelope, ZEND_STRL("timestamp"), (long) p->timestamp TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_long(this_ce, envelope, ZEND_STRL("timestamp"), 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_TYPE_FLAG) {
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("type"), (const char *) p->type.bytes, (int) p->type.len TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("type"), "", 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_USER_ID_FLAG) {
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("user_id"), (const char *) p->user_id.bytes, (int) p->user_id.len TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("user_id"), "", 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_APP_ID_FLAG) {
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("app_id"), (const char *) p->app_id.bytes, (int) p->app_id.len TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, envelope, ZEND_STRL("app_id"), "", 0 TSRMLS_CC);
    }

    zval_ptr_dtor(&headers);
}


/* {{{ proto AMQPEnvelope::__construct() */
PHP_METHOD (amqp_envelope_class, __construct) {
    PHP_AMQP_NOPARAMS();

    /* BC */
    zval *headers;

    MAKE_STD_ZVAL(headers);
    array_init(headers);

    zend_update_property_stringl(this_ce, getThis(), ZEND_STRL("body"), "", 0 TSRMLS_CC);

    zend_update_property_long(this_ce, getThis(), ZEND_STRL("delivery_tag"), 0 TSRMLS_CC);
    zend_update_property_bool(this_ce, getThis(), ZEND_STRL("is_redelivery"), 0 TSRMLS_CC);
    zend_update_property_stringl(this_ce, getThis(), ZEND_STRL("exchange_name"), "", 0 TSRMLS_CC);
    zend_update_property_stringl(this_ce, getThis(), ZEND_STRL("routing_key"), "", 0 TSRMLS_CC);

    zend_update_property_stringl(this_ce, getThis(), ZEND_STRL("content_type"), "", 0 TSRMLS_CC);
    zend_update_property_stringl(this_ce, getThis(), ZEND_STRL("content_encoding"), "", 0 TSRMLS_CC);
    zend_update_property(this_ce, getThis(), ZEND_STRL("headers"), headers TSRMLS_CC);
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

    zval_ptr_dtor(&headers);
}
/* }}} */


/* {{{ proto AMQPEnvelope::getBody()*/
PHP_METHOD (amqp_envelope_class, getBody) {
    PHP_AMQP_NOPARAMS();

    zval* zv = zend_read_property(this_ce, getThis(), ZEND_STRL("body"), 0 TSRMLS_CC);

    if (Z_STRLEN_P(zv) == 0) {
        /* BC */
        RETURN_FALSE;
    }

    RETURN_ZVAL(zv, 1, 0);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getRoutingKey() */
PHP_METHOD (amqp_envelope_class, getRoutingKey) {
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("routing_key");
}
/* }}} */

/* {{{ proto AMQPEnvelope::getDeliveryMode() */
PHP_METHOD (amqp_envelope_class, getDeliveryMode) {
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("delivery_mode");
}
/* }}} */


/* {{{ proto AMQPEnvelope::getDeliveryTag() */
PHP_METHOD (amqp_envelope_class, getDeliveryTag) {
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("delivery_tag");
}
/* }}} */

/* {{{ proto AMQPEnvelope::getExchangeName() */
PHP_METHOD (amqp_envelope_class, getExchangeName) {
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("exchange_name");
}
/* }}} */

/* {{{ proto AMQPEnvelope::isRedelivery() */
PHP_METHOD (amqp_envelope_class, isRedelivery) {
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("is_redelivery");
}
/* }}} */

/* {{{ proto AMQPEnvelope::getContentType() */
PHP_METHOD (amqp_envelope_class, getContentType) {
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("content_type");
}
/* }}} */

/* {{{ proto AMQPEnvelope::getContentEncoding() */
PHP_METHOD (amqp_envelope_class, getContentEncoding) {
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("content_encoding");
}
/* }}} */

/* {{{ proto AMQPEnvelope::getType() */
PHP_METHOD (amqp_envelope_class, getType) {
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("type");
}
/* }}} */

/* {{{ proto AMQPEnvelope::getTimestamp() */
PHP_METHOD (amqp_envelope_class, getTimestamp) {
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("timestamp");
}
/* }}} */

/* {{{ proto AMQPEnvelope::getPriority() */
PHP_METHOD (amqp_envelope_class, getPriority) {
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("priority");
}
/* }}} */

/* {{{ proto AMQPEnvelope::getExpiration()
check amqp envelope */
PHP_METHOD (amqp_envelope_class, getExpiration) {
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("expiration");
}
/* }}} */

/* {{{ proto AMQPEnvelope::getUserId() */
PHP_METHOD (amqp_envelope_class, getUserId) {
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("user_id");
}
/* }}} */

/* {{{ proto AMQPEnvelope::getAppId() */
PHP_METHOD (amqp_envelope_class, getAppId) {
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("app_id");
}
/* }}} */

/* {{{ proto AMQPEnvelope::getMessageId() */
PHP_METHOD (amqp_envelope_class, getMessageId) {
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("message_id");
}
/* }}} */

/* {{{ proto AMQPEnvelope::getReplyTo() */
PHP_METHOD (amqp_envelope_class, getReplyTo) {
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("reply_to");
}
/* }}} */

/* {{{ proto AMQPEnvelope::getCorrelationId() */
PHP_METHOD (amqp_envelope_class, getCorrelationId) {
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("correlation_id");
}
/* }}} */


/* {{{ proto AMQPEnvelope::getHeader(string name) */
PHP_METHOD (amqp_envelope_class, getHeader) {
    char *key;
    int key_len;
    zval **tmp;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &key_len) == FAILURE) {
        return;
    }

	zval* zv = zend_read_property(this_ce, getThis(), ZEND_STRL("headers"), 0 TSRMLS_CC);

	/* Look for the hash key */
	if (zend_hash_find(HASH_OF(zv),
					   key,
					   (uint) key_len + 1,
					   (void **) &tmp
	) == FAILURE) {
		RETURN_FALSE;
	}

    *return_value = **tmp;
    zval_copy_ctor(return_value);
    INIT_PZVAL(return_value);
}
/* }}} */


/* {{{ proto AMQPEnvelope::hasHeader(string name) */
PHP_METHOD (amqp_envelope_class, hasHeader) {
    char *key;
    int key_len;
    zval **tmp;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &key_len) == FAILURE) {
        return;
    }

    zval* zv = zend_read_property(this_ce, getThis(), ZEND_STRL("headers"), 0 TSRMLS_CC);

    /* Look for the hash key */
    if (zend_hash_find(HASH_OF(zv), key, (uint)(key_len + 1), (void **) &tmp) == FAILURE) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}
/* }}} */


/* {{{ proto AMQPEnvelope::getHeaders() */
PHP_METHOD (amqp_envelope_class, getHeaders) {
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

        {NULL, NULL, NULL}    /* Must be the last line in amqp_functions[] */
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
    zval *value;

    assert(Z_TYPE_P(result) == IS_ARRAY);

    for (i = 0; i < table->num_entries; i++) {
        MAKE_STD_ZVAL(value);

        amqp_table_entry_t *entry = &(table->entries[i]);
        switch (entry->value.kind) {
            case AMQP_FIELD_KIND_BOOLEAN:
                ZVAL_BOOL(value, entry->value.value.boolean);
                break;
            case AMQP_FIELD_KIND_I8: ZVAL_LONG(value, entry->value.value.i8);
                break;
            case AMQP_FIELD_KIND_U8: ZVAL_LONG(value, entry->value.value.u8);
                break;
            case AMQP_FIELD_KIND_I16: ZVAL_LONG(value, entry->value.value.i16);
                break;
            case AMQP_FIELD_KIND_U16: ZVAL_LONG(value, entry->value.value.u16);
                break;
            case AMQP_FIELD_KIND_I32: ZVAL_LONG(value, entry->value.value.i32);
                break;
            case AMQP_FIELD_KIND_U32: ZVAL_LONG(value, entry->value.value.u32);
                break;
            case AMQP_FIELD_KIND_I64: ZVAL_LONG(value, entry->value.value.i64);
                break;
            case AMQP_FIELD_KIND_U64: ZVAL_LONG(value, entry->value.value.i64);
                break;
            case AMQP_FIELD_KIND_F32: ZVAL_DOUBLE(value, entry->value.value.f32);
                break;
            case AMQP_FIELD_KIND_F64: ZVAL_DOUBLE(value, entry->value.value.f64);
                break;
            case AMQP_FIELD_KIND_UTF8:
            case AMQP_FIELD_KIND_BYTES:
                ZVAL_STRINGL(value, entry->value.value.bytes.bytes, entry->value.value.bytes.len, 1);
                break;
            case AMQP_FIELD_KIND_ARRAY: {
                int j;
                array_init(value);
                for (j = 0; j < entry->value.value.array.num_entries; ++j) {
                    switch (entry->value.value.array.entries[j].kind) {
                        case AMQP_FIELD_KIND_UTF8:
                            add_next_index_stringl(
                                    value,
                                    entry->value.value.array.entries[j].value.bytes.bytes,
                                    (uint) entry->value.value.array.entries[j].value.bytes.len,
                                    1
                            );
                            break;
                        case AMQP_FIELD_KIND_TABLE: {
                            zval *subtable;
                            MAKE_STD_ZVAL(subtable);
                            array_init(subtable);
                            parse_amqp_table(
                                    &(entry->value.value.array.entries[j].value.table),
                                    subtable
                            );
                            add_next_index_zval(value, subtable);
                        }
                            break;
                        default:
                            break;
                    }
                }
            }
                break;
            case AMQP_FIELD_KIND_TABLE:
                array_init(value);
                parse_amqp_table(&(entry->value.value.table), value);
                break;
            case AMQP_FIELD_KIND_TIMESTAMP: ZVAL_DOUBLE(value, entry->value.value.u64);
                break;
            case AMQP_FIELD_KIND_VOID:
            case AMQP_FIELD_KIND_DECIMAL:
            default: ZVAL_NULL(value);
                break;
        }

        if (Z_TYPE_P(value) != IS_NULL) {
            char *key = estrndup(entry->key.bytes, (uint) entry->key.len);
            add_assoc_zval(result, key, value);
            efree(key);
        } else {
            zval_dtor(value);
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
