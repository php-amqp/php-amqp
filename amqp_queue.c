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
#include "ext/standard/info.h"
#include "zend_exceptions.h"

#ifdef PHP_WIN32
    #if PHP_VERSION_ID >= 80000
        #include "main/php_stdint.h"
    #else
        #include "win32/php_stdint.h"
    #endif
    #include "win32/signal.h"
#else
    #include <signal.h>
    #include <stdint.h>
#endif
#if HAVE_LIBRABBITMQ_NEW_LAYOUT
    #include <rabbitmq-c/amqp.h>
    #include <rabbitmq-c/framing.h>
#else
    #include <amqp.h>
    #include <amqp_framing.h>
#endif

#ifdef PHP_WIN32
    #include "win32/unistd.h"
#else
    #include <unistd.h>
#endif

#include "php_amqp.h"
#include "amqp_envelope.h"
#include "amqp_connection.h"
#include "amqp_channel.h"
#include "amqp_connection.h"
#include "amqp_envelope.h"
#include "amqp_queue.h"
#include "amqp_type.h"
#include "php_amqp.h"

zend_class_entry *amqp_queue_class_entry;
#define this_ce amqp_queue_class_entry


/* {{{ proto AMQPQueue::__construct(AMQPChannel channel)
AMQPQueue constructor
*/
static PHP_METHOD(amqp_queue_class, __construct)
{
    zval rv;

    zval arguments;

    zval *channelObj;
    amqp_channel_resource *channel_resource;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &channelObj, amqp_channel_class_entry) == FAILURE) {
        return;
    }

    ZVAL_UNDEF(&arguments);
    array_init(&arguments);
    zend_update_property(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("arguments"), &arguments TSRMLS_CC);
    zval_ptr_dtor(&arguments);

    channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(channelObj);
    PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not create queue.");

    zend_update_property(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("channel"), channelObj TSRMLS_CC);
    zend_update_property(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("connection"),
        PHP_AMQP_READ_OBJ_PROP(amqp_channel_class_entry, channelObj, "connection") TSRMLS_CC
    );
}
/* }}} */


/* {{{ proto AMQPQueue::getName()
Get the queue name */
static PHP_METHOD(amqp_queue_class, getName)
{
    zval rv;

    PHP_AMQP_NOPARAMS();

    if (PHP_AMQP_READ_THIS_PROP_STRLEN("name") > 0) {
        PHP_AMQP_RETURN_THIS_PROP("name");
    } else {
        RETURN_NULL();
    }
}
/* }}} */


/* {{{ proto AMQPQueue::setName(string name)
Set the queue name */
static PHP_METHOD(amqp_queue_class, setName)
{
    char *name = NULL;
    size_t name_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len) == FAILURE) {
        return;
    }

    if (name_len < 1 || name_len > 255) {
        /* Verify that the name is not null and not an empty string */
        zend_throw_exception(
            amqp_queue_exception_class_entry,
            "Invalid queue name given, must be between 1 and 255 characters long.",
            0 TSRMLS_CC
        );
        return;
    }

    /* Set the queue name */
    zend_update_property_stringl(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("name"),
        name,
        name_len TSRMLS_CC
    );
}
/* }}} */


/* {{{ proto AMQPQueue::getFlags()
Get the queue parameters */
static PHP_METHOD(amqp_queue_class, getFlags)
{
    zval rv;

    zend_long flags = 0;

    PHP_AMQP_NOPARAMS();

    if (PHP_AMQP_READ_THIS_PROP_BOOL("passive")) {
        flags |= AMQP_PASSIVE;
    }

    if (PHP_AMQP_READ_THIS_PROP_BOOL("durable")) {
        flags |= AMQP_DURABLE;
    }

    if (PHP_AMQP_READ_THIS_PROP_BOOL("exclusive")) {
        flags |= AMQP_EXCLUSIVE;
    }

    if (PHP_AMQP_READ_THIS_PROP_BOOL("auto_delete")) {
        flags |= AMQP_AUTODELETE;
    }

    RETURN_LONG(flags);
}
/* }}} */


/* {{{ proto AMQPQueue::setFlags(long bitmask)
Set the queue parameters */
static PHP_METHOD(amqp_queue_class, setFlags)
{
    zend_long flags;
    zend_bool flags_is_null = 1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l!", &flags, &flags_is_null) == FAILURE) {
        return;
    }

    /* Set the flags based on the bitmask we were given */
    flags = flags ? flags & PHP_AMQP_QUEUE_FLAGS : flags;

    zend_update_property_bool(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("passive"),
        IS_PASSIVE(flags) TSRMLS_CC
    );
    zend_update_property_bool(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("durable"),
        IS_DURABLE(flags) TSRMLS_CC
    );
    zend_update_property_bool(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("exclusive"),
        IS_EXCLUSIVE(flags) TSRMLS_CC
    );
    zend_update_property_bool(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("auto_delete"),
        IS_AUTODELETE(flags) TSRMLS_CC
    );
}
/* }}} */


/* {{{ proto AMQPQueue::getArgument(string key)
Get the queue argument referenced by key */
static PHP_METHOD(amqp_queue_class, getArgument)
{
    zval rv;
    zval *tmp = NULL;
    char *key;
    size_t key_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &key_len) == FAILURE) {
        return;
    }

    if ((tmp = zend_hash_str_find(PHP_AMQP_READ_THIS_PROP_ARR("arguments"), key, key_len)) == NULL) {
        RETURN_NULL();
    }

    RETURN_ZVAL(tmp, 1, 0);
}
/* }}} */

/* {{{ proto AMQPQueue::hasArgument(string key) */
static PHP_METHOD(amqp_queue_class, hasArgument)
{
    zval rv;

    char *key;
    size_t key_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &key_len) == FAILURE) {
        return;
    }

    RETURN_BOOL(zend_hash_str_find(PHP_AMQP_READ_THIS_PROP_ARR("arguments"), key, key_len) != NULL);
}
/* }}} */


/* {{{ proto AMQPQueue::getArguments
Get the queue arguments */
static PHP_METHOD(amqp_queue_class, getArguments)
{
    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("arguments");
}
/* }}} */

/* {{{ proto AMQPQueue::setArguments(array args)
Overwrite all queue arguments with given args */
static PHP_METHOD(amqp_queue_class, setArguments)
{
    zval *zvalArguments;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a/", &zvalArguments) == FAILURE) {
        return;
    }

    zend_update_property(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("arguments"), zvalArguments TSRMLS_CC);
}
/* }}} */


/* {{{ proto AMQPQueue::setArgument(key, value)
Get the queue name */
static PHP_METHOD(amqp_queue_class, setArgument)
{
    zval rv;

    char *key = NULL;
    size_t key_len = 0;
    zval *value = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz", &key, &key_len, &value) == FAILURE) {
        return;
    }

    switch (Z_TYPE_P(value)) {
        case IS_NULL:
            zend_hash_str_del_ind(PHP_AMQP_READ_THIS_PROP_ARR("arguments"), key, key_len);
            break;
        case IS_TRUE:
        case IS_FALSE:
        case IS_LONG:
        case IS_DOUBLE:
        case IS_STRING:
            zend_hash_str_add(PHP_AMQP_READ_THIS_PROP_ARR("arguments"), key, key_len, value);
            Z_TRY_ADDREF_P(value);
            break;
        default:
            zend_throw_exception(
                amqp_exchange_exception_class_entry,
                "The value parameter must be of type NULL, int, double or string.",
                0 TSRMLS_CC
            );
            return;
    }
}
/* }}} */


/* {{{ proto int AMQPQueue::declareQueue();
declare queue
*/
static PHP_METHOD(amqp_queue_class, declareQueue)
{
    zval rv;

    amqp_channel_resource *channel_resource;

    char *name;
    amqp_table_t *arguments;
    zend_long message_count;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(PHP_AMQP_READ_THIS_PROP("channel"));
    PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not declare queue.");

    arguments = php_amqp_type_convert_zval_to_amqp_table(PHP_AMQP_READ_THIS_PROP("arguments") TSRMLS_CC);

    amqp_queue_declare_ok_t *r = amqp_queue_declare(
        channel_resource->connection_resource->connection_state,
        channel_resource->channel_id,
        amqp_cstring_bytes(PHP_AMQP_READ_THIS_PROP_STR("name")),
        PHP_AMQP_READ_THIS_PROP_BOOL("passive"),
        PHP_AMQP_READ_THIS_PROP_BOOL("durable"),
        PHP_AMQP_READ_THIS_PROP_BOOL("exclusive"),
        PHP_AMQP_READ_THIS_PROP_BOOL("auto_delete"),
        *arguments
    );

    php_amqp_type_free_amqp_table(arguments);

    if (!r) {
        amqp_rpc_reply_t res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

        php_amqp_error(
            res,
            &PHP_AMQP_G(error_message),
            channel_resource->connection_resource,
            channel_resource TSRMLS_CC
        );

        php_amqp_zend_throw_exception(
            res,
            amqp_queue_exception_class_entry,
            PHP_AMQP_G(error_message),
            PHP_AMQP_G(error_code) TSRMLS_CC
        );
        php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
        return;
    }

    message_count = r->message_count;

    /* Set the queue name, in case it is an autogenerated queue name */
    name = php_amqp_type_amqp_bytes_to_char(r->queue);
    zend_update_property_string(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("name"), name TSRMLS_CC);
    efree(name);

    php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

    RETURN_LONG(message_count);
}
/* }}} */


/* {{{ proto int AMQPQueue::bind(string exchangeName, [string routingKey, array arguments]);
bind queue to exchange by routing key
*/
static PHP_METHOD(amqp_queue_class, bind)
{
    zval rv;

    zval *zvalArguments = NULL;

    amqp_channel_resource *channel_resource;

    char *exchange_name;
    size_t exchange_name_len;
    char *keyname = NULL;
    size_t keyname_len = 0;

    amqp_table_t *arguments = NULL;

    if (zend_parse_parameters(
            ZEND_NUM_ARGS() TSRMLS_CC,
            "s|s!a",
            &exchange_name,
            &exchange_name_len,
            &keyname,
            &keyname_len,
            &zvalArguments
        ) == FAILURE) {
        return;
    }

    channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(PHP_AMQP_READ_THIS_PROP("channel"));
    PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not bind queue.");

    if (zvalArguments) {
        arguments = php_amqp_type_convert_zval_to_amqp_table(zvalArguments TSRMLS_CC);
    }

    amqp_queue_bind(
        channel_resource->connection_resource->connection_state,
        channel_resource->channel_id,
        amqp_cstring_bytes(PHP_AMQP_READ_THIS_PROP_STR("name")),
        (exchange_name_len > 0 ? amqp_cstring_bytes(exchange_name) : amqp_empty_bytes),
        (keyname_len > 0 ? amqp_cstring_bytes(keyname) : amqp_empty_bytes),
        (arguments ? *arguments : amqp_empty_table)
    );

    if (arguments) {
        php_amqp_type_free_amqp_table(arguments);
    }

    amqp_rpc_reply_t res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

    if (PHP_AMQP_MAYBE_ERROR(res, channel_resource)) {
        php_amqp_zend_throw_exception_short(res, amqp_queue_exception_class_entry TSRMLS_CC);
        php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
        return;
    }

    php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
}
/* }}} */


/* {{{ proto int AMQPQueue::get([bit flags=AMQP_NOPARAM]);
read messages from queue
return array (messages)
*/
static PHP_METHOD(amqp_queue_class, get)
{
    zval rv;

    amqp_channel_resource *channel_resource;

    zval message;

    zend_long flags = INI_INT("amqp.auto_ack") ? AMQP_AUTOACK : AMQP_NOPARAM;

    /* Parse out the method parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &flags) == FAILURE) {
        return;
    }

    channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(PHP_AMQP_READ_THIS_PROP("channel"));
    PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not get messages from queue.");

    amqp_rpc_reply_t res = amqp_basic_get(
        channel_resource->connection_resource->connection_state,
        channel_resource->channel_id,
        amqp_cstring_bytes(PHP_AMQP_READ_THIS_PROP_STR("name")),
        (AMQP_AUTOACK & flags) ? 1 : 0
    );

    if (PHP_AMQP_MAYBE_ERROR(res, channel_resource)) {
        php_amqp_zend_throw_exception_short(res, amqp_queue_exception_class_entry TSRMLS_CC);
        php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
        return;
    }

    if (AMQP_BASIC_GET_EMPTY_METHOD == res.reply.id) {
        php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
        RETURN_NULL();
    }

    assert(AMQP_BASIC_GET_OK_METHOD == res.reply.id);

    /* Fill the envelope from response */
    amqp_basic_get_ok_t *get_ok_method = res.reply.decoded;

    amqp_envelope_t envelope;

    envelope.channel = channel_resource->channel_id;
    envelope.consumer_tag = amqp_empty_bytes;
    envelope.delivery_tag = get_ok_method->delivery_tag;
    envelope.redelivered = get_ok_method->redelivered;
    envelope.exchange = amqp_bytes_malloc_dup(get_ok_method->exchange);
    envelope.routing_key = amqp_bytes_malloc_dup(get_ok_method->routing_key);

    php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

    res = amqp_read_message(
        channel_resource->connection_resource->connection_state,
        channel_resource->channel_id,
        &envelope.message,
        0
    );

    if (PHP_AMQP_MAYBE_ERROR(res, channel_resource)) {
        php_amqp_zend_throw_exception_short(res, amqp_queue_exception_class_entry TSRMLS_CC);
        php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
        amqp_destroy_envelope(&envelope);
        return;
    }

    ZVAL_UNDEF(&message);

    convert_amqp_envelope_to_zval(&envelope, &message TSRMLS_CC);

    php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
    amqp_destroy_envelope(&envelope);

    RETVAL_ZVAL(&message, 1, 0);
    zval_ptr_dtor(&message);
}
/* }}} */


/* {{{ proto array AMQPQueue::consume([callback, flags = <bitmask>, consumer_tag]);
consume the message
*/
static PHP_METHOD(amqp_queue_class, consume)
{
    zval rv;

    zval current_channel_zv;

    zval *current_queue_zv = NULL;

    amqp_channel_resource *channel_resource;
    amqp_channel_resource *current_channel_resource;

    zend_fcall_info fci = empty_fcall_info;
    zend_fcall_info_cache fci_cache = empty_fcall_info_cache;

    amqp_table_t *arguments;

    char *consumer_tag = NULL;
    size_t consumer_tag_len = 0;
    zend_long flags = INI_INT("amqp.auto_ack") ? AMQP_AUTOACK : AMQP_NOPARAM;

    if (zend_parse_parameters(
            ZEND_NUM_ARGS() TSRMLS_CC,
            "|f!ls",
            &fci,
            &fci_cache,
            &flags,
            &consumer_tag,
            &consumer_tag_len
        ) == FAILURE) {
        return;
    }

    zval *channel_zv = PHP_AMQP_READ_THIS_PROP("channel");
    zval *consumers = zend_read_property(
        amqp_channel_class_entry,
        PHP_AMQP_COMPAT_OBJ_P(channel_zv),
        ZEND_STRL("consumers"),
        0,
        &rv TSRMLS_CC
    );

    if (IS_ARRAY != Z_TYPE_P(consumers)) {
        zend_throw_exception(
            amqp_queue_exception_class_entry,
            "Invalid channel consumers, forgot to call channel constructor?",
            0 TSRMLS_CC
        );
        return;
    }

    amqp_channel_object *channel = PHP_AMQP_GET_CHANNEL(channel_zv);

    channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(channel_zv);
    PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not get channel.");

    if (!(AMQP_JUST_CONSUME & flags)) {
        /* Setup the consume */
        arguments = php_amqp_type_convert_zval_to_amqp_table(PHP_AMQP_READ_THIS_PROP("arguments") TSRMLS_CC);

        amqp_basic_consume_ok_t *r = amqp_basic_consume(
            channel_resource->connection_resource->connection_state,
            channel_resource->channel_id,
            amqp_cstring_bytes(PHP_AMQP_READ_THIS_PROP_STR("name")),
            (consumer_tag_len > 0 ? amqp_cstring_bytes(consumer_tag) : amqp_empty_bytes), /* Consumer tag */
            (AMQP_NOLOCAL & flags) ? 1 : 0,                                               /* No local */
            (AMQP_AUTOACK & flags) ? 1 : 0,                                               /* no_ack, aka AUTOACK */
            PHP_AMQP_READ_THIS_PROP_BOOL("exclusive"),
            *arguments
        );

        php_amqp_type_free_amqp_table(arguments);

        if (!r) {
            amqp_rpc_reply_t res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

            php_amqp_error(
                res,
                &PHP_AMQP_G(error_message),
                channel_resource->connection_resource,
                channel_resource TSRMLS_CC
            );

            zend_throw_exception(
                amqp_queue_exception_class_entry,
                PHP_AMQP_G(error_message),
                PHP_AMQP_G(error_code) TSRMLS_CC
            );
            php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
            return;
        }

        char *key;
        key = estrndup((char *) r->consumer_tag.bytes, (unsigned) r->consumer_tag.len);

        if (zend_hash_str_find(Z_ARRVAL_P(consumers), key, r->consumer_tag.len) != NULL) {
            // This should never happen as AMQP server guarantees that consumer tag is unique within channel
            zend_throw_exception(amqp_exception_class_entry, "Duplicate consumer tag", 0 TSRMLS_CC);
            efree(key);
            return;
        }

        zval tmp;

        ZVAL_UNDEF(&tmp);
        ZVAL_COPY(&tmp, getThis());

        zend_hash_str_add(Z_ARRVAL_P(consumers), key, r->consumer_tag.len, &tmp);
        efree(key);

        /* Set the consumer tag name, in case it is an autogenerated consumer tag name */
        zend_update_property_stringl(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(getThis()),
            ZEND_STRL("consumer_tag"),
            (const char *) r->consumer_tag.bytes,
            (size_t) r->consumer_tag.len TSRMLS_CC
        );
    }

    if (!ZEND_FCI_INITIALIZED(fci)) {
        /* Callback not set, we have nothing to do - real consuming may happens later */
        return;
    }

    struct timeval tv = {0};
    struct timeval *tv_ptr = &tv;

    double read_timeout = PHP_AMQP_READ_OBJ_PROP_DOUBLE(
        amqp_connection_class_entry,
        PHP_AMQP_READ_THIS_PROP("connection"),
        "read_timeout"
    );

    if (read_timeout > 0) {
        tv.tv_sec = (long int) read_timeout;
        tv.tv_usec = (long int) ((read_timeout - tv.tv_sec) * 1000000);
    } else {
        tv_ptr = NULL;
    }

    while (1) {
        /* Initialize the message */
        zval message;

        amqp_envelope_t envelope;

        php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

        amqp_rpc_reply_t res =
            amqp_consume_message(channel_resource->connection_resource->connection_state, &envelope, tv_ptr, 0);

        if (AMQP_RESPONSE_LIBRARY_EXCEPTION == res.reply_type && AMQP_STATUS_TIMEOUT == res.library_error) {
            zend_throw_exception(amqp_queue_exception_class_entry, "Consumer timeout exceed", 0 TSRMLS_CC);

            amqp_destroy_envelope(&envelope);
            php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
            return;
        }

        if (PHP_AMQP_MAYBE_ERROR_RECOVERABLE(res, channel_resource)) {

            if (PHP_AMQP_IS_ERROR_RECOVERABLE(res, channel_resource, channel)) {
                /* In case no message was received, continue the loop */
                amqp_destroy_envelope(&envelope);

                continue;
            } else {
                /* Mark connection resource as closed to prevent sending any further requests */
                channel_resource->connection_resource->is_connected = '\0';

                /* Close connection with all its channels */
                php_amqp_disconnect_force(channel_resource->connection_resource TSRMLS_CC);
            }

            php_amqp_zend_throw_exception_short(res, amqp_queue_exception_class_entry TSRMLS_CC);

            amqp_destroy_envelope(&envelope);
            php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

            return;
        }

        ZVAL_UNDEF(&message);
        convert_amqp_envelope_to_zval(&envelope, &message TSRMLS_CC);

        current_channel_resource = channel_resource->connection_resource->slots[envelope.channel - 1];

        if (!current_channel_resource) {
            // This should never happen, but just in case
            php_amqp_zend_throw_exception(
                res,
                amqp_queue_exception_class_entry,
                "Orphaned channel. Please, report a bug.",
                0 TSRMLS_CC
            );
            amqp_destroy_envelope(&envelope);
            break;
        }

        ZVAL_UNDEF(&current_channel_zv);
        ZVAL_OBJ(&current_channel_zv, &current_channel_resource->parent->zo);

        consumers = zend_read_property(
            amqp_channel_class_entry,
            PHP_AMQP_COMPAT_OBJ_P(&current_channel_zv),
            ZEND_STRL("consumers"),
            0,
            &rv TSRMLS_CC
        );

        if (IS_ARRAY != Z_TYPE_P(consumers)) {
            zend_throw_exception(
                amqp_queue_exception_class_entry,
                "Invalid channel consumers, forgot to call channel constructor?",
                0 TSRMLS_CC
            );
            amqp_destroy_envelope(&envelope);
            break;
        }

        char *key;
        key = estrndup((char *) envelope.consumer_tag.bytes, (unsigned) envelope.consumer_tag.len);

        if ((current_queue_zv = zend_hash_str_find(Z_ARRVAL_P(consumers), key, envelope.consumer_tag.len)) == NULL) {
            zval exception;
            ZVAL_UNDEF(&exception);
            object_init_ex(&exception, amqp_envelope_exception_class_entry);
            zend_update_property_string(
                zend_exception_get_default(TSRMLS_C),
                PHP_AMQP_COMPAT_OBJ_P(&exception),
                ZEND_STRL("message"),
                "Orphaned envelope" TSRMLS_CC
            );
            zend_update_property(
                amqp_envelope_exception_class_entry,
                PHP_AMQP_COMPAT_OBJ_P(&exception),
                ZEND_STRL("envelope"),
                &message TSRMLS_CC
            );

            zend_throw_exception_object(&exception TSRMLS_CC);

            zval_ptr_dtor(&message);

            amqp_destroy_envelope(&envelope);
            efree(key);
            break;
        }

        efree(key);
        amqp_destroy_envelope(&envelope);

        /* Make the callback */
        zval params;
        zval retval;

        /* Build the parameter array */
        ZVAL_UNDEF(&params);
        array_init(&params);

        /* Dump it into the params array */
        add_index_zval(&params, 0, &message);
        Z_ADDREF_P(&message);

        /* Add a pointer to the queue: */
        add_index_zval(&params, 1, current_queue_zv);
        Z_ADDREF_P(current_queue_zv);


        /* Convert everything to be callable */
        zend_fcall_info_args(&fci, &params TSRMLS_CC);
        /* Initialize the return value pointer */

        fci.retval = &retval;

        /* Call the function, and track the return value */
        if (zend_call_function(&fci, &fci_cache TSRMLS_CC) == SUCCESS && fci.retval) {
            RETVAL_ZVAL(&retval, 1, 1);
        }

        /* Clean up our mess */
        zend_fcall_info_args_clear(&fci, 1);
        zval_ptr_dtor(&params);
        zval_ptr_dtor(&message);

        /* Check if user land function wants to bail */
        if (EG(exception) || Z_TYPE_P(return_value) == IS_FALSE) {
            break;
        }
    }

    php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
    return;
}
/* }}} */


/* {{{ proto int AMQPQueue::ack(long deliveryTag, [bit flags=AMQP_NOPARAM]);
	acknowledge the message
*/
static PHP_METHOD(amqp_queue_class, ack)
{
    zval rv;

    amqp_channel_resource *channel_resource;

    zend_long deliveryTag = 0;
    zend_long flags = AMQP_NOPARAM;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|l", &deliveryTag, &flags) == FAILURE) {
        return;
    }

    channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(PHP_AMQP_READ_THIS_PROP("channel"));
    PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not ack message.");

    /* NOTE: basic.ack is asynchronous and thus will not indicate failure if something goes wrong on the broker */
    int status = amqp_basic_ack(
        channel_resource->connection_resource->connection_state,
        channel_resource->channel_id,
        (uint64_t) deliveryTag,
        (AMQP_MULTIPLE & flags) ? 1 : 0
    );

    if (status != AMQP_STATUS_OK) {
        /* Emulate library error */
        amqp_rpc_reply_t res;
        res.reply_type = AMQP_RESPONSE_LIBRARY_EXCEPTION;
        res.library_error = status;

        php_amqp_error(
            res,
            &PHP_AMQP_G(error_message),
            channel_resource->connection_resource,
            channel_resource TSRMLS_CC
        );

        php_amqp_zend_throw_exception(
            res,
            amqp_queue_exception_class_entry,
            PHP_AMQP_G(error_message),
            PHP_AMQP_G(error_code) TSRMLS_CC
        );
        php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
        return;
    }
}
/* }}} */


/* {{{ proto int AMQPQueue::nack(long deliveryTag, [bit flags=AMQP_NOPARAM]);
	acknowledge the message
*/
static PHP_METHOD(amqp_queue_class, nack)
{
    zval rv;

    amqp_channel_resource *channel_resource;

    zend_long deliveryTag = 0;
    zend_long flags = AMQP_NOPARAM;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|l", &deliveryTag, &flags) == FAILURE) {
        return;
    }

    channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(PHP_AMQP_READ_THIS_PROP("channel"));
    PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not nack message.");

    /* NOTE: basic.nack is asynchronous and thus will not indicate failure if something goes wrong on the broker */
    int status = amqp_basic_nack(
        channel_resource->connection_resource->connection_state,
        channel_resource->channel_id,
        (uint64_t) deliveryTag,
        (AMQP_MULTIPLE & flags) ? 1 : 0,
        (AMQP_REQUEUE & flags) ? 1 : 0
    );

    if (status != AMQP_STATUS_OK) {
        /* Emulate library error */
        amqp_rpc_reply_t res;
        res.reply_type = AMQP_RESPONSE_LIBRARY_EXCEPTION;
        res.library_error = status;

        php_amqp_error(
            res,
            &PHP_AMQP_G(error_message),
            channel_resource->connection_resource,
            channel_resource TSRMLS_CC
        );

        php_amqp_zend_throw_exception(
            res,
            amqp_queue_exception_class_entry,
            PHP_AMQP_G(error_message),
            PHP_AMQP_G(error_code) TSRMLS_CC
        );
        php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
        return;
    }
}
/* }}} */


/* {{{ proto int AMQPQueue::reject(long deliveryTag, [bit flags=AMQP_NOPARAM]);
	acknowledge the message
*/
static PHP_METHOD(amqp_queue_class, reject)
{
    zval rv;

    amqp_channel_resource *channel_resource;

    zend_long deliveryTag = 0;
    zend_long flags = AMQP_NOPARAM;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|l", &deliveryTag, &flags) == FAILURE) {
        return;
    }

    channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(PHP_AMQP_READ_THIS_PROP("channel"));
    PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not reject message.");

    /* NOTE: basic.reject is asynchronous and thus will not indicate failure if something goes wrong on the broker */
    int status = amqp_basic_reject(
        channel_resource->connection_resource->connection_state,
        channel_resource->channel_id,
        (uint64_t) deliveryTag,
        (AMQP_REQUEUE & flags) ? 1 : 0
    );

    if (status != AMQP_STATUS_OK) {
        /* Emulate library error */
        amqp_rpc_reply_t res;
        res.reply_type = AMQP_RESPONSE_LIBRARY_EXCEPTION;
        res.library_error = status;

        php_amqp_error(
            res,
            &PHP_AMQP_G(error_message),
            channel_resource->connection_resource,
            channel_resource TSRMLS_CC
        );

        php_amqp_zend_throw_exception(
            res,
            amqp_queue_exception_class_entry,
            PHP_AMQP_G(error_message),
            PHP_AMQP_G(error_code) TSRMLS_CC
        );
        php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
        return;
    }
}
/* }}} */


/* {{{ proto int AMQPQueue::purge();
purge queue
*/
static PHP_METHOD(amqp_queue_class, purge)
{
    zval rv;

    amqp_channel_resource *channel_resource;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(PHP_AMQP_READ_THIS_PROP("channel"));
    PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not purge queue.");

    amqp_queue_purge_ok_t *r = amqp_queue_purge(
        channel_resource->connection_resource->connection_state,
        channel_resource->channel_id,
        amqp_cstring_bytes(PHP_AMQP_READ_THIS_PROP_STR("name"))
    );

    if (!r) {
        amqp_rpc_reply_t res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

        php_amqp_error(
            res,
            &PHP_AMQP_G(error_message),
            channel_resource->connection_resource,
            channel_resource TSRMLS_CC
        );

        php_amqp_zend_throw_exception(
            res,
            amqp_queue_exception_class_entry,
            PHP_AMQP_G(error_message),
            PHP_AMQP_G(error_code) TSRMLS_CC
        );
        php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
        return;
    }

    php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

    RETURN_LONG(r->message_count);
}
/* }}} */


/* {{{ proto int AMQPQueue::cancel([string consumer_tag]);
cancel queue to consumer
*/
static PHP_METHOD(amqp_queue_class, cancel)
{
    zval rv;

    amqp_channel_resource *channel_resource;

    char *consumer_tag = NULL;
    size_t consumer_tag_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &consumer_tag, &consumer_tag_len) == FAILURE) {
        return;
    }

    zval *channel_zv = PHP_AMQP_READ_THIS_PROP("channel");
    zval *consumers = zend_read_property(
        amqp_channel_class_entry,
        PHP_AMQP_COMPAT_OBJ_P(channel_zv),
        ZEND_STRL("consumers"),
        0,
        &rv TSRMLS_CC
    );
    zend_bool previous_consumer_tag_exists =
        (zend_bool) (IS_STRING == Z_TYPE_P(PHP_AMQP_READ_THIS_PROP("consumer_tag")));

    if (IS_ARRAY != Z_TYPE_P(consumers)) {
        zend_throw_exception(
            amqp_queue_exception_class_entry,
            "Invalid channel consumers, forgot to call channel constructor?",
            0 TSRMLS_CC
        );
        return;
    }

    channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(channel_zv);
    PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not cancel queue.");

    if (!consumer_tag_len && (!previous_consumer_tag_exists || !PHP_AMQP_READ_THIS_PROP_STRLEN("consumer_tag"))) {
        return;
    }

    amqp_basic_cancel_ok_t *r = amqp_basic_cancel(
        channel_resource->connection_resource->connection_state,
        channel_resource->channel_id,
        consumer_tag_len > 0 ? amqp_cstring_bytes(consumer_tag)
                             : amqp_cstring_bytes(PHP_AMQP_READ_THIS_PROP_STR("consumer_tag"))
    );

    if (!r) {
        amqp_rpc_reply_t res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

        php_amqp_error(
            res,
            &PHP_AMQP_G(error_message),
            channel_resource->connection_resource,
            channel_resource TSRMLS_CC
        );

        php_amqp_zend_throw_exception(
            res,
            amqp_queue_exception_class_entry,
            PHP_AMQP_G(error_message),
            PHP_AMQP_G(error_code) TSRMLS_CC
        );
        php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
        return;
    }

    if (!consumer_tag_len ||
        (previous_consumer_tag_exists && strcmp(consumer_tag, PHP_AMQP_READ_THIS_PROP_STR("consumer_tag")) == 0)) {
        zend_update_property_null(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("consumer_tag") TSRMLS_CC);
    }

    zend_hash_str_del_ind(Z_ARRVAL_P(consumers), r->consumer_tag.bytes, r->consumer_tag.len);

    php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
}
/* }}} */


/* {{{ proto int AMQPQueue::unbind(string exchangeName, [string routingKey, array arguments]);
unbind queue from exchange
*/
static PHP_METHOD(amqp_queue_class, unbind)
{
    zval rv;

    zval *zvalArguments = NULL;
    amqp_channel_resource *channel_resource;

    char *exchange_name;
    size_t exchange_name_len;
    char *keyname = NULL;
    size_t keyname_len = 0;

    amqp_table_t *arguments = NULL;

    if (zend_parse_parameters(
            ZEND_NUM_ARGS() TSRMLS_CC,
            "s|sa",
            &exchange_name,
            &exchange_name_len,
            &keyname,
            &keyname_len,
            &zvalArguments
        ) == FAILURE) {
        return;
    }

    channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(PHP_AMQP_READ_THIS_PROP("channel"));
    PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not unbind queue.");

    if (zvalArguments) {
        arguments = php_amqp_type_convert_zval_to_amqp_table(zvalArguments TSRMLS_CC);
    }

    amqp_queue_unbind(
        channel_resource->connection_resource->connection_state,
        channel_resource->channel_id,
        amqp_cstring_bytes(PHP_AMQP_READ_THIS_PROP_STR("name")),
        (exchange_name_len > 0 ? amqp_cstring_bytes(exchange_name) : amqp_empty_bytes),
        (keyname_len > 0 ? amqp_cstring_bytes(keyname) : amqp_empty_bytes),
        (arguments ? *arguments : amqp_empty_table)
    );

    if (arguments) {
        php_amqp_type_free_amqp_table(arguments);
    }

    amqp_rpc_reply_t res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

    if (PHP_AMQP_MAYBE_ERROR(res, channel_resource)) {
        php_amqp_zend_throw_exception_short(res, amqp_queue_exception_class_entry TSRMLS_CC);
        php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
        return;
    }

    php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
}
/* }}} */


/* {{{ proto int AMQPQueue::delete([long flags = AMQP_NOPARAM]]);
delete queue and return the number of messages deleted in it
*/
static PHP_METHOD(amqp_queue_class, delete)
{
    zval rv;

    amqp_channel_resource *channel_resource;

    zend_long flags = AMQP_NOPARAM;

    zend_long message_count;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &flags) == FAILURE) {
        return;
    }

    channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(PHP_AMQP_READ_THIS_PROP("channel"));
    PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not delete queue.");

    amqp_queue_delete_ok_t *r = amqp_queue_delete(
        channel_resource->connection_resource->connection_state,
        channel_resource->channel_id,
        amqp_cstring_bytes(PHP_AMQP_READ_THIS_PROP_STR("name")),
        (AMQP_IFUNUSED & flags) ? 1 : 0,
        (AMQP_IFEMPTY & flags) ? 1 : 0
    );

    if (!r) {
        amqp_rpc_reply_t res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

        php_amqp_error(
            res,
            &PHP_AMQP_G(error_message),
            channel_resource->connection_resource,
            channel_resource TSRMLS_CC
        );

        php_amqp_zend_throw_exception(
            res,
            amqp_queue_exception_class_entry,
            PHP_AMQP_G(error_message),
            PHP_AMQP_G(error_code) TSRMLS_CC
        );
        php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
        return;
    }

    message_count = r->message_count;

    php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

    RETURN_LONG(message_count);
}
/* }}} */

/* {{{ proto AMQPChannel::getChannel()
Get the AMQPChannel object in use */
static PHP_METHOD(amqp_queue_class, getChannel)
{
    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("channel");
}
/* }}} */

/* {{{ proto AMQPChannel::getConnection()
Get the AMQPConnection object in use */
static PHP_METHOD(amqp_queue_class, getConnection)
{
    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("connection");
}
/* }}} */

/* {{{ proto string AMQPChannel::getConsumerTag()
Get latest consumer tag*/
static PHP_METHOD(amqp_queue_class, getConsumerTag)
{
    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("consumer_tag");
}

/* }}} */
/* amqp_queue_class ARG_INFO definition */
ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_queue_class__construct, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
    ZEND_ARG_OBJ_INFO(0, channel, AMQPChannel, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_queue_class_getName, ZEND_SEND_BY_VAL, 0, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_queue_class_setName, ZEND_SEND_BY_VAL, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_queue_class_getFlags, ZEND_SEND_BY_VAL, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_queue_class_setFlags, ZEND_SEND_BY_VAL, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_queue_class_getArgument, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
    ZEND_ARG_TYPE_INFO(0, argumentName, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_queue_class_getArguments, ZEND_SEND_BY_VAL, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_queue_class_setArgument, ZEND_SEND_BY_VAL, 2, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, argumentName, IS_STRING, 0)
    ZEND_ARG_INFO(0, argumentValue)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_queue_class_hasArgument, ZEND_SEND_BY_VAL, 1, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO(0, argumentName, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_queue_class_setArguments, ZEND_SEND_BY_VAL, 1, IS_VOID, 0)
    ZEND_ARG_ARRAY_INFO(0, arguments, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_queue_class_declareQueue, ZEND_SEND_BY_VAL, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_queue_class_bind, ZEND_RETURN_VALUE, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, exchangeName, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, routingKey, IS_STRING, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, arguments, IS_ARRAY, 0, "[]")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_amqp_queue_class_get, ZEND_SEND_BY_VAL, 0, AMQPEnvelope, 1)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "AMQP_NOPARAM")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_queue_class_consume, ZEND_SEND_BY_VAL, 0, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, callback, IS_CALLABLE, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "AMQP_NOPARAM")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, consumerTag, IS_STRING, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_queue_class_ack, ZEND_SEND_BY_VAL, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, deliveryTag, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "AMQP_NOPARAM")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_queue_class_nack, ZEND_SEND_BY_VAL, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, deliveryTag, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "AMQP_NOPARAM")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_queue_class_reject, ZEND_SEND_BY_VAL, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, deliveryTag, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "AMQP_NOPARAM")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_queue_class_purge, ZEND_SEND_BY_VAL, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_queue_class_cancel, ZEND_SEND_BY_VAL, 0, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, consumerTag, IS_STRING, 0, "\"\"")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_queue_class_unbind, ZEND_RETURN_VALUE, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, exchangeName, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, routingKey, IS_STRING, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, arguments, IS_ARRAY, 0, "[]")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_queue_class_delete, ZEND_SEND_BY_VAL, 0, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "AMQP_NOPARAM")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO(arginfo_amqp_queue_class_getChannel, AMQPChannel, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO(arginfo_amqp_queue_class_getConnection, AMQPConnection, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_queue_class_getConsumerTag, ZEND_SEND_BY_VAL, 0, IS_STRING, 1)
ZEND_END_ARG_INFO()

zend_function_entry amqp_queue_class_functions[] = {
    PHP_ME(amqp_queue_class, __construct,		arginfo_amqp_queue_class__construct,		ZEND_ACC_PUBLIC)

    PHP_ME(amqp_queue_class, getName,			arginfo_amqp_queue_class_getName,			ZEND_ACC_PUBLIC)
    PHP_ME(amqp_queue_class, setName,			arginfo_amqp_queue_class_setName,			ZEND_ACC_PUBLIC)

    PHP_ME(amqp_queue_class, getFlags,			arginfo_amqp_queue_class_getFlags,			ZEND_ACC_PUBLIC)
    PHP_ME(amqp_queue_class, setFlags,			arginfo_amqp_queue_class_setFlags,			ZEND_ACC_PUBLIC)

    PHP_ME(amqp_queue_class, getArgument,		arginfo_amqp_queue_class_getArgument,		ZEND_ACC_PUBLIC)
    PHP_ME(amqp_queue_class, getArguments,		arginfo_amqp_queue_class_getArguments,		ZEND_ACC_PUBLIC)
    PHP_ME(amqp_queue_class, setArgument,		arginfo_amqp_queue_class_setArgument,		ZEND_ACC_PUBLIC)
    PHP_ME(amqp_queue_class, setArguments,		arginfo_amqp_queue_class_setArguments,		ZEND_ACC_PUBLIC)
    PHP_ME(amqp_queue_class, hasArgument,		arginfo_amqp_queue_class_hasArgument,		ZEND_ACC_PUBLIC)

    PHP_ME(amqp_queue_class, declareQueue,		arginfo_amqp_queue_class_declareQueue,			ZEND_ACC_PUBLIC)
    PHP_MALIAS(amqp_queue_class, declare, declareQueue,		arginfo_amqp_queue_class_declareQueue,			ZEND_ACC_PUBLIC)
    PHP_ME(amqp_queue_class, bind,				arginfo_amqp_queue_class_bind,				ZEND_ACC_PUBLIC)

    PHP_ME(amqp_queue_class, get,				arginfo_amqp_queue_class_get,				ZEND_ACC_PUBLIC)
    PHP_ME(amqp_queue_class, consume,			arginfo_amqp_queue_class_consume,			ZEND_ACC_PUBLIC)
    PHP_ME(amqp_queue_class, ack,				arginfo_amqp_queue_class_ack,				ZEND_ACC_PUBLIC)
    PHP_ME(amqp_queue_class, nack,				arginfo_amqp_queue_class_nack,				ZEND_ACC_PUBLIC)
    PHP_ME(amqp_queue_class, reject,			arginfo_amqp_queue_class_reject,			ZEND_ACC_PUBLIC)
    PHP_ME(amqp_queue_class, purge,				arginfo_amqp_queue_class_purge,				ZEND_ACC_PUBLIC)

    PHP_ME(amqp_queue_class, cancel,			arginfo_amqp_queue_class_cancel,			ZEND_ACC_PUBLIC)
    PHP_ME(amqp_queue_class, delete,			arginfo_amqp_queue_class_delete,			ZEND_ACC_PUBLIC)
    PHP_ME(amqp_queue_class, unbind,			arginfo_amqp_queue_class_unbind,			ZEND_ACC_PUBLIC)

    PHP_ME(amqp_queue_class, getChannel,		arginfo_amqp_queue_class_getChannel,		ZEND_ACC_PUBLIC)
    PHP_ME(amqp_queue_class, getConnection,		arginfo_amqp_queue_class_getConnection,		ZEND_ACC_PUBLIC)
    PHP_ME(amqp_queue_class, getConsumerTag,	arginfo_amqp_queue_class_getConsumerTag,	ZEND_ACC_PUBLIC)

    {NULL, NULL, NULL}
};

PHP_MINIT_FUNCTION(amqp_queue)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "AMQPQueue", amqp_queue_class_functions);
    this_ce = zend_register_internal_class(&ce TSRMLS_CC);

    zend_declare_property_null(this_ce, ZEND_STRL("connection"), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(this_ce, ZEND_STRL("channel"), ZEND_ACC_PRIVATE TSRMLS_CC);

    zend_declare_property_stringl(this_ce, ZEND_STRL("name"), "", 0, ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(this_ce, ZEND_STRL("consumer_tag"), ZEND_ACC_PRIVATE TSRMLS_CC);

    zend_declare_property_bool(this_ce, ZEND_STRL("passive"), 0, ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_bool(this_ce, ZEND_STRL("durable"), 0, ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_bool(this_ce, ZEND_STRL("exclusive"), 0, ZEND_ACC_PRIVATE TSRMLS_CC);
    /* By default, the auto_delete flag should be set */
    zend_declare_property_bool(this_ce, ZEND_STRL("auto_delete"), 1, ZEND_ACC_PRIVATE TSRMLS_CC);


    zend_declare_property_null(this_ce, ZEND_STRL("arguments"), ZEND_ACC_PRIVATE TSRMLS_CC);

    return SUCCESS;
}
