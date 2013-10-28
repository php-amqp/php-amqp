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
  | Author: Duncan McIntyre dmcintyre@gopivotal.com Copyright(c) 2013                                               |
  +----------------------------------------------------------------------+
*/

void amqp_selector_dtor(void *object TSRMLS_DC);
zend_object_value amqp_selector_ctor(zend_class_entry *ce TSRMLS_DC);

PHP_METHOD(amqp_selector_class, __construct);
PHP_METHOD(amqp_selector_class, select);



/*
*Local variables:
*tab-width: 4
*c-basic-offset: 4
*End:
*vim600: noet sw=4 ts=4 fdm=marker
*vim<600: noet sw=4 ts=4
*/
