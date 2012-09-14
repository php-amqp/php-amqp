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

/* $Id: amqp_queue.h 321959 2012-01-09 17:56:10Z pdezwart $ */

void amqp_queue_dtor(void *object TSRMLS_DC);
zend_object_value amqp_queue_ctor(zend_class_entry *ce TSRMLS_DC);

PHP_METHOD(amqp_queue_class, __construct);

PHP_METHOD(amqp_queue_class, getName);
PHP_METHOD(amqp_queue_class, setName);

PHP_METHOD(amqp_queue_class, getFlags);
PHP_METHOD(amqp_queue_class, setFlags);

PHP_METHOD(amqp_queue_class, getArgument);
PHP_METHOD(amqp_queue_class, getArguments);
PHP_METHOD(amqp_queue_class, setArgument);
PHP_METHOD(amqp_queue_class, setArguments);

PHP_METHOD(amqp_queue_class, declareQueue);
PHP_METHOD(amqp_queue_class, bind);

PHP_METHOD(amqp_queue_class, get);
PHP_METHOD(amqp_queue_class, consume);
PHP_METHOD(amqp_queue_class, ack);
PHP_METHOD(amqp_queue_class, nack);
PHP_METHOD(amqp_queue_class, reject);
PHP_METHOD(amqp_queue_class, purge);

PHP_METHOD(amqp_queue_class, cancel);
PHP_METHOD(amqp_queue_class, unbind);
PHP_METHOD(amqp_queue_class, delete);

/*
*Local variables:
*tab-width: 4
*c-basic-offset: 4
*End:
*vim600: noet sw=4 ts=4 fdm=marker
*vim<600: noet sw=4 ts=4
*/
