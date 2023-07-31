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

#if HAVE_LIBRABBITMQ_NEW_LAYOUT
    #include <rabbitmq-c/amqp.h>
#else
    #include <amqp.h>
#endif
#include "Zend/zend_interfaces.h"
#include "amqp_decimal.h"
#include "amqp_timestamp.h"
#include "amqp_type.h"

#ifdef PHP_WIN32
    #define strtoimax _strtoi64
#endif

static void php_amqp_type_internal_free_amqp_array(amqp_array_t *array);
static void php_amqp_type_internal_free_amqp_table(amqp_table_t *object, zend_bool clear_root);

amqp_bytes_t php_amqp_type_char_to_amqp_long(char const *cstr, size_t len)
{
    amqp_bytes_t result;

    if (len < 1) {
        return amqp_empty_bytes;
    }

    result.len = (size_t) len;
    result.bytes = (void *) cstr;

    return result;
}

char *php_amqp_type_amqp_bytes_to_char(amqp_bytes_t bytes)
{
    /* We will need up to 4 chars per byte, plus the terminating 0 */
    char *res = emalloc(bytes.len * 4 + 1);
    uint8_t *data = bytes.bytes;
    char *p = res;
    size_t i;

    for (i = 0; i < bytes.len; i++) {
        if (data[i] >= 32 && data[i] != 127) {
            *p++ = data[i];
        } else {
            *p++ = '\\';
            *p++ = '0' + (data[i] >> 6);
            *p++ = '0' + (data[i] >> 3 & 0x7);
            *p++ = '0' + (data[i] & 0x7);
        }
    }

    *p = 0;
    return res;
}

void php_amqp_type_internal_convert_zval_array(zval *php_array, amqp_field_value_t **field, zend_bool allow_int_keys)
{
    HashTable *ht;
    zend_string *key;

    ht = Z_ARRVAL_P(php_array);

    zend_bool is_amqp_array = 1;

    ZEND_HASH_FOREACH_STR_KEY(ht, key)
        if (key) {
            is_amqp_array = 0;
            break;
        }
    ZEND_HASH_FOREACH_END ();

    if (is_amqp_array) {
        (*field)->kind = AMQP_FIELD_KIND_ARRAY;
        php_amqp_type_internal_convert_zval_to_amqp_array(php_array, &(*field)->value.array);
    } else {
        (*field)->kind = AMQP_FIELD_KIND_TABLE;
        php_amqp_type_internal_convert_zval_to_amqp_table(php_array, &(*field)->value.table, allow_int_keys);
    }
}

void php_amqp_type_internal_convert_zval_to_amqp_table(
    zval *php_array,
    amqp_table_t *amqp_table,
    zend_bool allow_int_keys
)
{
    HashTable *ht;
    zval *value;
    zend_string *zkey;
    zend_ulong index;
    char *key;
    unsigned key_len;
    ht = Z_ARRVAL_P(php_array);

    amqp_table->entries =
        (amqp_table_entry_t *) ecalloc((size_t) zend_hash_num_elements(ht), sizeof(amqp_table_entry_t));
    amqp_table->num_entries = 0;

    ZEND_HASH_FOREACH_KEY_VAL(ht, index, zkey, value)
        char *string_key;
        amqp_table_entry_t *table_entry;
        amqp_field_value_t *field;

        /* Now pull the key */
        if (!zkey) {
            if (allow_int_keys) {
                /* Convert to strings non-string keys */
                char str[32];

                key_len = snprintf(str, 32, "%lu", index);
                key = str;
            } else {
                /* Skip things that are not strings */
                php_error_docref(NULL, E_WARNING, "Ignoring non-string header field '%lu'", index);

                continue;
            }
        } else {
            key_len = ZSTR_LEN(zkey);
            key = ZSTR_VAL(zkey);
        }

        string_key = estrndup(key, key_len);

        /* Build the value */
        table_entry = &amqp_table->entries[amqp_table->num_entries++];
        field = &table_entry->value;

        if (!php_amqp_type_internal_convert_php_to_amqp_field_value(value, &field, key)) {
            /* Reset entries counter back */
            amqp_table->num_entries--;

            continue;
        }

        table_entry->key = amqp_cstring_bytes(string_key);
    ZEND_HASH_FOREACH_END();
}

void php_amqp_type_internal_convert_zval_to_amqp_array(zval *zval_arguments, amqp_array_t *arguments)
{
    HashTable *ht;

    zval *value;

    zend_string *zkey;

    ht = Z_ARRVAL_P(zval_arguments);

    /* Allocate all the memory necessary for storing the arguments */
    arguments->entries =
        (amqp_field_value_t *) ecalloc((size_t) zend_hash_num_elements(ht), sizeof(amqp_field_value_t));
    arguments->num_entries = 0;

    ZEND_HASH_FOREACH_STR_KEY_VAL(ht, zkey, value)
        amqp_field_value_t *field = &arguments->entries[arguments->num_entries++];

        if (!php_amqp_type_internal_convert_php_to_amqp_field_value(value, &field, ZSTR_VAL(zkey))) {
            /* Reset entries counter back */
            arguments->num_entries--;

            continue;
        }
    ZEND_HASH_FOREACH_END ();
}

zend_bool php_amqp_type_internal_convert_php_to_amqp_field_value(zval *value, amqp_field_value_t **fieldPtr, char *key)
{
    zend_bool result;
    char type[16];
    amqp_field_value_t *field;

    result = 1;
    field = *fieldPtr;

    switch (Z_TYPE_P(value)) {
        case IS_TRUE:
        case IS_FALSE:
            field->kind = AMQP_FIELD_KIND_BOOLEAN;
            field->value.boolean = (amqp_boolean_t) Z_TYPE_P(value) != IS_FALSE;
            break;
        case IS_DOUBLE:
            field->kind = AMQP_FIELD_KIND_F64;
            field->value.f64 = Z_DVAL_P(value);
            break;
        case IS_LONG:
            field->kind = AMQP_FIELD_KIND_I64;
            field->value.i64 = Z_LVAL_P(value);
            break;
        case IS_STRING:
            field->kind = AMQP_FIELD_KIND_UTF8;

            if (Z_STRLEN_P(value)) {
                amqp_bytes_t bytes;
                bytes.len = (size_t) Z_STRLEN_P(value);
                bytes.bytes = estrndup(Z_STRVAL_P(value), (unsigned) Z_STRLEN_P(value));

                field->value.bytes = bytes;
            } else {
                field->value.bytes = amqp_empty_bytes;
            }

            break;
        case IS_ARRAY:
            php_amqp_type_internal_convert_zval_array(value, &field, 1);
            break;
        case IS_NULL:
            field->kind = AMQP_FIELD_KIND_VOID;
            break;
        case IS_OBJECT:
            if (instanceof_function(Z_OBJCE_P(value), amqp_timestamp_class_entry)) {
                zval result_zv;

                zend_call_method_with_0_params(
                    PHP_AMQP_COMPAT_OBJ_P(value),
                    amqp_timestamp_class_entry,
                    NULL,
                    "gettimestamp",
                    &result_zv
                );

                field->kind = AMQP_FIELD_KIND_TIMESTAMP;
                field->value.u64 = Z_DVAL(result_zv);

                zval_ptr_dtor(&result_zv);

                break;
            } else if (instanceof_function(Z_OBJCE_P(value), amqp_decimal_class_entry)) {
                field->kind = AMQP_FIELD_KIND_DECIMAL;
                zval result_zv;

                zend_call_method_with_0_params(
                    PHP_AMQP_COMPAT_OBJ_P(value),
                    amqp_decimal_class_entry,
                    NULL,
                    "getexponent",
                    &result_zv
                );
                field->value.decimal.decimals = (uint8_t) Z_LVAL(result_zv);
                zval_ptr_dtor(&result_zv);

                zend_call_method_with_0_params(
                    PHP_AMQP_COMPAT_OBJ_P(value),
                    amqp_decimal_class_entry,
                    NULL,
                    "getsignificand",
                    &result_zv
                );
                field->value.decimal.value = (uint32_t) Z_LVAL(result_zv);
                zval_ptr_dtor(&result_zv);

                break;
            }
        default:
            switch (Z_TYPE_P(value)) {
                case IS_OBJECT:
                    strcpy(type, "object");
                    break;
                case IS_RESOURCE:
                    strcpy(type, "resource");
                    break;
                default:
                    strcpy(type, "unknown");
                    break;
            }

            php_error_docref(NULL, E_WARNING, "Ignoring field '%s' due to unsupported value type (%s)", key, type);
            result = 0;
            break;
    }

    return result;
}

amqp_table_t *php_amqp_type_convert_zval_to_amqp_table(zval *php_array)
{
    amqp_table_t *amqp_table;
    /* In setArguments, we are overwriting all the existing values */
    amqp_table = (amqp_table_t *) emalloc(sizeof(amqp_table_t));

    php_amqp_type_internal_convert_zval_to_amqp_table(php_array, amqp_table, 0);

    return amqp_table;
}


static void php_amqp_type_internal_free_amqp_array(amqp_array_t *array)
{
    if (!array) {
        return;
    }

    int macroEntryCounter;
    for (macroEntryCounter = 0; macroEntryCounter < array->num_entries; macroEntryCounter++) {

        amqp_field_value_t *entry = &array->entries[macroEntryCounter];

        switch (entry->kind) {
            case AMQP_FIELD_KIND_TABLE:
                php_amqp_type_internal_free_amqp_table(&entry->value.table, 0);
                break;
            case AMQP_FIELD_KIND_ARRAY:
                php_amqp_type_internal_free_amqp_array(&entry->value.array);
                break;
            case AMQP_FIELD_KIND_UTF8:
                if (entry->value.bytes.bytes) {
                    efree(entry->value.bytes.bytes);
                }
                break;
                //
            default:
                break;
        }
    }

    if (array->entries) {
        efree(array->entries);
    }
}

static void php_amqp_type_internal_free_amqp_table(amqp_table_t *object, zend_bool clear_root)
{
    if (!object) {
        return;
    }

    if (object->entries) {
        int macroEntryCounter;
        for (macroEntryCounter = 0; macroEntryCounter < object->num_entries; macroEntryCounter++) {

            amqp_table_entry_t *entry = &object->entries[macroEntryCounter];
            efree(entry->key.bytes);

            switch (entry->value.kind) {
                case AMQP_FIELD_KIND_TABLE:
                    php_amqp_type_internal_free_amqp_table(&entry->value.value.table, 0);
                    break;
                case AMQP_FIELD_KIND_ARRAY:
                    php_amqp_type_internal_free_amqp_array(&entry->value.value.array);
                    break;
                case AMQP_FIELD_KIND_UTF8:
                    if (entry->value.value.bytes.bytes) {
                        efree(entry->value.value.bytes.bytes);
                    }
                    break;
                default:
                    break;
            }
        }
        efree(object->entries);
    }

    if (clear_root) {
        efree(object);
    }
}

void php_amqp_type_free_amqp_table(amqp_table_t *object) { php_amqp_type_internal_free_amqp_table(object, 1); }
