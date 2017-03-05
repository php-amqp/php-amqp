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
