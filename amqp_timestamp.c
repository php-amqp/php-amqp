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

zend_class_entry *amqp_timestamp_class_entry;
#define this_ce amqp_timestamp_class_entry

/* {{{ proto AMQPTimestamp::__construct(string $timestamp)
 */
static PHP_METHOD(amqp_timestamp_class, __construct)
{
	double timestamp;
	zval *timestamp_value;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d", &timestamp) == FAILURE) {
		return;
	}

	if (timestamp < 0) {
		zend_throw_exception(amqp_value_exception_class_entry, "The timestamp parameter must be greater than 0.", 0 TSRMLS_CC);
		return;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &timestamp_value) == FAILURE) {
		return;
	}

	zend_update_property_str(this_ce, getThis(), ZEND_STRL("timestamp"), zval_get_string(timestamp_value));
}
/* }}} */


/* {{{ proto int AMQPTimestamp::getTimestamp()
Get timestamp */
static PHP_METHOD(amqp_timestamp_class, getTimestamp)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;
	PHP_AMQP_NOPARAMS();

	PHP_AMQP_RETURN_THIS_PROP("timestamp");
}
/* }}} */


/* {{{ proto string AMQPTimestamp::__toString()
Return timestamp as string */
static PHP_METHOD(amqp_timestamp_class, __toString)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;
	PHP_AMQP_NOPARAMS();

	PHP_AMQP_RETURN_THIS_PROP("timestamp");
}
/* }}} */

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_timestamp_class_construct, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, timestamp)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_timestamp_class_getTimestamp, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_timestamp_class_toString, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
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
	this_ce = zend_register_internal_class(&ce TSRMLS_CC);

	zend_declare_property_null(this_ce, ZEND_STRL("timestamp"), ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}

/*
*Local variables:
*tab-width: 4
*c-basic-offset: 4
*End:
*vim600: noet sw=4 ts=4 fdm=marker
*vim<6
*/
