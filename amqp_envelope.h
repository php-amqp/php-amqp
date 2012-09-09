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

/* $Id: amqp_envelope.h 321054 2011-12-16 01:23:28Z pdezwart $ */


void amqp_envelope_dtor(void *object TSRMLS_DC);
zend_object_value amqp_envelope_ctor(zend_class_entry *ce TSRMLS_DC);

PHP_METHOD(amqp_envelope_class, __construct);
PHP_METHOD(amqp_envelope_class, getBody);
PHP_METHOD(amqp_envelope_class, getRoutingKey);
PHP_METHOD(amqp_envelope_class, getDeliveryTag);
PHP_METHOD(amqp_envelope_class, getDeliveryMode);
PHP_METHOD(amqp_envelope_class, getExchangeName);
PHP_METHOD(amqp_envelope_class, isRedelivery);
PHP_METHOD(amqp_envelope_class, getContentType);
PHP_METHOD(amqp_envelope_class, getContentEncoding);
PHP_METHOD(amqp_envelope_class, getType);
PHP_METHOD(amqp_envelope_class, getTimestamp);
PHP_METHOD(amqp_envelope_class, getPriority);
PHP_METHOD(amqp_envelope_class, getExpiration);
PHP_METHOD(amqp_envelope_class, getUserId);
PHP_METHOD(amqp_envelope_class, getAppId);
PHP_METHOD(amqp_envelope_class, getMessageId);
PHP_METHOD(amqp_envelope_class, getReplyTo);
PHP_METHOD(amqp_envelope_class, getCorrelationId);
PHP_METHOD(amqp_envelope_class, getHeaders);
PHP_METHOD(amqp_envelope_class, getHeader);



/*
*Local variables:
*tab-width: 4
*c-basic-offset: 4
*End:
*vim600: noet sw=4 ts=4 fdm=marker
*vim<600: noet sw=4 ts=4
*/
