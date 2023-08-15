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
#include "Zend/zend_interfaces.h"

#if HAVE_LIBRABBITMQ_NEW_LAYOUT
    #include <rabbitmq-c/amqp.h>
    #include <rabbitmq-c/framing.h>
#else
    #include <amqp.h>
    #include <amqp_framing.h>
#endif

#ifdef PHP_WIN32
    #include "win32/unistd.h"
    #if PHP_VERSION_ID >= 80000
        #include <stdint.h>
    #else
        #include "win32/php_stdint.h"
    #endif
    #include "win32/signal.h"
#else
    #include <signal.h>
    #include <stdint.h>
    #include <unistd.h>
#endif

#if HAVE_INTTYPES_H
    #include <inttypes.h>
#endif

#include "amqp_basic_properties.h"
#include "php_amqp.h"
#include "amqp_timestamp.h"
#include "amqp_decimal.h"

void parse_amqp_array(amqp_array_t *array, zval *result);
zend_bool parse_amqp_value(amqp_field_value_t *value, zval *result);

zend_class_entry *amqp_basic_properties_class_entry;
#define this_ce amqp_basic_properties_class_entry


void php_amqp_basic_properties_convert_to_zval(amqp_basic_properties_t *props, zval *obj)
{
    object_init_ex(obj, this_ce);

    php_amqp_basic_properties_extract(props, obj);
}

void php_amqp_basic_properties_set_empty_headers(zval *obj)
{
    zval headers;

    ZVAL_UNDEF(&headers);
    array_init(&headers);

    zend_update_property(this_ce, PHP_AMQP_COMPAT_OBJ_P(obj), ZEND_STRL("headers"), &headers);

    zval_ptr_dtor(&headers);
}


/* {{{ proto AMQPBasicProperties::__construct() */
static PHP_METHOD(AMQPBasicProperties, __construct)
{
    char *content_type = NULL;
    size_t content_type_len = 0;

    char *content_encoding = NULL;
    size_t content_encoding_len = 0;

    zval *headers = NULL;

    zend_long delivery_mode = AMQP_DELIVERY_NONPERSISTENT;
    zend_long priority = 0;

    char *correlation_id = NULL;
    size_t correlation_id_len = 0;

    char *reply_to = NULL;
    size_t reply_to_len = 0;

    char *expiration = NULL;
    size_t expiration_len = 0;

    char *message_id = NULL;
    size_t message_id_len = 0;

    zend_long timestamp = 0;

    char *type = NULL;
    size_t type_len = 0;

    char *user_id = NULL;
    size_t user_id_len = 0;

    char *app_id = NULL;
    size_t app_id_len = 0;

    char *cluster_id = NULL;
    size_t cluster_id_len = 0;

    if (zend_parse_parameters(
            ZEND_NUM_ARGS(),
            "|ssallsssslssss",
            /* s */ &content_type,
            &content_type_len,
            /* s */ &content_encoding,
            &content_encoding_len,
            /* a */ &headers,
            /* l */ &delivery_mode,
            /* l */ &priority,
            /* s */ &correlation_id,
            &correlation_id_len,
            /* s */ &reply_to,
            &reply_to_len,
            /* s */ &expiration,
            &expiration_len,
            /* s */ &message_id,
            &message_id_len,
            /* l */ &timestamp,
            /* s */ &type,
            &type_len,
            /* s */ &user_id,
            &user_id_len,
            /* s */ &app_id,
            &app_id_len,
            /* s */ &cluster_id,
            &cluster_id_len
        ) == FAILURE) {
        return;
    }
    zend_update_property_stringl(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("contentType"),
        content_type,
        content_type_len
    );
    zend_update_property_stringl(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("contentEncoding"),
        content_encoding,
        content_encoding_len
    );

    if (headers != NULL) {
        zend_update_property(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("headers"), headers);
    } else {
        php_amqp_basic_properties_set_empty_headers(getThis());
    }

    zend_update_property_long(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("deliveryMode"), delivery_mode);
    zend_update_property_long(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("priority"), priority);

    zend_update_property_stringl(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("correlationId"),
        correlation_id,
        correlation_id_len
    );
    zend_update_property_stringl(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("replyTo"),
        reply_to,
        reply_to_len
    );
    zend_update_property_stringl(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("expiration"),
        expiration,
        expiration_len
    );
    zend_update_property_stringl(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("messageId"),
        message_id,
        message_id_len
    );

    zend_update_property_long(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("timestamp"), timestamp);

    zend_update_property_stringl(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("type"), type, type_len);
    zend_update_property_stringl(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("userId"), user_id, user_id_len);
    zend_update_property_stringl(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("appId"), app_id, app_id_len);
    zend_update_property_stringl(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("clusterId"),
        cluster_id,
        cluster_id_len
    );
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getContentType() */
static PHP_METHOD(AMQPBasicProperties, getContentType)
{
    zval rv;
    PHP_AMQP_NOPARAMS()
    PHP_AMQP_RETURN_THIS_PROP("contentType");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getContentEncoding() */
static PHP_METHOD(AMQPBasicProperties, getContentEncoding)
{
    zval rv;
    PHP_AMQP_NOPARAMS()
    PHP_AMQP_RETURN_THIS_PROP("contentEncoding");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getHeaders() */
static PHP_METHOD(AMQPBasicProperties, getHeaders)
{
    zval rv;
    PHP_AMQP_NOPARAMS()
    PHP_AMQP_RETURN_THIS_PROP("headers");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getDeliveryMode() */
static PHP_METHOD(AMQPBasicProperties, getDeliveryMode)
{
    zval rv;
    PHP_AMQP_NOPARAMS()
    PHP_AMQP_RETURN_THIS_PROP("deliveryMode");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getPriority() */
static PHP_METHOD(AMQPBasicProperties, getPriority)
{
    zval rv;
    PHP_AMQP_NOPARAMS()
    PHP_AMQP_RETURN_THIS_PROP("priority");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getCorrelationId() */
static PHP_METHOD(AMQPBasicProperties, getCorrelationId)
{
    zval rv;
    PHP_AMQP_NOPARAMS()
    PHP_AMQP_RETURN_THIS_PROP("correlationId");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getReplyTo() */
static PHP_METHOD(AMQPBasicProperties, getReplyTo)
{
    zval rv;
    PHP_AMQP_NOPARAMS()
    PHP_AMQP_RETURN_THIS_PROP("replyTo");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getExpiration()
check amqp envelope */
static PHP_METHOD(AMQPBasicProperties, getExpiration)
{
    zval rv;
    PHP_AMQP_NOPARAMS()
    PHP_AMQP_RETURN_THIS_PROP("expiration");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getMessageId() */
static PHP_METHOD(AMQPBasicProperties, getMessageId)
{
    zval rv;
    PHP_AMQP_NOPARAMS()
    PHP_AMQP_RETURN_THIS_PROP("messageId");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getTimestamp() */
static PHP_METHOD(AMQPBasicProperties, getTimestamp)
{
    zval rv;
    PHP_AMQP_NOPARAMS()
    PHP_AMQP_RETURN_THIS_PROP("timestamp");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getType() */
static PHP_METHOD(AMQPBasicProperties, getType)
{
    zval rv;
    PHP_AMQP_NOPARAMS()
    PHP_AMQP_RETURN_THIS_PROP("type");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getUserId() */
static PHP_METHOD(AMQPBasicProperties, getUserId)
{
    zval rv;
    PHP_AMQP_NOPARAMS()
    PHP_AMQP_RETURN_THIS_PROP("userId");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getAppId() */
static PHP_METHOD(AMQPBasicProperties, getAppId)
{
    zval rv;
    PHP_AMQP_NOPARAMS()
    PHP_AMQP_RETURN_THIS_PROP("appId");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getClusterId() */
static PHP_METHOD(AMQPBasicProperties, getClusterId)
{
    zval rv;
    PHP_AMQP_NOPARAMS()
    PHP_AMQP_RETURN_THIS_PROP("clusterId");
}
/* }}} */

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_basic_properties_class__construct, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, contentType, IS_STRING, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, contentEncoding, IS_STRING, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, headers, IS_ARRAY, 0, "[]")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, deliveryMode, IS_LONG, 0, "AMQP_DELIVERY_MODE_TRANSIENT")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, priority, IS_LONG, 0, "0")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, correlationId, IS_STRING, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, replyTo, IS_STRING, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, expiration, IS_STRING, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, messageId, IS_STRING, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timestamp, IS_LONG, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, type, IS_STRING, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, userId, IS_STRING, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, appId, IS_STRING, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, clusterId, IS_STRING, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(
    arginfo_amqp_basic_properties_class_getContentType,
    ZEND_SEND_BY_VAL,
    0,
    IS_STRING,
    1
)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(
    arginfo_amqp_basic_properties_class_getContentEncoding,
    ZEND_SEND_BY_VAL,
    0,
    IS_STRING,
    1
)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(
    arginfo_amqp_basic_properties_class_getHeaders,
    ZEND_SEND_BY_VAL,
    0,
    IS_ARRAY,
    0
)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(
    arginfo_amqp_basic_properties_class_getDeliveryMode,
    ZEND_SEND_BY_VAL,
    0,
    IS_LONG,
    0
)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(
    arginfo_amqp_basic_properties_class_getPriority,
    ZEND_SEND_BY_VAL,
    0,
    IS_LONG,
    0
)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(
    arginfo_amqp_basic_properties_class_getCorrelationId,
    ZEND_SEND_BY_VAL,
    0,
    IS_STRING,
    1
)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(
    arginfo_amqp_basic_properties_class_getReplyTo,
    ZEND_SEND_BY_VAL,
    0,
    IS_STRING,
    1
)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(
    arginfo_amqp_basic_properties_class_getExpiration,
    ZEND_SEND_BY_VAL,
    0,
    IS_STRING,
    1
)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(
    arginfo_amqp_basic_properties_class_getMessageId,
    ZEND_SEND_BY_VAL,
    0,
    IS_STRING,
    1
)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(
    arginfo_amqp_basic_properties_class_getTimestamp,
    ZEND_SEND_BY_VAL,
    0,
    IS_LONG,
    1
)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_basic_properties_class_getType, ZEND_SEND_BY_VAL, 0, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(
    arginfo_amqp_basic_properties_class_getUserId,
    ZEND_SEND_BY_VAL,
    0,
    IS_STRING,
    1
)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_basic_properties_class_getAppId, ZEND_SEND_BY_VAL, 0, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(
    arginfo_amqp_basic_properties_class_getClusterId,
    ZEND_SEND_BY_VAL,
    0,
    IS_STRING,
    1
)
ZEND_END_ARG_INFO()


zend_function_entry amqp_basic_properties_class_functions[] = {
        PHP_ME(AMQPBasicProperties, __construct, arginfo_amqp_basic_properties_class__construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)

        PHP_ME(AMQPBasicProperties, getContentType, arginfo_amqp_basic_properties_class_getContentType, ZEND_ACC_PUBLIC)
        PHP_ME(AMQPBasicProperties, getContentEncoding, arginfo_amqp_basic_properties_class_getContentEncoding, ZEND_ACC_PUBLIC)

        PHP_ME(AMQPBasicProperties, getHeaders, arginfo_amqp_basic_properties_class_getHeaders, ZEND_ACC_PUBLIC)

        PHP_ME(AMQPBasicProperties, getDeliveryMode, arginfo_amqp_basic_properties_class_getDeliveryMode, ZEND_ACC_PUBLIC)
        PHP_ME(AMQPBasicProperties, getPriority, arginfo_amqp_basic_properties_class_getPriority, ZEND_ACC_PUBLIC)

        PHP_ME(AMQPBasicProperties, getCorrelationId, arginfo_amqp_basic_properties_class_getCorrelationId, ZEND_ACC_PUBLIC)
        PHP_ME(AMQPBasicProperties, getReplyTo, arginfo_amqp_basic_properties_class_getReplyTo, ZEND_ACC_PUBLIC)
        PHP_ME(AMQPBasicProperties, getExpiration, arginfo_amqp_basic_properties_class_getExpiration, ZEND_ACC_PUBLIC)
        PHP_ME(AMQPBasicProperties, getMessageId, arginfo_amqp_basic_properties_class_getMessageId, ZEND_ACC_PUBLIC)

        PHP_ME(AMQPBasicProperties, getTimestamp, arginfo_amqp_basic_properties_class_getTimestamp, ZEND_ACC_PUBLIC)

        PHP_ME(AMQPBasicProperties, getType, arginfo_amqp_basic_properties_class_getType, ZEND_ACC_PUBLIC)
        PHP_ME(AMQPBasicProperties, getUserId, arginfo_amqp_basic_properties_class_getUserId, ZEND_ACC_PUBLIC)
        PHP_ME(AMQPBasicProperties, getAppId, arginfo_amqp_basic_properties_class_getAppId, ZEND_ACC_PUBLIC)
        PHP_ME(AMQPBasicProperties, getClusterId, arginfo_amqp_basic_properties_class_getClusterId, ZEND_ACC_PUBLIC)

    {NULL, NULL, NULL}
};

#define PHP_AMQP_ZVAL_AMQP_DELIVERY_NONPERSISTENT(v) ZVAL_LONG(v, AMQP_DELIVERY_NONPERSISTENT)

PHP_MINIT_FUNCTION(amqp_basic_properties)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "AMQPBasicProperties", amqp_basic_properties_class_functions);
    this_ce = zend_register_internal_class(&ce);

    PHP_AMQP_DECLARE_TYPED_PROPERTY(this_ce, "contentType", ZEND_ACC_PRIVATE, IS_STRING, 1);
    PHP_AMQP_DECLARE_TYPED_PROPERTY(this_ce, "contentEncoding", ZEND_ACC_PRIVATE, IS_STRING, 1);

#if PHP_VERSION_ID >= 80000
    PHP_AMQP_DECLARE_TYPED_PROPERTY_WITH_DEFAULT(this_ce, "headers", ZEND_ACC_PRIVATE, IS_ARRAY, 0, ZVAL_EMPTY_ARRAY);
#else
    PHP_AMQP_DECLARE_TYPED_PROPERTY_WITH_DEFAULT(this_ce, "headers", ZEND_ACC_PRIVATE, IS_ARRAY, 0, ZVAL_NULL);
#endif

    PHP_AMQP_DECLARE_TYPED_PROPERTY_WITH_DEFAULT(
        this_ce,
        "deliveryMode",
        ZEND_ACC_PRIVATE,
        IS_LONG,
        0,
        PHP_AMQP_ZVAL_AMQP_DELIVERY_NONPERSISTENT
    );
    zval default_priority;
    ZVAL_LONG(&default_priority, 0);
    PHP_AMQP_DECLARE_TYPED_PROPERTY_ZVAL(
        this_ce,
        "priority",
        ZEND_ACC_PRIVATE,
        PHP_AMQP_DECLARE_PROPERTY_TYPE(IS_LONG, 0),
        default_priority
    );

    PHP_AMQP_DECLARE_TYPED_PROPERTY(this_ce, "correlationId", ZEND_ACC_PRIVATE, IS_STRING, 1);
    PHP_AMQP_DECLARE_TYPED_PROPERTY(this_ce, "replyTo", ZEND_ACC_PRIVATE, IS_STRING, 1);
    PHP_AMQP_DECLARE_TYPED_PROPERTY(this_ce, "expiration", ZEND_ACC_PRIVATE, IS_STRING, 1);
    PHP_AMQP_DECLARE_TYPED_PROPERTY(this_ce, "messageId", ZEND_ACC_PRIVATE, IS_STRING, 1);

    PHP_AMQP_DECLARE_TYPED_PROPERTY(this_ce, "timestamp", ZEND_ACC_PRIVATE, IS_LONG, 1);

    PHP_AMQP_DECLARE_TYPED_PROPERTY(this_ce, "type", ZEND_ACC_PRIVATE, IS_STRING, 1);
    PHP_AMQP_DECLARE_TYPED_PROPERTY(this_ce, "userId", ZEND_ACC_PRIVATE, IS_STRING, 1);

    PHP_AMQP_DECLARE_TYPED_PROPERTY(this_ce, "appId", ZEND_ACC_PRIVATE, IS_STRING, 1);
    PHP_AMQP_DECLARE_TYPED_PROPERTY(this_ce, "clusterId", ZEND_ACC_PRIVATE, IS_STRING, 1);

    return SUCCESS;
}

zend_bool parse_amqp_value(amqp_field_value_t *value, zval *result)
{
    switch (value->kind) {
        case AMQP_FIELD_KIND_BOOLEAN:
            ZVAL_BOOL(result, value->value.boolean);
            break;

        case AMQP_FIELD_KIND_I8:
            ZVAL_LONG(result, value->value.i8);
            break;

        case AMQP_FIELD_KIND_U8:
            ZVAL_LONG(result, value->value.u8);
            break;

        case AMQP_FIELD_KIND_I16:
            ZVAL_LONG(result, value->value.i16);
            break;

        case AMQP_FIELD_KIND_U16:
            ZVAL_LONG(result, value->value.u16);
            break;

        case AMQP_FIELD_KIND_I32:
            ZVAL_LONG(result, value->value.i32);
            break;

        case AMQP_FIELD_KIND_U32:
            ZVAL_LONG(result, value->value.u32);
            break;

        case AMQP_FIELD_KIND_I64:
            ZVAL_LONG(result, value->value.i64);
            break;

        case AMQP_FIELD_KIND_U64:
            if (value->value.u64 > LONG_MAX) {
                ZVAL_DOUBLE(result, value->value.u64);
            } else {
                ZVAL_LONG(result, value->value.u64);
            }
            break;

        case AMQP_FIELD_KIND_F32:
            ZVAL_DOUBLE(result, value->value.f32);
            break;

        case AMQP_FIELD_KIND_F64:
            ZVAL_DOUBLE(result, value->value.f64);
            break;

        case AMQP_FIELD_KIND_UTF8:
        case AMQP_FIELD_KIND_BYTES:
            ZVAL_STRINGL(result, value->value.bytes.bytes, value->value.bytes.len);
            break;

        case AMQP_FIELD_KIND_VOID:
            ZVAL_NULL(result);
            break;

        case AMQP_FIELD_KIND_ARRAY:
            array_init(result);
            parse_amqp_array(&(value->value.array), result);
            break;

        case AMQP_FIELD_KIND_TABLE:
            array_init(result);
            parse_amqp_table(&(value->value.table), result);
            break;

        case AMQP_FIELD_KIND_TIMESTAMP: {
            zval timestamp;
            ZVAL_UNDEF(&timestamp);

            ZVAL_DOUBLE(&timestamp, value->value.u64);
            object_init_ex(result, amqp_timestamp_class_entry);

            zend_call_method_with_1_params(
                PHP_AMQP_COMPAT_OBJ_P(result),
                amqp_timestamp_class_entry,
                NULL,
                "__construct",
                NULL,
                &timestamp
            );
            break;
        }

        case AMQP_FIELD_KIND_DECIMAL: {
            zval e;
            zval n;
            ZVAL_UNDEF(&e);
            ZVAL_UNDEF(&n);

            ZVAL_LONG(&e, value->value.decimal.decimals);
            ZVAL_LONG(&n, value->value.decimal.value);

            object_init_ex(result, amqp_decimal_class_entry);

            zend_call_method_with_2_params(
                PHP_AMQP_COMPAT_OBJ_P(result),
                amqp_decimal_class_entry,
                NULL,
                "__construct",
                NULL,
                &e,
                &n
            );

            zval_ptr_dtor(&e);
            zval_ptr_dtor(&n);
            break;
        }

        default:
            return 0;
    }

    return 1;
}

void parse_amqp_array(amqp_array_t *array, zval *result)
{
    assert(Z_TYPE_P(result) == IS_ARRAY);

    int j;
    for (j = 0; j < array->num_entries; ++j) {
        zval result_nested;
        ZVAL_UNDEF(&result_nested);
        if (parse_amqp_value(&(array->entries[j]), &result_nested)) {
            add_next_index_zval(result, &result_nested);
        }
    }
}

void parse_amqp_table(amqp_table_t *table, zval *result)
{
    int i;
    zval result_nested;

    assert(Z_TYPE_P(result) == IS_ARRAY);

    for (i = 0; i < table->num_entries; i++) {
        amqp_table_entry_t *entry = &(table->entries[i]);
        ZVAL_UNDEF(&result_nested);
        if (parse_amqp_value(&(entry->value), &result_nested)) {
            char *key = estrndup(entry->key.bytes, (unsigned) entry->key.len);
            add_assoc_zval(result, key, &result_nested);
            efree(key);
        } else {
            zval_ptr_dtor(&result_nested);
        }
    }
}

void php_amqp_basic_properties_extract(amqp_basic_properties_t *p, zval *obj)
{
    zval headers;

    ZVAL_UNDEF(&headers);
    array_init(&headers);

    if (p->_flags & AMQP_BASIC_CONTENT_TYPE_FLAG) {
        zend_update_property_stringl(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(obj),
            ZEND_STRL("contentType"),
            (const char *) p->content_type.bytes,
            (size_t) p->content_type.len
        );
    } else {
        /* BC */
        zend_update_property_null(this_ce, PHP_AMQP_COMPAT_OBJ_P(obj), ZEND_STRL("contentType"));
    }

    if (p->_flags & AMQP_BASIC_CONTENT_ENCODING_FLAG) {
        zend_update_property_stringl(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(obj),
            ZEND_STRL("contentEncoding"),
            (const char *) p->content_encoding.bytes,
            (size_t) p->content_encoding.len
        );
    } else {
        /* BC */
        zend_update_property_null(this_ce, PHP_AMQP_COMPAT_OBJ_P(obj), ZEND_STRL("contentEncoding"));
    }

    if (p->_flags & AMQP_BASIC_HEADERS_FLAG) {
        parse_amqp_table(&(p->headers), &headers);
    }

    zend_update_property(this_ce, PHP_AMQP_COMPAT_OBJ_P(obj), ZEND_STRL("headers"), &headers);

    if (p->_flags & AMQP_BASIC_DELIVERY_MODE_FLAG) {
        zend_update_property_long(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(obj),
            ZEND_STRL("deliveryMode"),
            (zend_long) p->delivery_mode
        );
    } else {
        /* BC */
        zend_update_property_long(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(obj),
            ZEND_STRL("deliveryMode"),
            AMQP_DELIVERY_NONPERSISTENT
        );
    }

    if (p->_flags & AMQP_BASIC_PRIORITY_FLAG) {
        zend_update_property_long(this_ce, PHP_AMQP_COMPAT_OBJ_P(obj), ZEND_STRL("priority"), (zend_long) p->priority);
    } else {
        /* BC */
        zend_update_property_long(this_ce, PHP_AMQP_COMPAT_OBJ_P(obj), ZEND_STRL("priority"), 0);
    }

    if (p->_flags & AMQP_BASIC_CORRELATION_ID_FLAG) {
        zend_update_property_stringl(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(obj),
            ZEND_STRL("correlationId"),
            (const char *) p->correlation_id.bytes,
            (size_t) p->correlation_id.len
        );
    } else {
        /* BC */
        zend_update_property_null(this_ce, PHP_AMQP_COMPAT_OBJ_P(obj), ZEND_STRL("correlationId"));
    }

    if (p->_flags & AMQP_BASIC_REPLY_TO_FLAG) {
        zend_update_property_stringl(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(obj),
            ZEND_STRL("replyTo"),
            (const char *) p->reply_to.bytes,
            (size_t) p->reply_to.len
        );
    } else {
        /* BC */
        zend_update_property_null(this_ce, PHP_AMQP_COMPAT_OBJ_P(obj), ZEND_STRL("replyTo"));
    }

    if (p->_flags & AMQP_BASIC_EXPIRATION_FLAG) {
        zend_update_property_stringl(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(obj),
            ZEND_STRL("expiration"),
            (const char *) p->expiration.bytes,
            (size_t) p->expiration.len
        );
    } else {
        /* BC */
        zend_update_property_null(this_ce, PHP_AMQP_COMPAT_OBJ_P(obj), ZEND_STRL("expiration"));
    }

    if (p->_flags & AMQP_BASIC_MESSAGE_ID_FLAG) {
        zend_update_property_stringl(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(obj),
            ZEND_STRL("messageId"),
            (const char *) p->message_id.bytes,
            (size_t) p->message_id.len
        );
    } else {
        /* BC */
        zend_update_property_null(this_ce, PHP_AMQP_COMPAT_OBJ_P(obj), ZEND_STRL("messageId"));
    }

    if (p->_flags & AMQP_BASIC_TIMESTAMP_FLAG) {
        zend_update_property_long(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(obj),
            ZEND_STRL("timestamp"),
            (zend_long) p->timestamp
        );
    } else {
        /* BC */
        zend_update_property_long(this_ce, PHP_AMQP_COMPAT_OBJ_P(obj), ZEND_STRL("timestamp"), 0);
    }

    if (p->_flags & AMQP_BASIC_TYPE_FLAG) {
        zend_update_property_stringl(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(obj),
            ZEND_STRL("type"),
            (const char *) p->type.bytes,
            (size_t) p->type.len
        );
    } else {
        /* BC */
        zend_update_property_null(this_ce, PHP_AMQP_COMPAT_OBJ_P(obj), ZEND_STRL("type"));
    }

    if (p->_flags & AMQP_BASIC_USER_ID_FLAG) {
        zend_update_property_stringl(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(obj),
            ZEND_STRL("userId"),
            (const char *) p->user_id.bytes,
            (size_t) p->user_id.len
        );
    } else {
        /* BC */
        zend_update_property_null(this_ce, PHP_AMQP_COMPAT_OBJ_P(obj), ZEND_STRL("userId"));
    }

    if (p->_flags & AMQP_BASIC_APP_ID_FLAG) {
        zend_update_property_stringl(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(obj),
            ZEND_STRL("appId"),
            (const char *) p->app_id.bytes,
            (size_t) p->app_id.len
        );
    } else {
        /* BC */
        zend_update_property_null(this_ce, PHP_AMQP_COMPAT_OBJ_P(obj), ZEND_STRL("appId"));
    }

    zval_ptr_dtor(&headers);
}
