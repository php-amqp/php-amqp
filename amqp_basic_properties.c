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
        #include "main/php_stdint.h"
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

zend_class_entry *amqp_basic_properties_class_entry;
#define this_ce amqp_basic_properties_class_entry


void php_amqp_basic_properties_convert_to_zval(amqp_basic_properties_t *props, zval *obj TSRMLS_DC)
{
    object_init_ex(obj, this_ce);

    php_amqp_basic_properties_extract(props, obj TSRMLS_CC);
}

void php_amqp_basic_properties_set_empty_headers(zval *obj TSRMLS_DC)
{
    zval headers;

    ZVAL_UNDEF(&headers);
    array_init(&headers);

    zend_update_property(this_ce, PHP_AMQP_COMPAT_OBJ_P(obj), ZEND_STRL("headers"), &headers TSRMLS_CC);

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
            ZEND_NUM_ARGS() TSRMLS_CC,
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
        ZEND_STRL("content_type"),
        content_type,
        content_type_len TSRMLS_CC
    );
    zend_update_property_stringl(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("content_encoding"),
        content_encoding,
        content_encoding_len TSRMLS_CC
    );

    if (headers != NULL) {
        zend_update_property(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("headers"), headers TSRMLS_CC);
    } else {
        php_amqp_basic_properties_set_empty_headers(getThis() TSRMLS_CC);
    }

    zend_update_property_long(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("delivery_mode"),
        delivery_mode TSRMLS_CC
    );
    zend_update_property_long(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("priority"), priority TSRMLS_CC);

    zend_update_property_stringl(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("correlation_id"),
        correlation_id,
        correlation_id_len TSRMLS_CC
    );
    zend_update_property_stringl(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("reply_to"),
        reply_to,
        reply_to_len TSRMLS_CC
    );
    zend_update_property_stringl(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("expiration"),
        expiration,
        expiration_len TSRMLS_CC
    );
    zend_update_property_stringl(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("message_id"),
        message_id,
        message_id_len TSRMLS_CC
    );

    zend_update_property_long(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("timestamp"), timestamp TSRMLS_CC);

    zend_update_property_stringl(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("type"), type, type_len TSRMLS_CC);
    zend_update_property_stringl(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("user_id"),
        user_id,
        user_id_len TSRMLS_CC
    );
    zend_update_property_stringl(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("app_id"),
        app_id,
        app_id_len TSRMLS_CC
    );
    zend_update_property_stringl(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("cluster_id"),
        cluster_id,
        cluster_id_len TSRMLS_CC
    );
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getContentType() */
static PHP_METHOD(AMQPBasicProperties, getContentType)
{
    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("content_type");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getContentEncoding() */
static PHP_METHOD(AMQPBasicProperties, getContentEncoding)
{
    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("content_encoding");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getCorrelationId() */
static PHP_METHOD(AMQPBasicProperties, getHeaders)
{
    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("headers");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getDeliveryMode() */
static PHP_METHOD(AMQPBasicProperties, getDeliveryMode)
{
    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("delivery_mode");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getPriority() */
static PHP_METHOD(AMQPBasicProperties, getPriority)
{
    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("priority");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getCorrelationId() */
static PHP_METHOD(AMQPBasicProperties, getCorrelationId)
{
    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("correlation_id");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getReplyTo() */
static PHP_METHOD(AMQPBasicProperties, getReplyTo)
{
    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("reply_to");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getExpiration()
check amqp envelope */
static PHP_METHOD(AMQPBasicProperties, getExpiration)
{
    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("expiration");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getMessageId() */
static PHP_METHOD(AMQPBasicProperties, getMessageId)
{
    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("message_id");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getTimestamp() */
static PHP_METHOD(AMQPBasicProperties, getTimestamp)
{
    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("timestamp");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getType() */
static PHP_METHOD(AMQPBasicProperties, getType)
{
    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("type");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getUserId() */
static PHP_METHOD(AMQPBasicProperties, getUserId)
{
    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("user_id");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getAppId() */
static PHP_METHOD(AMQPBasicProperties, getAppId)
{
    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("app_id");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getClusterId() */
static PHP_METHOD(AMQPBasicProperties, getClusterId)
{
    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("cluster_id");
}
/* }}} */

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_basic_properties_class__construct, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
    ZEND_ARG_INFO(0, contentType)
    ZEND_ARG_INFO(0, contentEncoding)
    ZEND_ARG_ARRAY_INFO(0, headers, 0)
    ZEND_ARG_INFO(0, deliveryMode)
    ZEND_ARG_INFO(0, priority)
    ZEND_ARG_INFO(0, correlationId)
    ZEND_ARG_INFO(0, replyTo)
    ZEND_ARG_INFO(0, expiration)
    ZEND_ARG_INFO(0, messageId)
    ZEND_ARG_INFO(0, timestamp)
    ZEND_ARG_INFO(0, type)
    ZEND_ARG_INFO(0, userId)
    ZEND_ARG_INFO(0, appId)
    ZEND_ARG_INFO(0, clusterId)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_basic_properties_class_getContentType, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_basic_properties_class_getContentEncoding, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_basic_properties_class_getHeaders, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_basic_properties_class_getDeliveryMode, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_basic_properties_class_getPriority, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_basic_properties_class_getCorrelationId, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_basic_properties_class_getReplyTo, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_basic_properties_class_getExpiration, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_basic_properties_class_getMessageId, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_basic_properties_class_getTimestamp, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_basic_properties_class_getType, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_basic_properties_class_getUserId, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_basic_properties_class_getAppId, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_basic_properties_class_getClusterId, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
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


PHP_MINIT_FUNCTION(amqp_basic_properties)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "AMQPBasicProperties", amqp_basic_properties_class_functions);
    this_ce = zend_register_internal_class(&ce TSRMLS_CC);

    zend_declare_property_stringl(this_ce, ZEND_STRL("content_type"), "", 0, ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_stringl(this_ce, ZEND_STRL("content_encoding"), "", 0, ZEND_ACC_PRIVATE TSRMLS_CC);

    zend_declare_property_null(this_ce, ZEND_STRL("headers"), ZEND_ACC_PRIVATE TSRMLS_CC);

    zend_declare_property_long(
        this_ce,
        ZEND_STRL("delivery_mode"),
        AMQP_DELIVERY_NONPERSISTENT,
        ZEND_ACC_PRIVATE TSRMLS_CC
    );
    zend_declare_property_long(this_ce, ZEND_STRL("priority"), 0, ZEND_ACC_PRIVATE TSRMLS_CC);

    zend_declare_property_stringl(this_ce, ZEND_STRL("correlation_id"), "", 0, ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_stringl(this_ce, ZEND_STRL("reply_to"), "", 0, ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_stringl(this_ce, ZEND_STRL("expiration"), "", 0, ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_stringl(this_ce, ZEND_STRL("message_id"), "", 0, ZEND_ACC_PRIVATE TSRMLS_CC);

    zend_declare_property_long(this_ce, ZEND_STRL("timestamp"), 0, ZEND_ACC_PRIVATE TSRMLS_CC);

    zend_declare_property_stringl(this_ce, ZEND_STRL("type"), "", 0, ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_stringl(this_ce, ZEND_STRL("user_id"), "", 0, ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_stringl(this_ce, ZEND_STRL("app_id"), "", 0, ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_stringl(this_ce, ZEND_STRL("cluster_id"), "", 0, ZEND_ACC_PRIVATE TSRMLS_CC);

    return SUCCESS;
}


void parse_amqp_table(amqp_table_t *table, zval *result TSRMLS_DC)
{
    int i;
    zend_bool has_value = 0;

    zval value;

    assert(Z_TYPE_P(result) == IS_ARRAY);

    for (i = 0; i < table->num_entries; i++) {
        ZVAL_UNDEF(&value);
        has_value = 1;

        amqp_table_entry_t *entry = &(table->entries[i]);
        switch (entry->value.kind) {
            case AMQP_FIELD_KIND_BOOLEAN:
                ZVAL_BOOL(&value, entry->value.value.boolean);
                break;
            case AMQP_FIELD_KIND_I8:
                ZVAL_LONG(&value, entry->value.value.i8);
                break;
            case AMQP_FIELD_KIND_U8:
                ZVAL_LONG(&value, entry->value.value.u8);
                break;
            case AMQP_FIELD_KIND_I16:
                ZVAL_LONG(&value, entry->value.value.i16);
                break;
            case AMQP_FIELD_KIND_U16:
                ZVAL_LONG(&value, entry->value.value.u16);
                break;
            case AMQP_FIELD_KIND_I32:
                ZVAL_LONG(&value, entry->value.value.i32);
                break;
            case AMQP_FIELD_KIND_U32:
                ZVAL_LONG(&value, entry->value.value.u32);
                break;
            case AMQP_FIELD_KIND_I64:
                ZVAL_LONG(&value, entry->value.value.i64);
                break;
            case AMQP_FIELD_KIND_U64:
                if (entry->value.value.u64 > LONG_MAX) {
                    ZVAL_DOUBLE(&value, entry->value.value.u64);
                } else {
                    ZVAL_LONG(&value, entry->value.value.u64);
                }
                break;
            case AMQP_FIELD_KIND_F32:
                ZVAL_DOUBLE(&value, entry->value.value.f32);
                break;
            case AMQP_FIELD_KIND_F64:
                ZVAL_DOUBLE(&value, entry->value.value.f64);
                break;
            case AMQP_FIELD_KIND_UTF8:
            case AMQP_FIELD_KIND_BYTES:
                ZVAL_STRINGL(&value, entry->value.value.bytes.bytes, entry->value.value.bytes.len);
                break;
            case AMQP_FIELD_KIND_ARRAY: {
                int j;
                array_init(&value);
                for (j = 0; j < entry->value.value.array.num_entries; ++j) {
                    switch (entry->value.value.array.entries[j].kind) {
                        case AMQP_FIELD_KIND_UTF8:
                            add_next_index_stringl(
                                &value,
                                entry->value.value.array.entries[j].value.bytes.bytes,
                                entry->value.value.array.entries[j].value.bytes.len
                            );
                            break;
                        case AMQP_FIELD_KIND_TABLE: {
                            zval subtable;
                            ZVAL_UNDEF(&subtable);
                            array_init(&subtable);

                            parse_amqp_table(&(entry->value.value.array.entries[j].value.table), &subtable TSRMLS_CC);
                            add_next_index_zval(&value, &subtable);
                        } break;
                        default:
                            break;
                    }
                }
            } break;
            case AMQP_FIELD_KIND_TABLE:
                array_init(&value);
                parse_amqp_table(&(entry->value.value.table), &value TSRMLS_CC);
                break;

            case AMQP_FIELD_KIND_TIMESTAMP: {
                char timestamp_str[20];
                zval timestamp;
                ZVAL_UNDEF(&timestamp);

                int length = snprintf(timestamp_str, sizeof(timestamp_str), ZEND_ULONG_FMT, entry->value.value.u64);
                ZVAL_STRINGL(&timestamp, (char *) timestamp_str, length);
                object_init_ex(&value, amqp_timestamp_class_entry);

                zend_call_method_with_1_params(
                    PHP_AMQP_COMPAT_OBJ_P(&value),
                    amqp_timestamp_class_entry,
                    NULL,
                    "__construct",
                    NULL,
                    &timestamp
                );

                zval_ptr_dtor(&timestamp);
                break;
            }

            case AMQP_FIELD_KIND_VOID:
                ZVAL_NULL(&value);
                break;
            case AMQP_FIELD_KIND_DECIMAL: {

                zval e;
                zval n;
                ZVAL_UNDEF(&e);
                ZVAL_UNDEF(&n);

                ZVAL_LONG(&e, entry->value.value.decimal.decimals);
                ZVAL_LONG(&n, entry->value.value.decimal.value);

                object_init_ex(&value, amqp_decimal_class_entry);

                zend_call_method_with_2_params(
                    PHP_AMQP_COMPAT_OBJ_P(&value),
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
                has_value = 0;
                break;
        }

        if (has_value) {
            char *key = estrndup(entry->key.bytes, (unsigned) entry->key.len);
            add_assoc_zval(result, key, &value);
            efree(key);
        } else {
            if (!Z_ISUNDEF(value)) {
                zval_ptr_dtor(&value);
            }
        }
    }
    return;
}

void php_amqp_basic_properties_extract(amqp_basic_properties_t *p, zval *obj TSRMLS_DC)
{
    zval headers;

    ZVAL_UNDEF(&headers);
    array_init(&headers);

    if (p->_flags & AMQP_BASIC_CONTENT_TYPE_FLAG) {
        zend_update_property_stringl(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(obj),
            ZEND_STRL("content_type"),
            (const char *) p->content_type.bytes,
            (size_t) p->content_type.len TSRMLS_CC
        );
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, PHP_AMQP_COMPAT_OBJ_P(obj), ZEND_STRL("content_type"), "", 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_CONTENT_ENCODING_FLAG) {
        zend_update_property_stringl(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(obj),
            ZEND_STRL("content_encoding"),
            (const char *) p->content_encoding.bytes,
            (size_t) p->content_encoding.len TSRMLS_CC
        );
    } else {
        /* BC */
        zend_update_property_stringl(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(obj),
            ZEND_STRL("content_encoding"),
            "",
            0 TSRMLS_CC
        );
    }

    if (p->_flags & AMQP_BASIC_HEADERS_FLAG) {
        parse_amqp_table(&(p->headers), &headers TSRMLS_CC);
    }

    zend_update_property(this_ce, PHP_AMQP_COMPAT_OBJ_P(obj), ZEND_STRL("headers"), &headers TSRMLS_CC);

    if (p->_flags & AMQP_BASIC_DELIVERY_MODE_FLAG) {
        zend_update_property_long(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(obj),
            ZEND_STRL("delivery_mode"),
            (zend_long) p->delivery_mode TSRMLS_CC
        );
    } else {
        /* BC */
        zend_update_property_long(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(obj),
            ZEND_STRL("delivery_mode"),
            AMQP_DELIVERY_NONPERSISTENT TSRMLS_CC
        );
    }

    if (p->_flags & AMQP_BASIC_PRIORITY_FLAG) {
        zend_update_property_long(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(obj),
            ZEND_STRL("priority"),
            (zend_long) p->priority TSRMLS_CC
        );
    } else {
        /* BC */
        zend_update_property_long(this_ce, PHP_AMQP_COMPAT_OBJ_P(obj), ZEND_STRL("priority"), 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_CORRELATION_ID_FLAG) {
        zend_update_property_stringl(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(obj),
            ZEND_STRL("correlation_id"),
            (const char *) p->correlation_id.bytes,
            (size_t) p->correlation_id.len TSRMLS_CC
        );
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, PHP_AMQP_COMPAT_OBJ_P(obj), ZEND_STRL("correlation_id"), "", 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_REPLY_TO_FLAG) {
        zend_update_property_stringl(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(obj),
            ZEND_STRL("reply_to"),
            (const char *) p->reply_to.bytes,
            (size_t) p->reply_to.len TSRMLS_CC
        );
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, PHP_AMQP_COMPAT_OBJ_P(obj), ZEND_STRL("reply_to"), "", 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_EXPIRATION_FLAG) {
        zend_update_property_stringl(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(obj),
            ZEND_STRL("expiration"),
            (const char *) p->expiration.bytes,
            (size_t) p->expiration.len TSRMLS_CC
        );
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, PHP_AMQP_COMPAT_OBJ_P(obj), ZEND_STRL("expiration"), "", 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_MESSAGE_ID_FLAG) {
        zend_update_property_stringl(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(obj),
            ZEND_STRL("message_id"),
            (const char *) p->message_id.bytes,
            (size_t) p->message_id.len TSRMLS_CC
        );
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, PHP_AMQP_COMPAT_OBJ_P(obj), ZEND_STRL("message_id"), "", 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_TIMESTAMP_FLAG) {
        zend_update_property_long(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(obj),
            ZEND_STRL("timestamp"),
            (zend_long) p->timestamp TSRMLS_CC
        );
    } else {
        /* BC */
        zend_update_property_long(this_ce, PHP_AMQP_COMPAT_OBJ_P(obj), ZEND_STRL("timestamp"), 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_TYPE_FLAG) {
        zend_update_property_stringl(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(obj),
            ZEND_STRL("type"),
            (const char *) p->type.bytes,
            (size_t) p->type.len TSRMLS_CC
        );
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, PHP_AMQP_COMPAT_OBJ_P(obj), ZEND_STRL("type"), "", 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_USER_ID_FLAG) {
        zend_update_property_stringl(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(obj),
            ZEND_STRL("user_id"),
            (const char *) p->user_id.bytes,
            (size_t) p->user_id.len TSRMLS_CC
        );
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, PHP_AMQP_COMPAT_OBJ_P(obj), ZEND_STRL("user_id"), "", 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_APP_ID_FLAG) {
        zend_update_property_stringl(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(obj),
            ZEND_STRL("app_id"),
            (const char *) p->app_id.bytes,
            (size_t) p->app_id.len TSRMLS_CC
        );
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, PHP_AMQP_COMPAT_OBJ_P(obj), ZEND_STRL("app_id"), "", 0 TSRMLS_CC);
    }

    zval_ptr_dtor(&headers);
}
