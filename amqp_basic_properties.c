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

#include <amqp.h>
#include <amqp_framing.h>

#ifdef PHP_WIN32
# include "win32/unistd.h"
# include "win32/php_stdint.h"
# include "win32/signal.h"
#else
# include <signal.h>
# include <stdint.h>
# include <unistd.h>
#endif

#if HAVE_INTTYPES_H
# include <inttypes.h>
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

void php_amqp_basic_properties_set_empty_headers(zval *obj TSRMLS_DC) {
    PHP5to7_zval_t headers PHP5to7_MAYBE_SET_TO_NULL;

    PHP5to7_MAYBE_INIT(headers);
    PHP5to7_ARRAY_INIT(headers);

    zend_update_property(this_ce, PHP5to8_OBJ_PROP(obj), ZEND_STRL("headers"), PHP5to7_MAYBE_PTR(headers) TSRMLS_CC);

    PHP5to7_MAYBE_DESTROY(headers);
}


/* {{{ proto AMQPBasicProperties::__construct() */
static PHP_METHOD(AMQPBasicProperties, __construct) {

    char *content_type = NULL;      PHP5to7_param_str_len_type_t content_type_len = 0;
    char *content_encoding = NULL;  PHP5to7_param_str_len_type_t content_encoding_len = 0;

    zval *headers = NULL;

    PHP5to7_param_long_type_t delivery_mode = AMQP_DELIVERY_NONPERSISTENT;
    PHP5to7_param_long_type_t priority = 0;

    char *correlation_id = NULL;    PHP5to7_param_str_len_type_t correlation_id_len = 0;
    char *reply_to = NULL;          PHP5to7_param_str_len_type_t reply_to_len = 0;
    char *expiration = NULL;        PHP5to7_param_str_len_type_t expiration_len = 0;
    char *message_id = NULL;        PHP5to7_param_str_len_type_t message_id_len = 0;

    PHP5to7_param_long_type_t timestamp = 0;

    char *type = NULL;          PHP5to7_param_str_len_type_t type_len = 0;
    char *user_id = NULL;       PHP5to7_param_str_len_type_t user_id_len = 0;
    char *app_id = NULL;        PHP5to7_param_str_len_type_t app_id_len = 0;
    char *cluster_id = NULL;    PHP5to7_param_str_len_type_t cluster_id_len = 0;


    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|ssallsssslssss",
                              /* s */ &content_type, &content_type_len,
                              /* s */ &content_encoding, &content_encoding_len,
                              /* a */ &headers,
                              /* l */ &delivery_mode,
                              /* l */ &priority,
                              /* s */ &correlation_id, &correlation_id_len,
                              /* s */ &reply_to, &reply_to_len,
                              /* s */ &expiration, &expiration_len,
                              /* s */ &message_id, &message_id_len,
                              /* l */ &timestamp,
                              /* s */ &type, &type_len,
                              /* s */ &user_id, &user_id_len,
                              /* s */ &app_id, &app_id_len,
                              /* s */ &cluster_id, &cluster_id_len
                             ) == FAILURE) {
        return;
    }
    zend_update_property_stringl(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("content_type"), content_type, content_type_len TSRMLS_CC);
    zend_update_property_stringl(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("content_encoding"), content_encoding, content_encoding_len TSRMLS_CC);

    if (headers != NULL) {
        zend_update_property(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("headers"), headers TSRMLS_CC);
    } else {
        php_amqp_basic_properties_set_empty_headers(getThis() TSRMLS_CC);
    }

    zend_update_property_long(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("delivery_mode"), delivery_mode TSRMLS_CC);
    zend_update_property_long(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("priority"), priority TSRMLS_CC);

    zend_update_property_stringl(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("correlation_id"), correlation_id, correlation_id_len TSRMLS_CC);
    zend_update_property_stringl(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("reply_to"), reply_to, reply_to_len TSRMLS_CC);
    zend_update_property_stringl(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("expiration"), expiration, expiration_len TSRMLS_CC);
    zend_update_property_stringl(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("message_id"), message_id, message_id_len TSRMLS_CC);

    zend_update_property_long(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("timestamp"), timestamp TSRMLS_CC);

    zend_update_property_stringl(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("type"), type, type_len TSRMLS_CC);
    zend_update_property_stringl(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("user_id"), user_id, user_id_len TSRMLS_CC);
    zend_update_property_stringl(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("app_id"), app_id, app_id_len TSRMLS_CC);
    zend_update_property_stringl(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("cluster_id"), cluster_id, cluster_id_len TSRMLS_CC);
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getContentType() */
static PHP_METHOD(AMQPBasicProperties, getContentType) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("content_type");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getContentEncoding() */
static PHP_METHOD(AMQPBasicProperties, getContentEncoding) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("content_encoding");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getCorrelationId() */
static PHP_METHOD(AMQPBasicProperties, getHeaders) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("headers");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getDeliveryMode() */
static PHP_METHOD(AMQPBasicProperties, getDeliveryMode) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("delivery_mode");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getPriority() */
static PHP_METHOD(AMQPBasicProperties, getPriority) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("priority");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getCorrelationId() */
static PHP_METHOD(AMQPBasicProperties, getCorrelationId) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("correlation_id");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getReplyTo() */
static PHP_METHOD(AMQPBasicProperties, getReplyTo) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("reply_to");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getExpiration()
check amqp envelope */
static PHP_METHOD(AMQPBasicProperties, getExpiration) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("expiration");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getMessageId() */
static PHP_METHOD(AMQPBasicProperties, getMessageId) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("message_id");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getTimestamp() */
static PHP_METHOD(AMQPBasicProperties, getTimestamp) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("timestamp");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getType() */
static PHP_METHOD(AMQPBasicProperties, getType) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("type");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getUserId() */
static PHP_METHOD(AMQPBasicProperties, getUserId) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("user_id");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getAppId() */
static PHP_METHOD(AMQPBasicProperties, getAppId) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("app_id");
}
/* }}} */

/* {{{ proto AMQPBasicProperties::getClusterId() */
static PHP_METHOD(AMQPBasicProperties, getClusterId) {
    PHP5to7_READ_PROP_RV_PARAM_DECL;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("cluster_id");
}
/* }}} */

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_basic_properties_class__construct, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
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


PHP_MINIT_FUNCTION (amqp_basic_properties) {
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "AMQPBasicProperties", amqp_basic_properties_class_functions);
    this_ce = zend_register_internal_class(&ce TSRMLS_CC);

    zend_declare_property_stringl(this_ce, ZEND_STRL("content_type"), "", 0, ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_stringl(this_ce, ZEND_STRL("content_encoding"), "", 0, ZEND_ACC_PRIVATE TSRMLS_CC);

    zend_declare_property_null(this_ce, ZEND_STRL("headers"), ZEND_ACC_PRIVATE TSRMLS_CC);

    zend_declare_property_long(this_ce, ZEND_STRL("delivery_mode"), AMQP_DELIVERY_NONPERSISTENT, ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_long(this_ce, ZEND_STRL("priority"), 0, ZEND_ACC_PRIVATE TSRMLS_CC);

    zend_declare_property_stringl(this_ce, ZEND_STRL("correlation_id"), "", 0, ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_stringl(this_ce, ZEND_STRL("reply_to"), "", 0, ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_stringl(this_ce, ZEND_STRL("expiration"), "", 0, ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_stringl(this_ce, ZEND_STRL("message_id"), "", 0, ZEND_ACC_PRIVATE TSRMLS_CC);

    zend_declare_property_long(this_ce, ZEND_STRL("timestamp"), 0, ZEND_ACC_PRIVATE TSRMLS_CC);

    zend_declare_property_stringl(this_ce, ZEND_STRL("type"), "", 0, ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_stringl(this_ce, ZEND_STRL("user_id"), "", 0,  ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_stringl(this_ce, ZEND_STRL("app_id"), "", 0,  ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_stringl(this_ce, ZEND_STRL("cluster_id"), "", 0, ZEND_ACC_PRIVATE TSRMLS_CC);

    return SUCCESS;
}


void parse_amqp_table(amqp_table_t *table, zval *result TSRMLS_DC) {
    int i;
    zend_bool has_value = 0;

    PHP5to7_zval_t value PHP5to7_MAYBE_SET_TO_NULL;

    assert(Z_TYPE_P(result) == IS_ARRAY);

    for (i = 0; i < table->num_entries; i++) {
        PHP5to7_MAYBE_INIT(value);
        has_value = 1;

        amqp_table_entry_t *entry = &(table->entries[i]);
        switch (entry->value.kind) {
            case AMQP_FIELD_KIND_BOOLEAN:
                ZVAL_BOOL(PHP5to7_MAYBE_PTR(value), entry->value.value.boolean);
                break;
            case AMQP_FIELD_KIND_I8:
                ZVAL_LONG(PHP5to7_MAYBE_PTR(value), entry->value.value.i8);
                break;
            case AMQP_FIELD_KIND_U8:
                ZVAL_LONG(PHP5to7_MAYBE_PTR(value), entry->value.value.u8);
                break;
            case AMQP_FIELD_KIND_I16:
                ZVAL_LONG(PHP5to7_MAYBE_PTR(value), entry->value.value.i16);
                break;
            case AMQP_FIELD_KIND_U16:
                ZVAL_LONG(PHP5to7_MAYBE_PTR(value), entry->value.value.u16);
                break;
            case AMQP_FIELD_KIND_I32:
                ZVAL_LONG(PHP5to7_MAYBE_PTR(value), entry->value.value.i32);
                break;
            case AMQP_FIELD_KIND_U32:
                ZVAL_LONG(PHP5to7_MAYBE_PTR(value), entry->value.value.u32);
                break;
            case AMQP_FIELD_KIND_I64:
                ZVAL_LONG(PHP5to7_MAYBE_PTR(value), entry->value.value.i64);
                break;
            case AMQP_FIELD_KIND_U64:
                if (entry->value.value.u64 > LONG_MAX) {
                    ZVAL_DOUBLE(PHP5to7_MAYBE_PTR(value), entry->value.value.u64);
                } else {
                    ZVAL_LONG(PHP5to7_MAYBE_PTR(value), entry->value.value.u64);
                }
                break;
            case AMQP_FIELD_KIND_F32:
                ZVAL_DOUBLE(PHP5to7_MAYBE_PTR(value), entry->value.value.f32);
                break;
            case AMQP_FIELD_KIND_F64:
                ZVAL_DOUBLE(PHP5to7_MAYBE_PTR(value), entry->value.value.f64);
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
                                    (unsigned) entry->value.value.array.entries[j].value.bytes.len
                            );
                            break;
                        case AMQP_FIELD_KIND_TABLE: {
                            PHP5to7_zval_t subtable PHP5to7_MAYBE_SET_TO_NULL;
                            PHP5to7_MAYBE_INIT(subtable);
                            PHP5to7_ARRAY_INIT(subtable);

                            parse_amqp_table(
                                    &(entry->value.value.array.entries[j].value.table),
                                    PHP5to7_MAYBE_PTR(subtable) TSRMLS_CC
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
                parse_amqp_table(&(entry->value.value.table), PHP5to7_MAYBE_PTR(value) TSRMLS_CC);
                break;

            case AMQP_FIELD_KIND_TIMESTAMP: {
				char timestamp_str[20];
				PHP5to7_zval_t timestamp PHP5to7_MAYBE_SET_TO_NULL;
				PHP5to7_MAYBE_INIT(timestamp);

				int length = snprintf(timestamp_str, sizeof(timestamp_str), ZEND_ULONG_FMT, entry->value.value.u64);
                PHP5to7_ZVAL_STRINGL_DUP(PHP5to7_MAYBE_PTR(timestamp), (char *)timestamp_str, length);
				object_init_ex(PHP5to7_MAYBE_PTR(value), amqp_timestamp_class_entry);

				zend_call_method_with_1_params(
						PHP5to8_OBJ_PROP(&value),
						amqp_timestamp_class_entry,
						NULL,
						"__construct",
						NULL,
						PHP5to7_MAYBE_PTR(timestamp)
				);

                PHP5to7_MAYBE_DESTROY(timestamp);
				break;
			}

            case AMQP_FIELD_KIND_VOID:
                ZVAL_NULL(PHP5to7_MAYBE_PTR(value));
                break;
            case AMQP_FIELD_KIND_DECIMAL: {

                PHP5to7_zval_t e PHP5to7_MAYBE_SET_TO_NULL;
                PHP5to7_zval_t n PHP5to7_MAYBE_SET_TO_NULL;
                PHP5to7_MAYBE_INIT(e);
                PHP5to7_MAYBE_INIT(n);

                ZVAL_LONG(PHP5to7_MAYBE_PTR(e), entry->value.value.decimal.decimals);
                ZVAL_LONG(PHP5to7_MAYBE_PTR(n), entry->value.value.decimal.value);

                object_init_ex(PHP5to7_MAYBE_PTR(value), amqp_decimal_class_entry);

                zend_call_method_with_2_params(
                        PHP5to8_OBJ_PROP(&value),
                        amqp_decimal_class_entry,
                        NULL,
                        "__construct",
                        NULL,
                        PHP5to7_MAYBE_PTR(e),
                        PHP5to7_MAYBE_PTR(n)
                );

                PHP5to7_MAYBE_DESTROY(e);
                PHP5to7_MAYBE_DESTROY(n);
                break;
            }
            default:
                has_value = 0;
                break;
        }

        if (has_value) {
            char *key = estrndup(entry->key.bytes, (unsigned) entry->key.len);
            add_assoc_zval(result, key, PHP5to7_MAYBE_PTR(value));
            efree(key);
        } else {
            PHP5to7_MAYBE_DESTROY(value);
        }
    }
    return;
}

void php_amqp_basic_properties_extract(amqp_basic_properties_t *p, zval *obj TSRMLS_DC)
{
    PHP5to7_zval_t headers PHP5to7_MAYBE_SET_TO_NULL;

    PHP5to7_MAYBE_INIT(headers);
    PHP5to7_ARRAY_INIT(headers);

    if (p->_flags & AMQP_BASIC_CONTENT_TYPE_FLAG) {
        zend_update_property_stringl(this_ce, PHP5to8_OBJ_PROP(obj), ZEND_STRL("content_type"), (const char *) p->content_type.bytes, (PHP5to7_param_str_len_type_t) p->content_type.len TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, PHP5to8_OBJ_PROP(obj), ZEND_STRL("content_type"), "", 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_CONTENT_ENCODING_FLAG) {
        zend_update_property_stringl(this_ce, PHP5to8_OBJ_PROP(obj), ZEND_STRL("content_encoding"), (const char *) p->content_encoding.bytes, (PHP5to7_param_str_len_type_t) p->content_encoding.len TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, PHP5to8_OBJ_PROP(obj), ZEND_STRL("content_encoding"), "", 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_HEADERS_FLAG) {
        parse_amqp_table(&(p->headers), PHP5to7_MAYBE_PTR(headers) TSRMLS_CC);
    }

    zend_update_property(this_ce, PHP5to8_OBJ_PROP(obj), ZEND_STRL("headers"), PHP5to7_MAYBE_PTR(headers) TSRMLS_CC);

    if (p->_flags & AMQP_BASIC_DELIVERY_MODE_FLAG) {
        zend_update_property_long(this_ce, PHP5to8_OBJ_PROP(obj), ZEND_STRL("delivery_mode"), (PHP5to7_param_long_type_t) p->delivery_mode TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_long(this_ce, PHP5to8_OBJ_PROP(obj), ZEND_STRL("delivery_mode"), AMQP_DELIVERY_NONPERSISTENT TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_PRIORITY_FLAG) {
        zend_update_property_long(this_ce, PHP5to8_OBJ_PROP(obj), ZEND_STRL("priority"), (PHP5to7_param_long_type_t) p->priority TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_long(this_ce, PHP5to8_OBJ_PROP(obj), ZEND_STRL("priority"), 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_CORRELATION_ID_FLAG) {
        zend_update_property_stringl(this_ce, PHP5to8_OBJ_PROP(obj), ZEND_STRL("correlation_id"), (const char *) p->correlation_id.bytes, (PHP5to7_param_str_len_type_t) p->correlation_id.len TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, PHP5to8_OBJ_PROP(obj), ZEND_STRL("correlation_id"), "", 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_REPLY_TO_FLAG) {
        zend_update_property_stringl(this_ce, PHP5to8_OBJ_PROP(obj), ZEND_STRL("reply_to"), (const char *) p->reply_to.bytes, (PHP5to7_param_str_len_type_t) p->reply_to.len TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, PHP5to8_OBJ_PROP(obj), ZEND_STRL("reply_to"), "", 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_EXPIRATION_FLAG) {
        zend_update_property_stringl(this_ce, PHP5to8_OBJ_PROP(obj), ZEND_STRL("expiration"), (const char *) p->expiration.bytes, (PHP5to7_param_str_len_type_t) p->expiration.len TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, PHP5to8_OBJ_PROP(obj), ZEND_STRL("expiration"), "", 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_MESSAGE_ID_FLAG) {
        zend_update_property_stringl(this_ce, PHP5to8_OBJ_PROP(obj), ZEND_STRL("message_id"), (const char *) p->message_id.bytes, (PHP5to7_param_str_len_type_t) p->message_id.len TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, PHP5to8_OBJ_PROP(obj), ZEND_STRL("message_id"), "", 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_TIMESTAMP_FLAG) {
        zend_update_property_long(this_ce, PHP5to8_OBJ_PROP(obj), ZEND_STRL("timestamp"), (PHP5to7_param_long_type_t) p->timestamp TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_long(this_ce, PHP5to8_OBJ_PROP(obj), ZEND_STRL("timestamp"), 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_TYPE_FLAG) {
        zend_update_property_stringl(this_ce, PHP5to8_OBJ_PROP(obj), ZEND_STRL("type"), (const char *) p->type.bytes, (PHP5to7_param_str_len_type_t) p->type.len TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, PHP5to8_OBJ_PROP(obj), ZEND_STRL("type"), "", 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_USER_ID_FLAG) {
        zend_update_property_stringl(this_ce, PHP5to8_OBJ_PROP(obj), ZEND_STRL("user_id"), (const char *) p->user_id.bytes, (PHP5to7_param_str_len_type_t) p->user_id.len TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, PHP5to8_OBJ_PROP(obj), ZEND_STRL("user_id"), "", 0 TSRMLS_CC);
    }

    if (p->_flags & AMQP_BASIC_APP_ID_FLAG) {
        zend_update_property_stringl(this_ce, PHP5to8_OBJ_PROP(obj), ZEND_STRL("app_id"), (const char *) p->app_id.bytes, (PHP5to7_param_str_len_type_t) p->app_id.len TSRMLS_CC);
    } else {
        /* BC */
        zend_update_property_stringl(this_ce, PHP5to8_OBJ_PROP(obj), ZEND_STRL("app_id"), "", 0 TSRMLS_CC);
    }

    PHP5to7_MAYBE_DESTROY(headers);
}


/*
*Local variables:
*tab-width: 4
*c-basic-offset: 4
*End:
*vim600: noet sw=4 ts=4 fdm=marker
*vim<600: noet sw=4 ts=4
*/
