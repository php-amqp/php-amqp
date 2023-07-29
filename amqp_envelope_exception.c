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
#include "php_ini.h"
#include "zend_exceptions.h"
#include "php_amqp.h"

zend_class_entry *amqp_envelope_exception_class_entry;
#define this_ce amqp_envelope_exception_class_entry

/* {{{ proto float AMQPEnvelopeException::getEnvelope()
Get AMQPEnvelope object */
static PHP_METHOD(amqp_envelope_exception_class, getEnvelope)
{
    zval rv;
    PHP_AMQP_NOPARAMS();

    PHP_AMQP_RETURN_THIS_PROP("envelope");
}
/* }}} */


ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(
    arginfo_amqp_envelope_exception_class_getEnvelope,
    ZEND_SEND_BY_VAL,
    0,
    AMQPEnvelope,
    0
)
ZEND_END_ARG_INFO()

zend_function_entry amqp_envelope_exception_class_functions[] = {
	PHP_ME(amqp_envelope_exception_class, getEnvelope, 	arginfo_amqp_envelope_exception_class_getEnvelope,	ZEND_ACC_PUBLIC)

    {NULL, NULL, NULL}
};


PHP_MINIT_FUNCTION(amqp_envelope_exception)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "AMQPEnvelopeException", amqp_envelope_exception_class_functions);
    amqp_envelope_exception_class_entry = zend_register_internal_class_ex(&ce, amqp_exception_class_entry);

    PHP_AMQP_DECLARE_TYPED_PROPERTY_OBJ(
        amqp_envelope_exception_class_entry,
        "envelope",
        ZEND_ACC_PRIVATE,
        AMQPEnvelope,
        0
    );

    return SUCCESS;
}
