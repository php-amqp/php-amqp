/*
 * This file is part of the pdezwart/php-amqp PHP extension.
 *
 * Copyright (c) php-amqp contributors
 *
 * Licensed under the MIT license: http://opensource.org/licenses/MIT
 *
 * For the full copyright and license information, please view the
 * LICENSE file that was distributed with this source or visit
 * http://opensource.org/licenses/MIT
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"

#include <amqp.h>
#if PHP_MAJOR_VERSION >= 7
	#include "php7_support.h"
#else
	#include "php5_support.h"
#endif

PHP_MINIT_FUNCTION(amqp_type);

char *php_amqp_type_amqp_bytes_to_char(amqp_bytes_t bytes);
amqp_bytes_t php_amqp_type_char_to_amqp_long(char const *cstr, PHP5to7_param_str_len_type_t len);

amqp_table_t *php_amqp_type_convert_zval_to_amqp_table(zval *php_array TSRMLS_DC);
void php_amqp_type_free_amqp_table(amqp_table_t *object);

/** Internal functions */
zend_bool php_amqp_type_internal_convert_php_to_amqp_field_value(zval *value, amqp_field_value_t **fieldPtr, char *key TSRMLS_DC);
void php_amqp_type_internal_convert_zval_array(zval *php_array, amqp_field_value_t **field, zend_bool allow_int_keys TSRMLS_DC);
void php_amqp_type_internal_convert_zval_to_amqp_table(zval *php_array, amqp_table_t *amqp_table, zend_bool allow_int_keys TSRMLS_DC);
void php_amqp_type_internal_convert_zval_to_amqp_array(zval *php_array, amqp_array_t *amqp_array TSRMLS_DC);
