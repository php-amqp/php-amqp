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
#ifndef PHP_AMQP_METHODS_HANDLING_H
#define PHP_AMQP_METHODS_HANDLING_H

#include "php_amqp.h"
#include "amqp.h"
#include "php.h"

int amqp_simple_wait_method_list_noblock(amqp_connection_state_t state,
                                         amqp_channel_t expected_channel,
                                         amqp_method_number_t *expected_methods,
                                         amqp_method_t *output,
                                         struct timeval *timeout);

int amqp_simple_wait_method_noblock(amqp_connection_state_t state,
                            amqp_channel_t expected_channel,
                            amqp_method_number_t expected_method,
                            amqp_method_t *output,
                            struct timeval *timeout);

int php_amqp_call_callback_with_params(PHP5to7_zval_t params, amqp_callback_bucket *cb TSRMLS_DC);

int php_amqp_call_basic_return_callback(amqp_basic_return_t *m, amqp_message_t *msg, amqp_callback_bucket *cb TSRMLS_DC);
int php_amqp_handle_basic_return(char **message, amqp_connection_resource *resource, amqp_channel_t channel_id, amqp_channel_object *channel, amqp_method_t *method TSRMLS_DC);

int php_amqp_call_basic_ack_callback(amqp_basic_ack_t *m, amqp_callback_bucket *cb TSRMLS_DC);
int php_amqp_handle_basic_ack(char **message, amqp_connection_resource *resource, amqp_channel_t channel_id, amqp_channel_object *channel, amqp_method_t *method TSRMLS_DC);

int php_amqp_call_basic_nack_callback(amqp_basic_nack_t *m, amqp_callback_bucket *cb TSRMLS_DC);
int php_amqp_handle_basic_nack(char **message, amqp_connection_resource *resource, amqp_channel_t channel_id, amqp_channel_object *channel, amqp_method_t *method TSRMLS_DC);

#endif

/*
*Local variables:
*tab-width: 4
*c-basic-offset: 4
*End:
*vim600: noet sw=4 ts=4 fdm=marker
*vim<600: noet sw=4 ts=4
*/
