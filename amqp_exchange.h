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

/* $Id: amqp_exchange.h 318206 2011-10-19 04:17:01Z pdezwart $ */

void amqp_exchange_dtor(void *object TSRMLS_DC);
zend_object_value amqp_exchange_ctor(zend_class_entry *ce TSRMLS_DC);

PHP_METHOD(amqp_exchange_class, __construct);

PHP_METHOD(amqp_exchange_class, getName);
PHP_METHOD(amqp_exchange_class, setName);

PHP_METHOD(amqp_exchange_class, getType);
PHP_METHOD(amqp_exchange_class, setType);

PHP_METHOD(amqp_exchange_class, getFlags);
PHP_METHOD(amqp_exchange_class, setFlags);

PHP_METHOD(amqp_exchange_class, getArgument);
PHP_METHOD(amqp_exchange_class, getArguments);
PHP_METHOD(amqp_exchange_class, setArgument);
PHP_METHOD(amqp_exchange_class, setArguments);

PHP_METHOD(amqp_exchange_class, declareExchange);
PHP_METHOD(amqp_exchange_class, delete);
PHP_METHOD(amqp_exchange_class, bind);
PHP_METHOD(amqp_exchange_class, publish);


/*
*Local variables:
*tab-width: 4
*c-basic-offset: 4
*End:
*vim600: noet sw=4 ts=4 fdm=marker
*vim<600: noet sw=4 ts=4
*/
