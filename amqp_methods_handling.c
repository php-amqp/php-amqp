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

#include "amqp_basic_properties.h"
#include "amqp_methods_handling.h"

/* taken from rabbbitmq-c */
static int amqp_id_in_reply_list(amqp_method_number_t expected, amqp_method_number_t *list) {
	while (*list != 0) {
		if (*list == expected) {
			return 1;
		}
		list++;
	}

	return 0;
}

/* taken from rabbbitmq-c */
static int amqp_simple_wait_method_list(amqp_connection_state_t state,
										amqp_channel_t expected_channel,
										amqp_method_number_t *expected_methods,
										amqp_method_t *output) {
	amqp_frame_t frame;
	int res = amqp_simple_wait_frame(state, &frame);

	if (AMQP_STATUS_OK != res) {
		return res;
	}

	if (AMQP_FRAME_METHOD != frame.frame_type ||
		expected_channel != frame.channel ||
		!amqp_id_in_reply_list(frame.payload.method.id, expected_methods)) {
		return AMQP_STATUS_WRONG_METHOD;
	}

	*output = frame.payload.method;
	return AMQP_STATUS_OK;
}

/* taken from rabbbitmq-c */
int amqp_simple_wait_method_list_noblock(amqp_connection_state_t state,
										 amqp_channel_t expected_channel,
										 amqp_method_number_t *expected_methods,
										 amqp_method_t *output,
										 struct timeval *timeout) {
	amqp_frame_t frame;
	int res = amqp_simple_wait_frame_noblock(state, &frame, timeout);

	if (AMQP_STATUS_OK != res) {
		return res;
	}

	if (AMQP_FRAME_METHOD != frame.frame_type ||
		expected_channel != frame.channel ||
		!amqp_id_in_reply_list(frame.payload.method.id, expected_methods)) {

		if (AMQP_CHANNEL_CLOSE_METHOD == frame.payload.method.id || AMQP_CONNECTION_CLOSE_METHOD == frame.payload.method.id) {

			*output = frame.payload.method;

			return AMQP_RESPONSE_SERVER_EXCEPTION;
		}

		return AMQP_STATUS_WRONG_METHOD;
	}

	*output = frame.payload.method;
	return AMQP_STATUS_OK;
}

/* taken from rabbbitmq-c */
int amqp_simple_wait_method_noblock(amqp_connection_state_t state,
                            amqp_channel_t expected_channel,
                            amqp_method_number_t expected_method,
                            amqp_method_t *output,
                            struct timeval *timeout)
{
  amqp_method_number_t expected_methods[] = { 0, 0 };
  expected_methods[0] = expected_method;

  return amqp_simple_wait_method_list_noblock(state, expected_channel, expected_methods, output, timeout);
}


int php_amqp_handle_basic_return(char **message, amqp_connection_resource *resource, amqp_channel_t channel_id, amqp_channel_object *channel, amqp_method_t *method TSRMLS_DC) {
	amqp_rpc_reply_t ret;
	amqp_message_t msg;
	int status = PHP_AMQP_RESOURCE_RESPONSE_OK;

	assert(AMQP_BASIC_RETURN_METHOD == method->id);

	amqp_basic_return_t *m = (amqp_basic_return_t *) method->decoded;

	ret = amqp_read_message(resource->connection_state, channel_id, &msg, 0);

	if (AMQP_RESPONSE_NORMAL != ret.reply_type) {
		return php_amqp_connection_resource_error(ret, message, resource, channel_id TSRMLS_CC);
	}

	if (channel->callbacks.basic_return.fci.size > 0) {
		status = php_amqp_call_basic_return_callback(m, &msg, &channel->callbacks.basic_return TSRMLS_CC);
	} else {
		zend_error(E_NOTICE, "Unhandled basic.return method from server received. Use AMQPChannel::setReturnCallback() to process it.");
		status = PHP_AMQP_RESOURCE_RESPONSE_BREAK;
	}

	amqp_destroy_message(&msg);

	return status;
}

int php_amqp_call_basic_return_callback(amqp_basic_return_t *m, amqp_message_t *msg, amqp_callback_bucket *cb TSRMLS_DC) {
	PHP5to7_zval_t params PHP5to7_MAYBE_SET_TO_NULL;
	PHP5to7_zval_t basic_properties PHP5to7_MAYBE_SET_TO_NULL;

	int status = PHP_AMQP_RESOURCE_RESPONSE_OK;

	PHP5to7_MAYBE_INIT(params);
	PHP5to7_ARRAY_INIT(params);

	PHP5to7_MAYBE_INIT(basic_properties);

	/* callback(int $reply_code, string $reply_text, string $exchange, string $routing_key, AMQPBasicProperties $properties, string $body); */

	add_next_index_long(PHP5to7_MAYBE_PTR(params), (PHP5to7_param_long_type_t) m->reply_code);
	PHP5to7_ADD_NEXT_INDEX_STRINGL_DUP(PHP5to7_MAYBE_PTR(params), (const char *) m->reply_text.bytes, (PHP5to7_param_str_len_type_t) m->reply_text.len);
	PHP5to7_ADD_NEXT_INDEX_STRINGL_DUP(PHP5to7_MAYBE_PTR(params), (const char *) m->exchange.bytes, (PHP5to7_param_str_len_type_t) m->exchange.len);
	PHP5to7_ADD_NEXT_INDEX_STRINGL_DUP(PHP5to7_MAYBE_PTR(params), (const char *) m->routing_key.bytes, (PHP5to7_param_str_len_type_t) m->routing_key.len);

	php_amqp_basic_properties_convert_to_zval(&msg->properties, PHP5to7_MAYBE_PTR(basic_properties) TSRMLS_CC);
	add_next_index_zval(PHP5to7_MAYBE_PTR(params), PHP5to7_MAYBE_PTR(basic_properties));
	Z_ADDREF_P(PHP5to7_MAYBE_PTR(basic_properties));

	PHP5to7_ADD_NEXT_INDEX_STRINGL_DUP(PHP5to7_MAYBE_PTR(params), (const char *) msg->body.bytes, (PHP5to7_param_str_len_type_t) msg->body.len);

	status = php_amqp_call_callback_with_params(params, cb TSRMLS_CC);

	PHP5to7_MAYBE_DESTROY(basic_properties);

	return status;
}

int php_amqp_handle_basic_ack(char **message, amqp_connection_resource *resource, amqp_channel_t channel_id, amqp_channel_object *channel, amqp_method_t *method TSRMLS_DC) {
	amqp_rpc_reply_t ret;
	int status = PHP_AMQP_RESOURCE_RESPONSE_OK;

	assert(AMQP_BASIC_ACK_METHOD == method->id);

	amqp_basic_ack_t *m = (amqp_basic_ack_t *) method->decoded;

	if (channel->callbacks.basic_ack.fci.size > 0) {
		status = php_amqp_call_basic_ack_callback(m, &channel->callbacks.basic_ack TSRMLS_CC);
	} else {
		zend_error(E_NOTICE, "Unhandled basic.ack method from server received. Use AMQPChannel::setConfirmCallback() to process it.");
		status = PHP_AMQP_RESOURCE_RESPONSE_BREAK;
	}

	return status;
}

int php_amqp_call_basic_ack_callback(amqp_basic_ack_t *m, amqp_callback_bucket *cb TSRMLS_DC) {
	PHP5to7_zval_t params PHP5to7_MAYBE_SET_TO_NULL;

	PHP5to7_MAYBE_INIT(params);
	PHP5to7_ARRAY_INIT(params);

	/* callback(int $delivery_tag, bool $multiple); */
	add_next_index_long(PHP5to7_MAYBE_PTR(params), (PHP5to7_param_long_type_t) m->delivery_tag);
	add_next_index_bool(PHP5to7_MAYBE_PTR(params), m->multiple);

	return php_amqp_call_callback_with_params(params, cb TSRMLS_CC);
}

int php_amqp_handle_basic_nack(char **message, amqp_connection_resource *resource, amqp_channel_t channel_id, amqp_channel_object *channel, amqp_method_t *method TSRMLS_DC) {
	amqp_rpc_reply_t ret;
	int status = PHP_AMQP_RESOURCE_RESPONSE_OK;

	assert(AMQP_BASIC_NACK_METHOD == method->id);

	amqp_basic_nack_t *m = (amqp_basic_nack_t *) method->decoded;

	if (channel->callbacks.basic_nack.fci.size > 0) {
		status = php_amqp_call_basic_nack_callback(m, &channel->callbacks.basic_nack TSRMLS_CC);
	} else {
		zend_error(E_NOTICE, "Unhandled basic.nack method from server received. Use AMQPChannel::setConfirmCallback() to process it.");
		status = PHP_AMQP_RESOURCE_RESPONSE_BREAK;
	}

	return status;
}

int php_amqp_call_basic_nack_callback(amqp_basic_nack_t *m, amqp_callback_bucket *cb TSRMLS_DC) {
	PHP5to7_zval_t params PHP5to7_MAYBE_SET_TO_NULL;

	PHP5to7_MAYBE_INIT(params);
	PHP5to7_ARRAY_INIT(params);

	/* callback(int $delivery_tag, bool $multiple, bool $requeue); */
	add_next_index_long(PHP5to7_MAYBE_PTR(params), (PHP5to7_param_long_type_t) m->delivery_tag);
	add_next_index_bool(PHP5to7_MAYBE_PTR(params), m->multiple);
	add_next_index_bool(PHP5to7_MAYBE_PTR(params), m->requeue);

	return php_amqp_call_callback_with_params(params, cb TSRMLS_CC);
}

int php_amqp_call_callback_with_params(PHP5to7_zval_t params, amqp_callback_bucket *cb TSRMLS_DC)
{
	zval retval;
	zval *retval_ptr = &retval;

	int status = PHP_AMQP_RESOURCE_RESPONSE_OK;

	ZVAL_NULL(&retval);

	/* Convert everything to be callable */
	zend_fcall_info_args(&cb->fci, PHP5to7_MAYBE_PTR(params) TSRMLS_CC);

	/* Initialize the return value pointer */
	PHP5to7_SET_FCI_RETVAL_PTR(cb->fci, retval_ptr);

	zend_call_function(&cb->fci, &cb->fcc TSRMLS_CC);

	/* Check if user land function wants to bail */
	if (EG(exception) || PHP5to7_IS_FALSE_P(retval_ptr)) {
		status = PHP_AMQP_RESOURCE_RESPONSE_BREAK;
	}

	/* Clean up our mess */
	zend_fcall_info_args_clear(&cb->fci, 1);
	PHP5to7_MAYBE_DESTROY(params);
	PHP5to7_MAYBE_DESTROY2(retval, retval_ptr);

	return status;
}


/*
*Local variables:
*tab-width: 4
*c-basic-offset: 4
*End:
*vim600: noet sw=4 ts=4 fdm=marker
*vim<600: noet sw=4 ts=4
*/
