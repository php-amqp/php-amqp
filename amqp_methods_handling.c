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
static int amqp_id_in_reply_list(amqp_method_number_t expected, amqp_method_number_t *list)
{
    while (*list != 0) {
        if (*list == expected) {
            return 1;
        }
        list++;
    }

    return 0;
}

/* taken from rabbbitmq-c */
int amqp_simple_wait_method_list_noblock(
    amqp_connection_state_t state,
    amqp_channel_t expected_channel,
    amqp_method_number_t *expected_methods,
    amqp_method_t *output,
    struct timeval *timeout
)
{
    amqp_frame_t frame;
    int res = amqp_simple_wait_frame_noblock(state, &frame, timeout);

    if (AMQP_STATUS_OK != res) {
        return res;
    }

    if (AMQP_FRAME_METHOD != frame.frame_type || expected_channel != frame.channel ||
        !amqp_id_in_reply_list(frame.payload.method.id, expected_methods)) {

        if (AMQP_CHANNEL_CLOSE_METHOD == frame.payload.method.id ||
            AMQP_CONNECTION_CLOSE_METHOD == frame.payload.method.id) {

            *output = frame.payload.method;

            return AMQP_RESPONSE_SERVER_EXCEPTION;
        }

        return AMQP_STATUS_WRONG_METHOD;
    }

    *output = frame.payload.method;
    return AMQP_STATUS_OK;
}

/* taken from rabbbitmq-c */
int amqp_simple_wait_method_noblock(
    amqp_connection_state_t state,
    amqp_channel_t expected_channel,
    amqp_method_number_t expected_method,
    amqp_method_t *output,
    struct timeval *timeout
)
{
    amqp_method_number_t expected_methods[] = {0, 0};
    expected_methods[0] = expected_method;

    return amqp_simple_wait_method_list_noblock(state, expected_channel, expected_methods, output, timeout);
}


int php_amqp_handle_basic_return(
    char **message,
    amqp_connection_resource *resource,
    amqp_channel_t channel_id,
    amqp_channel_object *channel,
    amqp_method_t *method TSRMLS_DC
)
{
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
        zend_error(
            E_NOTICE,
            "Unhandled basic.return method from server received. Use AMQPChannel::setReturnCallback() to process it."
        );
        status = PHP_AMQP_RESOURCE_RESPONSE_BREAK;
    }

    amqp_destroy_message(&msg);

    return status;
}

int php_amqp_call_basic_return_callback(amqp_basic_return_t *m, amqp_message_t *msg, amqp_callback_bucket *cb TSRMLS_DC)
{
    zval params;
    zval basic_properties;

    int status = PHP_AMQP_RESOURCE_RESPONSE_OK;

    ZVAL_UNDEF(&params);
    array_init(&params);

    ZVAL_UNDEF(&basic_properties);

    /* callback(int $reply_code, string $reply_text, string $exchange, string $routing_key, AMQPBasicProperties $properties, string $body); */

    add_next_index_long(&params, (zend_long) m->reply_code);
    add_next_index_stringl(&params, m->reply_text.bytes, m->reply_text.len);
    add_next_index_stringl(&params, m->exchange.bytes, m->exchange.len);
    add_next_index_stringl(&params, m->routing_key.bytes, m->routing_key.len);

    php_amqp_basic_properties_convert_to_zval(&msg->properties, &basic_properties TSRMLS_CC);
    add_next_index_zval(&params, &basic_properties);
    Z_ADDREF_P(&basic_properties);

    add_next_index_stringl(&params, msg->body.bytes, msg->body.len);

    status = php_amqp_call_callback_with_params(params, cb TSRMLS_CC);

    zval_ptr_dtor(&basic_properties);

    return status;
}

int php_amqp_handle_basic_ack(
    char **message,
    amqp_connection_resource *resource,
    amqp_channel_t channel_id,
    amqp_channel_object *channel,
    amqp_method_t *method TSRMLS_DC
)
{
    int status = PHP_AMQP_RESOURCE_RESPONSE_OK;

    assert(AMQP_BASIC_ACK_METHOD == method->id);

    amqp_basic_ack_t *m = (amqp_basic_ack_t *) method->decoded;

    if (channel->callbacks.basic_ack.fci.size > 0) {
        status = php_amqp_call_basic_ack_callback(m, &channel->callbacks.basic_ack TSRMLS_CC);
    } else {
        zend_error(
            E_NOTICE,
            "Unhandled basic.ack method from server received. Use AMQPChannel::setConfirmCallback() to process it."
        );
        status = PHP_AMQP_RESOURCE_RESPONSE_BREAK;
    }

    return status;
}

int php_amqp_call_basic_ack_callback(amqp_basic_ack_t *m, amqp_callback_bucket *cb TSRMLS_DC)
{
    zval params;

    ZVAL_UNDEF(&params);
    array_init(&params);

    /* callback(int $delivery_tag, bool $multiple); */
    add_next_index_long(&params, (zend_long) m->delivery_tag);
    add_next_index_bool(&params, m->multiple);

    return php_amqp_call_callback_with_params(params, cb TSRMLS_CC);
}

int php_amqp_handle_basic_nack(
    char **message,
    amqp_connection_resource *resource,
    amqp_channel_t channel_id,
    amqp_channel_object *channel,
    amqp_method_t *method TSRMLS_DC
)
{
    int status = PHP_AMQP_RESOURCE_RESPONSE_OK;

    assert(AMQP_BASIC_NACK_METHOD == method->id);

    amqp_basic_nack_t *m = (amqp_basic_nack_t *) method->decoded;

    if (channel->callbacks.basic_nack.fci.size > 0) {
        status = php_amqp_call_basic_nack_callback(m, &channel->callbacks.basic_nack TSRMLS_CC);
    } else {
        zend_error(
            E_NOTICE,
            "Unhandled basic.nack method from server received. Use AMQPChannel::setConfirmCallback() to process it."
        );
        status = PHP_AMQP_RESOURCE_RESPONSE_BREAK;
    }

    return status;
}

int php_amqp_call_basic_nack_callback(amqp_basic_nack_t *m, amqp_callback_bucket *cb TSRMLS_DC)
{
    zval params;

    ZVAL_UNDEF(&params);
    array_init(&params);

    /* callback(int $delivery_tag, bool $multiple, bool $requeue); */
    add_next_index_long(&params, (zend_long) m->delivery_tag);
    add_next_index_bool(&params, m->multiple);
    add_next_index_bool(&params, m->requeue);

    return php_amqp_call_callback_with_params(params, cb TSRMLS_CC);
}

int php_amqp_call_callback_with_params(zval params, amqp_callback_bucket *cb TSRMLS_DC)
{
    zval retval;
    zval *retval_ptr = &retval;

    int status = PHP_AMQP_RESOURCE_RESPONSE_OK;

    ZVAL_NULL(&retval);

    /* Convert everything to be callable */
    zend_fcall_info_args(&cb->fci, &params TSRMLS_CC);

    /* Initialize the return value pointer */
    cb->fci.retval = retval_ptr;

    zend_call_function(&cb->fci, &cb->fcc TSRMLS_CC);

    /* Check if user land function wants to bail */
    if (EG(exception) || Z_TYPE_P(retval_ptr) == IS_FALSE) {
        status = PHP_AMQP_RESOURCE_RESPONSE_BREAK;
    }

    /* Clean up our mess */
    zend_fcall_info_args_clear(&cb->fci, 1);
    zval_ptr_dtor(&params);
    zval_ptr_dtor(retval_ptr);

    return status;
}
