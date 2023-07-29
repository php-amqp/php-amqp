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
#include "ext/standard/php_math.h"

zend_class_entry *amqp_timestamp_class_entry;
#define this_ce amqp_timestamp_class_entry

static const double AMQP_TIMESTAMP_MAX = 0xFFFFFFFFFFFFFFFFp0;
static const double AMQP_TIMESTAMP_MIN = 0;

/* {{{ proto AMQPTimestamp::__construct(float $timestamp)
 */
static PHP_METHOD(amqp_timestamp_class, __construct)
{
    double timestamp;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "d", &timestamp) == FAILURE) {
        return;
    }

    if (timestamp < AMQP_TIMESTAMP_MIN) {
        zend_throw_exception_ex(
            amqp_value_exception_class_entry,
            0,
            "The timestamp parameter must be greater than %0.f.",
            AMQP_TIMESTAMP_MIN
        );
        return;
    }

    if (timestamp > AMQP_TIMESTAMP_MAX) {
        zend_throw_exception_ex(
            amqp_value_exception_class_entry,
            0,
            "The timestamp parameter must be less than %0.f.",
            AMQP_TIMESTAMP_MAX
        );
        return;
    }

    zend_update_property_double(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("timestamp"), floor(timestamp));
}
/* }}} */


/* {{{ proto float AMQPTimestamp::getTimestamp()
Get timestamp */
static PHP_METHOD(amqp_timestamp_class, getTimestamp)
{
    zval rv;
    PHP_AMQP_NOPARAMS();

    PHP_AMQP_RETURN_THIS_PROP("timestamp");
}
/* }}} */


/* {{{ proto string AMQPTimestamp::__toString()
Return timestamp as string */
static PHP_METHOD(amqp_timestamp_class, __toString)
{
    zval rv;
    PHP_AMQP_NOPARAMS();

    zval *timestamp = zend_read_property(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("timestamp"), 0, &rv);

    RETURN_NEW_STR(_php_math_number_format_ex(Z_DVAL_P(timestamp), 0, "", 0, "", 0));
}
/* }}} */

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_timestamp_class_construct, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
    ZEND_ARG_TYPE_INFO(0, timestamp, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_timestamp_class_getTimestamp, ZEND_SEND_BY_VAL, 0, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_timestamp_class_toString, ZEND_SEND_BY_VAL, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

zend_function_entry amqp_timestamp_class_functions[] = {
	PHP_ME(amqp_timestamp_class, __construct, 	arginfo_amqp_timestamp_class_construct,	ZEND_ACC_PUBLIC)
	PHP_ME(amqp_timestamp_class, getTimestamp, 	arginfo_amqp_timestamp_class_getTimestamp,	ZEND_ACC_PUBLIC)
	PHP_ME(amqp_timestamp_class, __toString, 	arginfo_amqp_timestamp_class_toString,	ZEND_ACC_PUBLIC)

    {NULL, NULL, NULL}
};


PHP_MINIT_FUNCTION(amqp_timestamp)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "AMQPTimestamp", amqp_timestamp_class_functions);
    this_ce = zend_register_internal_class(&ce);
    this_ce->ce_flags = this_ce->ce_flags | ZEND_ACC_FINAL;

    PHP_AMQP_DECLARE_TYPED_PROPERTY(this_ce, "timestamp", ZEND_ACC_PRIVATE, IS_DOUBLE, 0);

    zend_declare_class_constant_double(this_ce, ZEND_STRL("MAX"), AMQP_TIMESTAMP_MAX);
    zend_declare_class_constant_double(this_ce, ZEND_STRL("MIN"), AMQP_TIMESTAMP_MIN);

    return SUCCESS;
}
