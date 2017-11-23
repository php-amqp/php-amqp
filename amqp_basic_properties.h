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

#include "php.h"
#include "php_amqp.h"

extern zend_class_entry *amqp_basic_properties_class_entry;

void parse_amqp_table(amqp_table_t *table, zval *result TSRMLS_DC);
void php_amqp_basic_properties_extract(amqp_basic_properties_t *p, zval *obj TSRMLS_DC);


void php_amqp_basic_properties_convert_to_zval(amqp_basic_properties_t *props, zval *obj TSRMLS_DC);
void php_amqp_basic_properties_set_empty_headers(zval *obj TSRMLS_DC);


PHP_MINIT_FUNCTION(amqp_basic_properties);


/*
*Local variables:
*tab-width: 4
*c-basic-offset: 4
*End:
*vim600: noet sw=4 ts=4 fdm=marker
*vim<600: noet sw=4 ts=4
*/
