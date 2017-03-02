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
#include "amqp_type.h"

amqp_bytes_t php_amqp_type_char_to_amqp_long(char const *cstr, PHP5to7_param_str_len_type_t len)
{
	amqp_bytes_t result;

	if (len < 1) {
		return amqp_empty_bytes;
	}

	result.len   = (size_t)len;
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

void php_amqp_type_internal_convert_zval_array(zval *php_array, amqp_field_value_t **field, zend_bool allow_int_keys TSRMLS_DC)
{
	HashTable *ht;
	HashPosition pos;

	zval *value;
	zval **data;

	PHP5to7_ZEND_REAL_HASH_KEY_T *real_key;

	char *key;
	uint key_len;

	ulong index;
	ht = Z_ARRVAL_P(php_array);

	zend_bool is_amqp_array = 1;

	PHP5to7_ZEND_HASH_FOREACH_KEY_VAL(ht, index, real_key, key, key_len, data, value, pos) {
		if (PHP5to7_ZEND_HASH_KEY_IS_STRING(ht, real_key, key, key_len, index, pos)) {
			is_amqp_array = 0;
			break;
		}

	} PHP5to7_ZEND_HASH_FOREACH_END();

	if (is_amqp_array) {
		(*field)->kind = AMQP_FIELD_KIND_ARRAY;
		php_amqp_type_internal_convert_zval_to_amqp_array(php_array, &(*field)->value.array TSRMLS_CC);
	} else {
		(*field)->kind = AMQP_FIELD_KIND_TABLE;
		php_amqp_type_internal_convert_zval_to_amqp_table(php_array, &(*field)->value.table, allow_int_keys TSRMLS_CC);
	}
}

void php_amqp_type_internal_convert_zval_to_amqp_table(zval *php_array, amqp_table_t *amqp_table, zend_bool allow_int_keys TSRMLS_DC)
{
	HashTable *ht;
	HashPosition pos;

	zval *value;
	zval **data;

	PHP5to7_ZEND_REAL_HASH_KEY_T *real_key;

	char *key;
	uint key_len;

	ulong index;


	ht = Z_ARRVAL_P(php_array);

	amqp_table->entries = (amqp_table_entry_t *)ecalloc((size_t)zend_hash_num_elements(ht), sizeof(amqp_table_entry_t));
	amqp_table->num_entries = 0;

	PHP5to7_ZEND_HASH_FOREACH_KEY_VAL(ht, index, real_key, key, key_len, data, value, pos) {
		char *string_key;
		amqp_table_entry_t *table_entry;
		amqp_field_value_t *field;


		/* Now pull the key */

		if (!PHP5to7_ZEND_HASH_KEY_IS_STRING(ht, real_key, key, key_len, index, pos)) {
			if (allow_int_keys) {
				/* Convert to strings non-string keys */
				char str[32];

				key_len = sprintf(str, "%lu", index);
				key     = str;
			} else {
				/* Skip things that are not strings */
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Ignoring non-string header field '%lu'", index);

				continue;
			}
		} else {
			PHP5to7_ZEND_HASH_KEY_MAYBE_UNPACK(real_key, key, key_len);
		}

		/* Build the value */
		table_entry = &amqp_table->entries[amqp_table->num_entries++];
		field = &table_entry->value;

		if (!php_amqp_type_internal_convert_php_to_amqp_field_value(value, &field, key TSRMLS_CC)) {
			/* Reset entries counter back */
			amqp_table->num_entries --;

			continue;
		}

		string_key = estrndup(key, key_len);
		table_entry->key = amqp_cstring_bytes(string_key);

	} PHP5to7_ZEND_HASH_FOREACH_END();
}

void php_amqp_type_internal_convert_zval_to_amqp_array(zval *zvalArguments, amqp_array_t *arguments TSRMLS_DC)
{
	HashTable *ht;
	HashPosition pos;

	zval *value;
	zval **data;

	PHP5to7_ZEND_REAL_HASH_KEY_T *real_key;

	char *key;
	uint key_len;

	ulong index;

	char type[16];

	ht = Z_ARRVAL_P(zvalArguments);

	/* Allocate all the memory necessary for storing the arguments */
	arguments->entries = (amqp_field_value_t *)ecalloc((size_t)zend_hash_num_elements(ht), sizeof(amqp_field_value_t));
	arguments->num_entries = 0;

	PHP5to7_ZEND_HASH_FOREACH_KEY_VAL(ht, index, real_key, key, key_len, data, value, pos) {
		amqp_field_value_t *field = &arguments->entries[arguments->num_entries++];

		if (!php_amqp_type_internal_convert_php_to_amqp_field_value(value, &field, key TSRMLS_CC)) {
			/* Reset entries counter back */
			arguments->num_entries --;

			continue;
		}
	} PHP5to7_ZEND_HASH_FOREACH_END();
}

zend_bool php_amqp_type_internal_convert_php_to_amqp_field_value(zval *value, amqp_field_value_t **fieldPtr, char *key TSRMLS_DC)
{
	zend_bool result;
	char type[16];
	amqp_field_value_t *field;

	result = 1;
	field = *fieldPtr;

	switch (Z_TYPE_P(value)) {
		PHP5to7_CASE_IS_BOOL:
			field->kind = AMQP_FIELD_KIND_BOOLEAN;
			field->value.boolean = (amqp_boolean_t)!PHP5to7_IS_FALSE_P(value);
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
				bytes.bytes = estrndup(Z_STRVAL_P(value), (uint)Z_STRLEN_P(value));

				field->value.bytes = bytes;
			} else {
				field->value.bytes = amqp_empty_bytes;
			}

			break;
		case IS_ARRAY:
			php_amqp_type_internal_convert_zval_array(value, &field, 1 TSRMLS_CC);
			break;
		case IS_NULL:
			field->kind = AMQP_FIELD_KIND_VOID;
			break;
		default:
			switch(Z_TYPE_P(value)) {
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

			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Ignoring field '%s' due to unsupported value type (%s)", key, type);
			result = 0;
			break;
	}

	return result;
}

amqp_table_t *php_amqp_type_convert_zval_to_amqp_table(zval *php_array TSRMLS_DC)
{
	amqp_table_t *amqp_table;
	/* In setArguments, we are overwriting all the existing values */
	amqp_table = (amqp_table_t *)emalloc(sizeof(amqp_table_t));

	php_amqp_type_internal_convert_zval_to_amqp_table(php_array, amqp_table, 0 TSRMLS_CC);

	return amqp_table;
}

void php_amqp_type_internal_free_amqp_table(amqp_table_t *object, zend_bool clear_root)
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

void php_amqp_type_free_amqp_table(amqp_table_t *object)
{
	php_amqp_type_internal_free_amqp_table(object, 1);
}
