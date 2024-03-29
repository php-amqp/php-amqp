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
#include "amqp_value.h"

zend_class_entry *amqp_decimal_class_entry;
#define this_ce amqp_decimal_class_entry

static const zend_long AMQP_DECIMAL_EXPONENT_MIN = 0;
static const zend_long AMQP_DECIMAL_EXPONENT_MAX = UINT8_MAX;
static const zend_long AMQP_DECIMAL_SIGNIFICAND_MIN = 0;
static const zend_long AMQP_DECIMAL_SIGNIFICAND_MAX = UINT32_MAX;


/* {{{ proto AMQPDecimal::__construct(int $e, int $n)
 */
static PHP_METHOD(amqp_decimal_class, __construct)
{
    zend_long exponent, significand;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll", &exponent, &significand) == FAILURE) {
        RETURN_THROWS();
    }

    if (exponent < AMQP_DECIMAL_EXPONENT_MIN) {
        zend_throw_exception_ex(amqp_value_exception_class_entry, 0, "Decimal exponent value must be unsigned.");
        return;
    }

    if (exponent > AMQP_DECIMAL_EXPONENT_MAX) {
        zend_throw_exception_ex(
            amqp_value_exception_class_entry,
            0,
            "Decimal exponent value must be less than %u.",
            (unsigned) AMQP_DECIMAL_EXPONENT_MAX
        );
        return;
    }
    if (significand < AMQP_DECIMAL_SIGNIFICAND_MIN) {
        zend_throw_exception_ex(amqp_value_exception_class_entry, 0, "Decimal significand value must be unsigned.");
        return;
    }

    if (significand > AMQP_DECIMAL_SIGNIFICAND_MAX) {
        zend_throw_exception_ex(
            amqp_value_exception_class_entry,
            0,
            "Decimal significand value must be less than %u.",
            (unsigned) AMQP_DECIMAL_SIGNIFICAND_MAX
        );
        return;
    }

    zend_update_property_long(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("exponent"), exponent);
    zend_update_property_long(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("significand"), significand);
}
/* }}} */

/* {{{ proto int AMQPDecimal::getExponent()
Get exponent */
static PHP_METHOD(amqp_decimal_class, getExponent)
{
    zval rv;
    PHP_AMQP_NOPARAMS()

    PHP_AMQP_RETURN_THIS_PROP("exponent");
}
/* }}} */

/* {{{ proto int AMQPDecimal::getSignificand()
Get significand */
static PHP_METHOD(amqp_decimal_class, getSignificand)
{
    zval rv;
    PHP_AMQP_NOPARAMS()

    PHP_AMQP_RETURN_THIS_PROP("significand");
}
/* }}} */

/* {{{ proto int AMQPDecimal::toAmqpValue()
Get AMQPDecimal as AMQPValue */
static PHP_METHOD(amqp_decimal_class, toAmqpValue)
{
    PHP_AMQP_NOPARAMS()

    RETURN_ZVAL(getThis(), 1, 0);
}
/* }}} */


ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_decimal_class_construct, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 2)
    ZEND_ARG_TYPE_INFO(0, exponent, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, significand, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_decimal_class_getExponent, ZEND_SEND_BY_VAL, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_decimal_class_getSignificand, ZEND_SEND_BY_VAL, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_decimal_class_toAmqpValue, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

zend_function_entry amqp_decimal_class_functions[] = {
	PHP_ME(amqp_decimal_class, __construct, 	arginfo_amqp_decimal_class_construct,	ZEND_ACC_PUBLIC)
	PHP_ME(amqp_decimal_class, getExponent, 	arginfo_amqp_decimal_class_getExponent,	ZEND_ACC_PUBLIC)
	PHP_ME(amqp_decimal_class, getSignificand, 	arginfo_amqp_decimal_class_getSignificand,	ZEND_ACC_PUBLIC)
	PHP_ME(amqp_decimal_class, toAmqpValue, arginfo_amqp_decimal_class_toAmqpValue, ZEND_ACC_PUBLIC)

    {NULL, NULL, NULL}
};


PHP_MINIT_FUNCTION(amqp_decimal)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "AMQPDecimal", amqp_decimal_class_functions);
    this_ce = zend_register_internal_class(&ce);
    zend_class_implements(this_ce, 1, amqp_value_class_entry);
    this_ce->ce_flags |= ZEND_ACC_FINAL;
#if PHP_VERSION_ID >= 80200
    this_ce->ce_flags |= ZEND_ACC_READONLY_CLASS;
#endif

    zend_declare_class_constant_long(this_ce, ZEND_STRL("EXPONENT_MIN"), AMQP_DECIMAL_EXPONENT_MIN);
    zend_declare_class_constant_long(this_ce, ZEND_STRL("EXPONENT_MAX"), AMQP_DECIMAL_EXPONENT_MAX);
    zend_declare_class_constant_long(this_ce, ZEND_STRL("SIGNIFICAND_MIN"), AMQP_DECIMAL_SIGNIFICAND_MIN);
    zend_declare_class_constant_long(this_ce, ZEND_STRL("SIGNIFICAND_MAX"), AMQP_DECIMAL_SIGNIFICAND_MAX);

    PHP_AMQP_DECLARE_TYPED_PROPERTY(this_ce, "exponent", ZEND_ACC_PRIVATE, IS_LONG, 0);
    PHP_AMQP_DECLARE_TYPED_PROPERTY(this_ce, "significand", ZEND_ACC_PRIVATE, IS_LONG, 0);

    return SUCCESS;
}
