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
#include "zend_exceptions.h"

#ifdef PHP_WIN32
    #if PHP_VERSION_ID >= 80000
        #include <stdint.h>
    #else
        #include "win32/php_stdint.h"
    #endif
    #include "win32/signal.h"
#else
    #include <signal.h>
    #include <stdint.h>
#endif

#if HAVE_LIBRABBITMQ_NEW_LAYOUT
    #include <rabbitmq-c/amqp.h>
    #include <rabbitmq-c/framing.h>
#else
    #include <amqp.h>
    #include <amqp_framing.h>
#endif

#ifdef PHP_WIN32
    #include "win32/unistd.h"
#else
    #include <unistd.h>
#endif

#include "amqp_envelope.h"
#include "amqp_basic_properties.h"
#include "php_amqp.h"

zend_class_entry *amqp_envelope_class_entry;
#define this_ce amqp_envelope_class_entry


void convert_amqp_envelope_to_zval(amqp_envelope_t *amqp_envelope, zval *envelope)
{
    /* Build the envelope */
    object_init_ex(envelope, this_ce);


    amqp_basic_properties_t *p = &amqp_envelope->message.properties;
    amqp_message_t *message = &amqp_envelope->message;

    zend_update_property_stringl(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(envelope),
        ZEND_STRL("body"),
        (const char *) message->body.bytes,
        (size_t) message->body.len
    );

    zend_update_property_stringl(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(envelope),
        ZEND_STRL("consumerTag"),
        (const char *) amqp_envelope->consumer_tag.bytes,
        (size_t) amqp_envelope->consumer_tag.len
    );
    zend_update_property_long(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(envelope),
        ZEND_STRL("deliveryTag"),
        (zend_long) amqp_envelope->delivery_tag
    );
    zend_update_property_bool(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(envelope),
        ZEND_STRL("isRedelivery"),
        (zend_long) amqp_envelope->redelivered
    );
    zend_update_property_stringl(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(envelope),
        ZEND_STRL("exchangeName"),
        (const char *) amqp_envelope->exchange.bytes,
        (size_t) amqp_envelope->exchange.len
    );
    zend_update_property_stringl(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(envelope),
        ZEND_STRL("routingKey"),
        (const char *) amqp_envelope->routing_key.bytes,
        (size_t) amqp_envelope->routing_key.len
    );

    php_amqp_basic_properties_extract(p, envelope);
}

/* {{{ proto AMQPEnvelope::__construct() */
static PHP_METHOD(amqp_envelope_class, __construct)
{
    PHP_AMQP_NOPARAMS()

    /* BC */
    php_amqp_basic_properties_set_empty_headers(getThis());
}
/* }}} */


/* {{{ proto AMQPEnvelope::getBody()*/
static PHP_METHOD(amqp_envelope_class, getBody)
{
    zval rv;

    PHP_AMQP_NOPARAMS()

    zval *zv = PHP_AMQP_READ_THIS_PROP("body");

    if (Z_STRLEN_P(zv) == 0) {
        RETURN_STRING("");
    }

    RETURN_ZVAL(zv, 1, 0);
}
/* }}} */

/* {{{ proto AMQPEnvelope::getRoutingKey() */
static PHP_METHOD(amqp_envelope_class, getRoutingKey)
{
    zval rv;
    PHP_AMQP_NOPARAMS()
    PHP_AMQP_RETURN_THIS_PROP("routingKey");
}
/* }}} */

/* {{{ proto AMQPEnvelope::getDeliveryTag() */
static PHP_METHOD(amqp_envelope_class, getDeliveryTag)
{
    zval rv;
    PHP_AMQP_NOPARAMS()
    PHP_AMQP_RETURN_THIS_PROP("deliveryTag");
}
/* }}} */

/* {{{ proto AMQPEnvelope::getConsumerTag() */
static PHP_METHOD(amqp_envelope_class, getConsumerTag)
{
    zval rv;
    PHP_AMQP_NOPARAMS()
    PHP_AMQP_RETURN_THIS_PROP("consumerTag");
}
/* }}} */

/* {{{ proto AMQPEnvelope::getExchangeName() */
static PHP_METHOD(amqp_envelope_class, getExchangeName)
{
    zval rv;
    PHP_AMQP_NOPARAMS()
    PHP_AMQP_RETURN_THIS_PROP("exchangeName");
}
/* }}} */

/* {{{ proto AMQPEnvelope::isRedelivery() */
static PHP_METHOD(amqp_envelope_class, isRedelivery)
{
    zval rv;
    PHP_AMQP_NOPARAMS()
    PHP_AMQP_RETURN_THIS_PROP("isRedelivery");
}
/* }}} */


/* {{{ proto AMQPEnvelope::getHeader(string name) */
static PHP_METHOD(amqp_envelope_class, getHeader)
{
    zval rv;

    char *key;
    size_t key_len;
    zval *tmp = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &key, &key_len) == FAILURE) {
        RETURN_THROWS();
    }

    zval *zv = PHP_AMQP_READ_THIS_PROP_CE("headers", amqp_basic_properties_class_entry);

    /* Look for the hash key */
    if ((tmp = zend_hash_str_find(HASH_OF(zv), key, key_len)) == NULL) {
        RETURN_NULL();
    }

    RETURN_ZVAL(tmp, 1, 0);
}
/* }}} */


/* {{{ proto AMQPEnvelope::hasHeader(string name) */
static PHP_METHOD(amqp_envelope_class, hasHeader)
{
    zval rv;

    char *key;
    size_t key_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &key, &key_len) == FAILURE) {
        RETURN_THROWS();
    }

    zval *zv = PHP_AMQP_READ_THIS_PROP_CE("headers", amqp_basic_properties_class_entry);

    /* Look for the hash key */
    RETURN_BOOL(zend_hash_str_find(HASH_OF(zv), key, key_len) != NULL);
}
/* }}} */


/* amqp_envelope_class ARG_INFO definition */
ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_envelope_class__construct, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_envelope_class_getBody, ZEND_SEND_BY_VAL, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_envelope_class_getRoutingKey, ZEND_SEND_BY_VAL, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_envelope_class_getConsumerTag, ZEND_SEND_BY_VAL, 0, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_envelope_class_getDeliveryTag, ZEND_SEND_BY_VAL, 0, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_envelope_class_getExchangeName, ZEND_SEND_BY_VAL, 0, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_envelope_class_isRedelivery, ZEND_SEND_BY_VAL, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_envelope_class_getHeader, ZEND_SEND_BY_VAL, 1, IS_STRING, 1)
    ZEND_ARG_TYPE_INFO(0, headerName, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_envelope_class_hasHeader, ZEND_SEND_BY_VAL, 1, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO(0, headerName, IS_STRING, 0)
ZEND_END_ARG_INFO()


zend_function_entry amqp_envelope_class_functions[] = {
    PHP_ME(amqp_envelope_class, __construct, arginfo_amqp_envelope_class__construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)

    PHP_ME(amqp_envelope_class, getBody, arginfo_amqp_envelope_class_getBody, ZEND_ACC_PUBLIC)

    PHP_ME(amqp_envelope_class, getRoutingKey, arginfo_amqp_envelope_class_getRoutingKey, ZEND_ACC_PUBLIC)
    PHP_ME(amqp_envelope_class, getConsumerTag, arginfo_amqp_envelope_class_getConsumerTag, ZEND_ACC_PUBLIC)
    PHP_ME(amqp_envelope_class, getDeliveryTag, arginfo_amqp_envelope_class_getDeliveryTag, ZEND_ACC_PUBLIC)
    PHP_ME(amqp_envelope_class, getExchangeName, arginfo_amqp_envelope_class_getExchangeName, ZEND_ACC_PUBLIC)
    PHP_ME(amqp_envelope_class, isRedelivery, arginfo_amqp_envelope_class_isRedelivery, ZEND_ACC_PUBLIC)

    PHP_ME(amqp_envelope_class, getHeader, arginfo_amqp_envelope_class_getHeader, ZEND_ACC_PUBLIC)
    PHP_ME(amqp_envelope_class, hasHeader, arginfo_amqp_envelope_class_hasHeader, ZEND_ACC_PUBLIC)

    {NULL, NULL, NULL}
};


PHP_MINIT_FUNCTION(amqp_envelope)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "AMQPEnvelope", amqp_envelope_class_functions);
    this_ce = zend_register_internal_class_ex(&ce, amqp_basic_properties_class_entry);

    PHP_AMQP_DECLARE_TYPED_PROPERTY_WITH_DEFAULT(this_ce, "body", ZEND_ACC_PRIVATE, IS_STRING, 0, ZVAL_EMPTY_STRING);
    PHP_AMQP_DECLARE_TYPED_PROPERTY(this_ce, "consumerTag", ZEND_ACC_PRIVATE, IS_STRING, 1);
    PHP_AMQP_DECLARE_TYPED_PROPERTY(this_ce, "deliveryTag", ZEND_ACC_PRIVATE, IS_LONG, 1);
    PHP_AMQP_DECLARE_TYPED_PROPERTY_WITH_DEFAULT(this_ce, "isRedelivery", ZEND_ACC_PRIVATE, _IS_BOOL, 0, ZVAL_FALSE);
    PHP_AMQP_DECLARE_TYPED_PROPERTY_WITH_DEFAULT(this_ce, "exchangeName", ZEND_ACC_PRIVATE, IS_STRING, 1, ZVAL_NULL);
    PHP_AMQP_DECLARE_TYPED_PROPERTY_WITH_DEFAULT(
        this_ce,
        "routingKey",
        ZEND_ACC_PRIVATE,
        IS_STRING,
        0,
        ZVAL_EMPTY_STRING
    );

    return SUCCESS;
}
