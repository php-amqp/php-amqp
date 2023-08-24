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
#include "zend_exceptions.h"
#include "php_amqp.h"
#include "ext/standard/php_math.h"

zend_class_entry *amqp_value_class_entry;
#define this_ce amqp_value_class_entry

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_value_class_toAmqpValue, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

zend_function_entry amqp_value_class_functions[] = {
    PHP_ABSTRACT_ME(amqp_value_class, toAmqpValue, arginfo_amqp_value_class_toAmqpValue)

        {NULL, NULL, NULL}
};

PHP_MINIT_FUNCTION(amqp_value)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "AMQPValue", amqp_value_class_functions);
    amqp_value_class_entry = zend_register_internal_interface(&ce);

    return SUCCESS;
}
