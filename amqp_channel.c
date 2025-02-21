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

#ifdef PHP_WIN32
    #if PHP_VERSION_ID >= 80000
        #include <stdint.h>
    #else
        #include "win32/php_stdint.h"
    #endif
    #include "win32/signal.h"
#else
    #include <stdint.h>
    #include <signal.h>
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
#include "amqp_connection.h"
#include "amqp_methods_handling.h"
#include "amqp_connection_resource.h"
#include "amqp_channel.h"

zend_class_entry *amqp_channel_class_entry;
#define this_ce amqp_channel_class_entry

zend_object_handlers amqp_channel_object_handlers;

void php_amqp_close_channel(amqp_channel_resource *channel_resource, bool throw)
{
    assert(channel_resource != NULL);

    amqp_connection_resource *connection_resource = channel_resource->connection_resource;

    if (connection_resource != NULL) {
        /* First, remove it from active channels table to prevent recursion in case of connection error */
        php_amqp_connection_resource_unregister_channel(connection_resource, channel_resource->channel_id);
    } else {
        channel_resource->is_connected = '\0';
    }

    assert(channel_resource->connection_resource == NULL);

    if (!channel_resource->is_connected) {
        /* Nothing to do more - channel was previously marked as closed, possibly, due to channel-level error */
        return;
    }

    channel_resource->is_connected = '\0';

    if (connection_resource && connection_resource->is_connected && channel_resource->channel_id > 0) {
        assert(connection_resource != NULL);

        amqp_rpc_reply_t close_res =
            amqp_channel_close(connection_resource->connection_state, channel_resource->channel_id, AMQP_REPLY_SUCCESS);

        if (throw && PHP_AMQP_MAYBE_ERROR(close_res, channel_resource, connection_resource)) {
            php_amqp_zend_throw_exception_short(close_res, amqp_channel_exception_class_entry);
            goto err;
        }

        if (close_res.reply_type != AMQP_RESPONSE_NORMAL) {
            goto err;
        }

        amqp_rpc_reply_t reply_res = amqp_get_rpc_reply(connection_resource->connection_state);
        if (throw && PHP_AMQP_MAYBE_ERROR(reply_res, channel_resource, connection_resource)) {
            php_amqp_zend_throw_exception_short(reply_res, amqp_channel_exception_class_entry);
            goto err;
        }

        if (reply_res.reply_type != AMQP_RESPONSE_NORMAL) {
            goto err;
        }

        php_amqp_maybe_release_buffers_on_channel(connection_resource, channel_resource);
        return;

    err:
        // Mark failed slot as used
        connection_resource->used_slots++;
        return;
    }
}

static void php_amqp_destroy_fci(zend_fcall_info *fci)
{
    if (fci->size > 0) {
        zval_ptr_dtor(&fci->function_name);
        if (fci->object != NULL) {
            GC_DELREF(fci->object);
        }
        fci->size = 0;
    }
}

static void php_amqp_duplicate_fci(zend_fcall_info *source)
{
    if (source->size > 0) {

        zval_add_ref(&source->function_name);
        if (source->object != NULL) {
            GC_ADDREF(source->object);
        }
    }
}

static int php_amqp_get_fci_gc_data_count(zend_fcall_info *fci)
{
    int cnt = 0;

    if (fci->size > 0) {
        cnt++;

        if (fci->object != NULL) {
            cnt++;
        }
    }

    return cnt;
}

static zval *php_amqp_get_fci_gc_data(zend_fcall_info *fci, zval *gc_data)
{
    if (ZEND_FCI_INITIALIZED(*fci)) {

        ZVAL_COPY_VALUE(gc_data++, &fci->function_name);

        if (fci->object != NULL) {
            ZVAL_OBJ(gc_data++, fci->object);
        }
    }

    return gc_data;
}

#if PHP_MAJOR_VERSION < 8
static HashTable *amqp_channel_gc(zval *object, zval **table, int *n) /* {{{ */
{
    amqp_channel_object *channel = PHP_AMQP_GET_CHANNEL(object);
#else
static HashTable *amqp_channel_gc(zend_object *object, zval **table, int *n) /* {{{ */
{
    amqp_channel_object *channel = php_amqp_channel_object_fetch(object);
#endif
    int basic_return_cnt = php_amqp_get_fci_gc_data_count(&channel->callbacks.basic_return.fci);
    int basic_ack_cnt = php_amqp_get_fci_gc_data_count(&channel->callbacks.basic_ack.fci);
    int basic_nack_cnt = php_amqp_get_fci_gc_data_count(&channel->callbacks.basic_nack.fci);

    int cnt = basic_return_cnt + basic_ack_cnt + basic_nack_cnt;

    if (cnt > channel->gc_data_count) {
        channel->gc_data_count = cnt;
        channel->gc_data = (zval *) erealloc(channel->gc_data, sizeof(zval) * cnt);
    }

    zval *gc_data = channel->gc_data;

    gc_data = php_amqp_get_fci_gc_data(&channel->callbacks.basic_return.fci, gc_data);
    gc_data = php_amqp_get_fci_gc_data(&channel->callbacks.basic_ack.fci, gc_data);
    php_amqp_get_fci_gc_data(&channel->callbacks.basic_nack.fci, gc_data);

    *table = channel->gc_data;
    *n = cnt;

    return zend_std_get_properties(object);
} /* }}} */

static void php_amqp_clean_callbacks(amqp_channel_callbacks *callbacks)
{
    php_amqp_destroy_fci(&callbacks->basic_return.fci);
    php_amqp_destroy_fci(&callbacks->basic_ack.fci);
    php_amqp_destroy_fci(&callbacks->basic_nack.fci);
}


void amqp_channel_free(zend_object *object)
{
    amqp_channel_object *channel = PHP_AMQP_FETCH_CHANNEL(object);

    if (channel->channel_resource != NULL) {
        php_amqp_close_channel(channel->channel_resource, 0);

        efree(channel->channel_resource);
        channel->channel_resource = NULL;
    }

    if (channel->gc_data) {
        efree(channel->gc_data);
    }

    php_amqp_clean_callbacks(&channel->callbacks);

    zend_object_std_dtor(&channel->zo);
}


zend_object *amqp_channel_ctor(zend_class_entry *ce)
{
    amqp_channel_object *channel =
        (amqp_channel_object *) ecalloc(1, sizeof(amqp_channel_object) + zend_object_properties_size(ce));

    zend_object_std_init(&channel->zo, ce);
    AMQP_OBJECT_PROPERTIES_INIT(channel->zo, ce);

#if PHP_MAJOR_VERSION >= 7
    channel->zo.handlers = &amqp_channel_object_handlers;

    return &channel->zo;
#else
    zend_object *new_value;

    new_value.handle =
        zend_objects_store_put(channel, NULL, (zend_objects_free_object_storage_t) amqp_channel_free, NULL);

    new_value.handlers = zend_get_std_object_handlers();

    return new_value;
#endif
}


/* {{{ proto AMQPChannel::__construct(AMQPConnection obj)
 */
static PHP_METHOD(amqp_channel_class, __construct)
{
    zval rv;

    zval *connection_object = NULL;

    amqp_channel_resource *channel_resource;
    amqp_channel_object *channel;
    amqp_connection_object *connection;

    /* Parse out the method parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "O", &connection_object, amqp_connection_class_entry) == FAILURE) {
        zend_throw_exception(amqp_channel_exception_class_entry, "Parameter must be an instance of AMQPConnection.", 0);
        RETURN_NULL();
    }

    zval consumers;

    ZVAL_UNDEF(&consumers);
    array_init(&consumers);

    zend_update_property(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("consumers"), &consumers);

    zval_ptr_dtor(&consumers);

    channel = PHP_AMQP_GET_CHANNEL(getThis());

    /* Set the prefetch count */
    zend_update_property_long(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("prefetchCount"),
        INI_INT("amqp.prefetch_count")
    );

    /* Set the prefetch size */
    zend_update_property_long(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("prefetchSize"),
        INI_INT("amqp.prefetch_size")
    );

    /* Set the global prefetch count */
    zend_update_property_long(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("globalPrefetchCount"),
        INI_INT("amqp.global_prefetch_count")
    );

    /* Set the global prefetch size */
    zend_update_property_long(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("globalPrefetchSize"),
        INI_INT("amqp.global_prefetch_size")
    );

    /* Pull out and verify the connection */
    connection = PHP_AMQP_GET_CONNECTION(connection_object);
    PHP_AMQP_VERIFY_CONNECTION(connection, "Could not create channel.");

    if (!connection->connection_resource) {
        zend_throw_exception(
            amqp_channel_exception_class_entry,
            "Could not create channel. No connection resource.",
            0
        );
        RETURN_THROWS();
    }

    if (!connection->connection_resource->is_connected) {
        zend_throw_exception(
            amqp_channel_exception_class_entry,
            "Could not create channel. Connection resource is not connected.",
            0
        );
        return;
    }

    zend_update_property(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("connection"), connection_object);

    channel_resource = (amqp_channel_resource *) ecalloc(1, sizeof(amqp_channel_resource));
    channel->channel_resource = channel_resource;
    channel_resource->parent = channel;

    /* Figure out what the next available channel is on this connection */
    channel_resource->channel_id =
        php_amqp_connection_resource_get_available_channel_id(connection->connection_resource);

    /* Check that we got a valid channel */
    if (!channel_resource->channel_id) {
        zend_throw_exception(
            amqp_channel_exception_class_entry,
            "Could not create channel. Connection has no open channel slots remaining.",
            0
        );
        return;
    }

    if (php_amqp_connection_resource_register_channel(
            connection->connection_resource,
            channel_resource,
            channel_resource->channel_id
        ) == FAILURE) {
        zend_throw_exception(
            amqp_channel_exception_class_entry,
            "Could not create channel. Failed to add channel to connection slot.",
            0
        );
    }

    /* Open up the channel for use */
    amqp_channel_open_ok_t *r =
        amqp_channel_open(channel_resource->connection_resource->connection_state, channel_resource->channel_id);


    if (!r) {
        amqp_rpc_reply_t res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

        php_amqp_error(res, &PHP_AMQP_G(error_message), channel_resource->connection_resource, channel_resource);

        php_amqp_zend_throw_exception(
            res,
            amqp_channel_exception_class_entry,
            PHP_AMQP_G(error_message),
            PHP_AMQP_G(error_code)
        );
        php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

        /* Prevent double free, it may happen in case the channel resource was already freed due to some hard error. */
        if (channel_resource->connection_resource) {
            php_amqp_connection_resource_unregister_channel(
                channel_resource->connection_resource,
                channel_resource->channel_id
            );
            channel_resource->channel_id = 0;
        }

        return;
    }

    /* r->channel_id is a 16-bit channel number inside amqp_bytes_t, assertion below will without converting to uint16_t*/
    /* assert (r->channel_id == channel_resource->channel_id);*/
    php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

    channel_resource->is_connected = '\1';

    /* Set the prefetch count: */
    amqp_basic_qos(
        channel_resource->connection_resource->connection_state,
        channel_resource->channel_id,
        0,                                                        /* prefetch window size */
        (uint16_t) PHP_AMQP_READ_THIS_PROP_LONG("prefetchCount"), /* prefetch message count */
        /* NOTE that RabbitMQ has reinterpreted global flag field. See https://www.rabbitmq.com/amqp-0-9-1-reference.html#basic.qos.global for details */
        0 /* global flag */
    );

    amqp_rpc_reply_t res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

    if (PHP_AMQP_MAYBE_ERROR(res, channel_resource, channel_resource->connection_resource)) {
        php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry);
        php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
        return;
    }

    php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

    uint32_t global_prefetch_size = (uint32_t) PHP_AMQP_READ_THIS_PROP_LONG("globalPrefetchSize");
    uint16_t global_prefetch_count = (uint16_t) PHP_AMQP_READ_THIS_PROP_LONG("globalPrefetchCount");

    /* Set the global prefetch settings (ignoring if 0 for backwards compatibility) */
    if (global_prefetch_size != 0 || global_prefetch_count != 0) {
        amqp_basic_qos(
            channel_resource->connection_resource->connection_state,
            channel_resource->channel_id,
            global_prefetch_size,
            global_prefetch_count,
            1
        );

        res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

        if (PHP_AMQP_MAYBE_ERROR(res, channel_resource, channel_resource->connection_resource)) {
            php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry);
            php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
            return;
        }

        php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
    }
}
/* }}} */


/* {{{ proto bool amqp::isConnected()
check amqp channel */
static PHP_METHOD(amqp_channel_class, isConnected)
{
    amqp_channel_resource *channel_resource;

    PHP_AMQP_NOPARAMS()

    channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(getThis());

    RETURN_BOOL(channel_resource && channel_resource->is_connected);
}
/* }}} */

/* {{{ proto bool AMQPChannel::close()
Close amqp channel */
static PHP_METHOD(amqp_channel_class, close)
{
    amqp_channel_resource *channel_resource;

    PHP_AMQP_NOPARAMS()

    channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(getThis());

    if (channel_resource && channel_resource->is_connected) {
        php_amqp_close_channel(channel_resource, 1);
    }
}
/* }}} */

/* {{{ proto bool amqp::getChannelId()
get amqp channel ID */
static PHP_METHOD(amqp_channel_class, getChannelId)
{
    amqp_channel_resource *channel_resource;

    PHP_AMQP_NOPARAMS()

    channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(getThis());

    if (!channel_resource) {
        RETURN_NULL();
    }

    RETURN_LONG(channel_resource->channel_id);
}
/* }}} */

/* {{{ proto bool amqp::setPrefetchCount(long count)
set the number of prefetches */
static PHP_METHOD(amqp_channel_class, setPrefetchCount)
{
    zval rv;

    amqp_channel_resource *channel_resource;
    zend_long prefetch_count;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &prefetch_count) == FAILURE) {
        RETURN_THROWS();
    }

    if (!php_amqp_is_valid_prefetch_count(prefetch_count)) {
        zend_throw_exception_ex(
            amqp_connection_exception_class_entry,
            0,
            "Parameter 'prefetchCount' must be between 0 and %u.",
            PHP_AMQP_MAX_PREFETCH_COUNT
        );
        return;
    }

    channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(getThis());
    PHP_AMQP_VERIFY_CHANNEL_CONNECTION_RESOURCE(channel_resource, "Could not set prefetch count.");
    // TODO: verify that connection is active and resource exists. that is enough

    /* If we are already connected, set the new prefetch count */
    if (channel_resource->is_connected) {
        amqp_basic_qos(
            channel_resource->connection_resource->connection_state,
            channel_resource->channel_id,
            0,
            (uint16_t) prefetch_count,
            0
        );

        amqp_rpc_reply_t res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

        if (PHP_AMQP_MAYBE_ERROR(res, channel_resource, channel_resource->connection_resource)) {
            php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry);
            php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
            return;
        }

        php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

        uint32_t global_prefetch_size = (uint32_t) PHP_AMQP_READ_THIS_PROP_LONG("globalPrefetchSize");
        uint16_t global_prefetch_count = (uint16_t) PHP_AMQP_READ_THIS_PROP_LONG("globalPrefetchCount");

        /* Re-apply current global prefetch settings if set (writing consumer prefetch settings will clear global prefetch settings) */
        if (global_prefetch_size != 0 || global_prefetch_count != 0) {
            amqp_basic_qos(
                channel_resource->connection_resource->connection_state,
                channel_resource->channel_id,
                global_prefetch_size,
                global_prefetch_count,
                1
            );

            res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

            if (PHP_AMQP_MAYBE_ERROR(res, channel_resource, channel_resource->connection_resource)) {
                php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry);
                php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
                return;
            }

            php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
        }
    }

    /* Set the prefetch count - the implication is to disable the size */
    zend_update_property_long(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("prefetchCount"), prefetch_count);
    zend_update_property_long(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("prefetchSize"), 0);
}
/* }}} */

/* {{{ proto long amqp::getPrefetchCount()
get the number of prefetches */
static PHP_METHOD(amqp_channel_class, getPrefetchCount)
{
    zval rv;
    PHP_AMQP_NOPARAMS()
    PHP_AMQP_RETURN_THIS_PROP("prefetchCount")
}
/* }}} */

/* {{{ proto bool amqp::setPrefetchSize(long size)
set the number of prefetches */
static PHP_METHOD(amqp_channel_class, setPrefetchSize)
{
    zval rv;

    amqp_channel_resource *channel_resource;
    zend_long prefetch_size;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &prefetch_size) == FAILURE) {
        RETURN_THROWS();
    }

    if (!php_amqp_is_valid_prefetch_size(prefetch_size)) {
        zend_throw_exception_ex(
            amqp_connection_exception_class_entry,
            0,
            "Parameter 'prefetchSize' must be between 0 and %u.",
            PHP_AMQP_MAX_PREFETCH_SIZE
        );
        return;
    }

    channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(getThis());
    PHP_AMQP_VERIFY_CHANNEL_CONNECTION_RESOURCE(channel_resource, "Could not set prefetch size.");

    /* If we are already connected, set the new prefetch count */
    if (channel_resource->is_connected) {
        amqp_basic_qos(
            channel_resource->connection_resource->connection_state,
            channel_resource->channel_id,
            (uint32_t) prefetch_size,
            0,
            0
        );

        amqp_rpc_reply_t res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

        if (PHP_AMQP_MAYBE_ERROR(res, channel_resource, channel_resource->connection_resource)) {
            php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry);
            php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
            return;
        }

        php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

        uint32_t global_prefetch_size = (uint32_t) PHP_AMQP_READ_THIS_PROP_LONG("globalPrefetchSize");
        uint16_t global_prefetch_count = (uint16_t) PHP_AMQP_READ_THIS_PROP_LONG("globalPrefetchCount");

        /* Re-apply current global prefetch settings if set (writing consumer prefetch settings will clear global prefetch settings) */
        if (global_prefetch_size != 0 || global_prefetch_count != 0) {
            amqp_basic_qos(
                channel_resource->connection_resource->connection_state,
                channel_resource->channel_id,
                global_prefetch_size,
                global_prefetch_count,
                1
            );

            res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

            if (PHP_AMQP_MAYBE_ERROR(res, channel_resource, channel_resource->connection_resource)) {
                php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry);
                php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
                return;
            }

            php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
        }
    }

    /* Set the prefetch size - the implication is to disable the count */
    zend_update_property_long(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("prefetchCount"), 0);
    zend_update_property_long(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("prefetchSize"), prefetch_size);
}
/* }}} */

/* {{{ proto long amqp::getPrefetchSize()
get the number of prefetches */
static PHP_METHOD(amqp_channel_class, getPrefetchSize)
{
    zval rv;
    PHP_AMQP_NOPARAMS()
    PHP_AMQP_RETURN_THIS_PROP("prefetchSize")
}
/* }}} */

/* {{{ proto bool amqp::setGlobalPrefetchCount(long count)
set the number of prefetches */
static PHP_METHOD(amqp_channel_class, setGlobalPrefetchCount)
{
    amqp_channel_resource *channel_resource;
    zend_long global_prefetch_count;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &global_prefetch_count) == FAILURE) {
        RETURN_THROWS();
    }

    if (!php_amqp_is_valid_prefetch_count(global_prefetch_count)) {
        zend_throw_exception_ex(
            amqp_connection_exception_class_entry,
            0,
            "Parameter 'globalPrefetchCount' must be between 0 and %u.",
            PHP_AMQP_MAX_PREFETCH_COUNT
        );
        return;
    }

    channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(getThis());
    PHP_AMQP_VERIFY_CHANNEL_CONNECTION_RESOURCE(channel_resource, "Could not set global prefetch count.");

    /* If we are already connected, set the new prefetch count */
    if (channel_resource->is_connected) {
        /* Applying global prefetch settings retains existing consumer prefetch settings */
        amqp_basic_qos(
            channel_resource->connection_resource->connection_state,
            channel_resource->channel_id,
            0,
            (uint16_t) global_prefetch_count,
            1
        );

        amqp_rpc_reply_t res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

        if (PHP_AMQP_MAYBE_ERROR(res, channel_resource, channel_resource->connection_resource)) {
            php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry);
            php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
            return;
        }

        php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
    }

    /* Set the global prefetch count - the implication is to disable the size */
    zend_update_property_long(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("globalPrefetchCount"),
        global_prefetch_count
    );
    zend_update_property_long(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("globalPrefetchSize"), 0);
}
/* }}} */

/* {{{ proto long amqp::getGlobalPrefetchCount()
get the number of prefetches */
static PHP_METHOD(amqp_channel_class, getGlobalPrefetchCount)
{
    zval rv;
    PHP_AMQP_NOPARAMS()
    PHP_AMQP_RETURN_THIS_PROP("globalPrefetchCount")
}
/* }}} */

/* {{{ proto bool amqp::setGlobalPrefetchSize(long size)
set the number of prefetches */
static PHP_METHOD(amqp_channel_class, setGlobalPrefetchSize)
{
    amqp_channel_resource *channel_resource;
    zend_long global_prefetch_size;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &global_prefetch_size) == FAILURE) {
        RETURN_THROWS();
    }

    if (!php_amqp_is_valid_prefetch_size(global_prefetch_size)) {
        zend_throw_exception_ex(
            amqp_connection_exception_class_entry,
            0,
            "Parameter 'globalPrefetchSize' must be between 0 and %u.",
            PHP_AMQP_MAX_PREFETCH_SIZE
        );
        return;
    }

    channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(getThis());
    PHP_AMQP_VERIFY_CHANNEL_CONNECTION_RESOURCE(channel_resource, "Could not set prefetch size.");

    /* If we are already connected, set the new prefetch count */
    if (channel_resource->is_connected) {
        /* Applying global prefetch settings retains existing consumer prefetch settings */
        amqp_basic_qos(
            channel_resource->connection_resource->connection_state,
            channel_resource->channel_id,
            (uint32_t) global_prefetch_size,
            0,
            1
        );

        amqp_rpc_reply_t res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

        if (PHP_AMQP_MAYBE_ERROR(res, channel_resource, channel_resource->connection_resource)) {
            php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry);
            php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
            return;
        }

        php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
    }

    /* Set the global prefetch size - the implication is to disable the count */
    zend_update_property_long(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("globalPrefetchCount"), 0);
    zend_update_property_long(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("globalPrefetchSize"),
        global_prefetch_size
    );
}
/* }}} */

/* {{{ proto long amqp::getGlobalPrefetchSize()
get the number of prefetches */
static PHP_METHOD(amqp_channel_class, getGlobalPrefetchSize)
{
    zval rv;
    PHP_AMQP_NOPARAMS()
    PHP_AMQP_RETURN_THIS_PROP("globalPrefetchSize")
}
/* }}} */

/* {{{ proto amqp::qos(long size, long count, bool global)
set the number of prefetches */
static PHP_METHOD(amqp_channel_class, qos)
{
    zval rv;

    amqp_channel_resource *channel_resource;
    zend_long prefetch_size;
    zend_long prefetch_count;
    bool global = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll|b", &prefetch_size, &prefetch_count, &global) == FAILURE) {
        RETURN_THROWS();
    }

    channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(getThis());
    PHP_AMQP_VERIFY_CHANNEL_CONNECTION_RESOURCE(channel_resource, "Could not set qos parameters.");

    /* Set the prefetch size and prefetch count */
    if (global) {
        zend_update_property_long(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(getThis()),
            ZEND_STRL("globalPrefetchSize"),
            prefetch_size
        );
        zend_update_property_long(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(getThis()),
            ZEND_STRL("globalPrefetchCount"),
            prefetch_count
        );
    } else {
        zend_update_property_long(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("prefetchSize"), prefetch_size);
        zend_update_property_long(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(getThis()),
            ZEND_STRL("prefetchCount"),
            prefetch_count
        );
    }

    /* If we are already connected, set the new prefetch count */
    if (channel_resource->is_connected) {
        amqp_basic_qos(
            channel_resource->connection_resource->connection_state,
            channel_resource->channel_id,
            (uint32_t) PHP_AMQP_READ_THIS_PROP_LONG("prefetchSize"),
            (uint16_t) PHP_AMQP_READ_THIS_PROP_LONG("prefetchCount"),
            /* NOTE that RabbitMQ has reinterpreted global flag field. See https://www.rabbitmq.com/amqp-0-9-1-reference.html#basic.qos.global for details */
            0 /* Global flag - whether this change should affect every channel_resource */
        );

        amqp_rpc_reply_t res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

        if (PHP_AMQP_MAYBE_ERROR(res, channel_resource, channel_resource->connection_resource)) {
            php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry);
            php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
            return;
        }

        php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

        uint32_t global_prefetch_size = (uint32_t) PHP_AMQP_READ_THIS_PROP_LONG("globalPrefetchSize");
        uint16_t global_prefetch_count = (uint16_t) PHP_AMQP_READ_THIS_PROP_LONG("globalPrefetchCount");

        /* Re-apply current global prefetch settings if set (writing consumer prefetch settings will clear global prefetch settings) */
        if (global_prefetch_size != 0 || global_prefetch_count != 0) {
            amqp_basic_qos(
                channel_resource->connection_resource->connection_state,
                channel_resource->channel_id,
                global_prefetch_size,
                global_prefetch_count,
                1
            );

            res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

            if (PHP_AMQP_MAYBE_ERROR(res, channel_resource, channel_resource->connection_resource)) {
                php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry);
                php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
                return;
            }

            php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
        }
    }
}
/* }}} */


/* {{{ proto amqp::startTransaction()
start a transaction on the given channel */
static PHP_METHOD(amqp_channel_class, startTransaction)
{
    amqp_channel_resource *channel_resource;

    amqp_rpc_reply_t res;

    PHP_AMQP_NOPARAMS()

    channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(getThis());
    PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not start the transaction.");

    amqp_tx_select(channel_resource->connection_resource->connection_state, channel_resource->channel_id);

    res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

    if (PHP_AMQP_MAYBE_ERROR(res, channel_resource, channel_resource->connection_resource)) {
        php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry);
        php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
        return;
    }

    php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
}
/* }}} */


/* {{{ proto amqp::startTransaction()
start a transaction on the given channel */
static PHP_METHOD(amqp_channel_class, commitTransaction)
{
    amqp_channel_resource *channel_resource;

    amqp_rpc_reply_t res;

    PHP_AMQP_NOPARAMS()

    channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(getThis());
    PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not start the transaction.");

    amqp_tx_commit(channel_resource->connection_resource->connection_state, channel_resource->channel_id);

    res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

    if (PHP_AMQP_MAYBE_ERROR(res, channel_resource, channel_resource->connection_resource)) {
        php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry);
        php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
        return;
    }

    php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
}
/* }}} */

/* {{{ proto amqp::startTransaction()
start a transaction on the given channel */
static PHP_METHOD(amqp_channel_class, rollbackTransaction)
{
    amqp_channel_resource *channel_resource;

    amqp_rpc_reply_t res;

    PHP_AMQP_NOPARAMS()

    channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(getThis());
    PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not rollback the transaction.");

    amqp_tx_rollback(channel_resource->connection_resource->connection_state, channel_resource->channel_id);

    res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

    if (PHP_AMQP_MAYBE_ERROR(res, channel_resource, channel_resource->connection_resource)) {
        php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry);
        php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
        return;
    }

    php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
}
/* }}} */

/* {{{ proto AMQPChannel::getConnection()
Get the AMQPConnection object in use */
static PHP_METHOD(amqp_channel_class, getConnection)
{
    zval rv;
    PHP_AMQP_NOPARAMS()
    PHP_AMQP_RETURN_THIS_PROP("connection")
}
/* }}} */

/* {{{ proto bool amqp::basicRecover([bool requeue=TRUE])
Redeliver unacknowledged messages */
static PHP_METHOD(amqp_channel_class, basicRecover)
{
    amqp_channel_resource *channel_resource;

    amqp_rpc_reply_t res;

    bool requeue = 1;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "|b", &requeue) == FAILURE) {
        RETURN_THROWS();
    }

    channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(getThis());
    PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not redeliver unacknowledged messages.");

    amqp_basic_recover(
        channel_resource->connection_resource->connection_state,
        channel_resource->channel_id,
        (amqp_boolean_t) requeue
    );

    res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

    if (PHP_AMQP_MAYBE_ERROR(res, channel_resource, channel_resource->connection_resource)) {
        php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry);
        php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
        return;
    }

    php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
}
/* }}} */

/* {{{ proto bool amqp::confirmSelect()
Redeliver unacknowledged messages */
PHP_METHOD(amqp_channel_class, confirmSelect)
{
    amqp_channel_resource *channel_resource;
    amqp_rpc_reply_t res;

    PHP_AMQP_NOPARAMS()

    channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(getThis());
    PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not enable confirms mode.");

    amqp_confirm_select(channel_resource->connection_resource->connection_state, channel_resource->channel_id);

    res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

    if (PHP_AMQP_MAYBE_ERROR(res, channel_resource, channel_resource->connection_resource)) {
        php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry);
        php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

        return;
    }

    php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
}
/* }}} */

/* {{{ proto bool AMQPChannel::setReturnCallback(callable return_callback)
Set callback for basic.return server method handling */
PHP_METHOD(amqp_channel_class, setReturnCallback)
{
    zend_fcall_info fci = empty_fcall_info;
    zend_fcall_info_cache fcc = empty_fcall_info_cache;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "f!", &fci, &fcc) == FAILURE) {
        RETURN_THROWS();
    }

    amqp_channel_object *channel = PHP_AMQP_GET_CHANNEL(getThis());

    php_amqp_destroy_fci(&channel->callbacks.basic_return.fci);

    if (ZEND_FCI_INITIALIZED(fci)) {
        php_amqp_duplicate_fci(&fci);
        channel->callbacks.basic_return.fci = fci;
        channel->callbacks.basic_return.fcc = fcc;
    }
}
/* }}} */

/* {{{ proto bool AMQPChannel::waitForBasicReturn([double timeout=0.0])
Wait for basic.return method from server */
PHP_METHOD(amqp_channel_class, waitForBasicReturn)
{
    amqp_channel_object *channel;
    amqp_channel_resource *channel_resource;
    amqp_method_t method;

    int status;

    double timeout = 0;

    struct timeval tv = {0};
    struct timeval *tv_ptr = &tv;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "|d", &timeout) == FAILURE) {
        RETURN_THROWS();
    }

    if (timeout < 0) {
        zend_throw_exception(amqp_channel_exception_class_entry, "Timeout must be greater than or equal to zero.", 0);
        return;
    }

    channel = PHP_AMQP_GET_CHANNEL(getThis());

    channel_resource = channel->channel_resource;
    PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not start wait loop for basic.return method.");

    if (timeout > 0) {
        tv.tv_sec = (long int) timeout;
        tv.tv_usec = (long int) ((timeout - tv.tv_sec) * 1000000);
    } else {
        tv_ptr = NULL;
    }

    assert(channel_resource->channel_id > 0);

    while (1) {
        php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

        status = amqp_simple_wait_method_noblock(
            channel_resource->connection_resource->connection_state,
            channel_resource->channel_id,
            AMQP_BASIC_RETURN_METHOD,
            &method,
            tv_ptr
        );

        if (AMQP_STATUS_TIMEOUT == status) {
            zend_throw_exception(amqp_queue_exception_class_entry, "Wait timeout exceed", 0);

            php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
            return;
        }

        if (status != AMQP_STATUS_OK) {
            /* Emulate library error */
            amqp_rpc_reply_t res;

            if (AMQP_RESPONSE_SERVER_EXCEPTION == status) {
                res.reply_type = AMQP_RESPONSE_SERVER_EXCEPTION;
                res.reply = method;
            } else {
                res.reply_type = AMQP_RESPONSE_LIBRARY_EXCEPTION;
                res.library_error = status;
            }

            php_amqp_error(res, &PHP_AMQP_G(error_message), channel_resource->connection_resource, channel_resource);

            php_amqp_zend_throw_exception(
                res,
                amqp_queue_exception_class_entry,
                PHP_AMQP_G(error_message),
                PHP_AMQP_G(error_code)
            );
            php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
            return;
        }

        status = php_amqp_handle_basic_return(&PHP_AMQP_G(error_message), channel, &method);

        if (PHP_AMQP_RESOURCE_RESPONSE_BREAK == status) {
            php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
            break;
        }

        if (PHP_AMQP_RESOURCE_RESPONSE_OK != status) {
            /* Emulate library error */
            amqp_rpc_reply_t res;
            res.reply_type = AMQP_RESPONSE_LIBRARY_EXCEPTION;
            res.library_error = status;

            php_amqp_error(res, &PHP_AMQP_G(error_message), channel_resource->connection_resource, channel_resource);

            php_amqp_zend_throw_exception(
                res,
                amqp_channel_exception_class_entry,
                PHP_AMQP_G(error_message),
                PHP_AMQP_G(error_code)
            );
            php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
            return;
        }
    }
}
/* }}} */

/* {{{ proto bool AMQPChannel::setConfirmCallback(callable ack_callback [, callable nack_callback = null])
Set callback for basic.ack and, optionally, basic.nac server methods handling */
PHP_METHOD(amqp_channel_class, setConfirmCallback)
{
    zend_fcall_info ack_fci = empty_fcall_info;
    zend_fcall_info_cache ack_fcc = empty_fcall_info_cache;

    zend_fcall_info nack_fci = empty_fcall_info;
    zend_fcall_info_cache nack_fcc = empty_fcall_info_cache;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "f!|f!", &ack_fci, &ack_fcc, &nack_fci, &nack_fcc) == FAILURE) {
        RETURN_THROWS();
    }

    amqp_channel_object *channel = PHP_AMQP_GET_CHANNEL(getThis());

    php_amqp_destroy_fci(&channel->callbacks.basic_ack.fci);

    if (ZEND_FCI_INITIALIZED(ack_fci)) {
        php_amqp_duplicate_fci(&ack_fci);
        channel->callbacks.basic_ack.fci = ack_fci;
        channel->callbacks.basic_ack.fcc = ack_fcc;
    }

    php_amqp_destroy_fci(&channel->callbacks.basic_nack.fci);

    if (ZEND_FCI_INITIALIZED(nack_fci)) {
        php_amqp_duplicate_fci(&nack_fci);
        channel->callbacks.basic_nack.fci = nack_fci;
        channel->callbacks.basic_nack.fcc = nack_fcc;
    }
}
/* }}} */


/* {{{ proto bool amqp::waitForConfirm([double timeout=0.0])
Redeliver unacknowledged messages */
PHP_METHOD(amqp_channel_class, waitForConfirm)
{
    amqp_channel_object *channel;
    amqp_channel_resource *channel_resource;
    amqp_method_t method;

    int status;

    double timeout = 0;

    struct timeval tv = {0};
    struct timeval *tv_ptr = &tv;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "|d", &timeout) == FAILURE) {
        RETURN_THROWS();
    }

    if (timeout < 0) {
        zend_throw_exception(amqp_channel_exception_class_entry, "Timeout must be greater than or equal to zero.", 0);
        return;
    }

    channel = PHP_AMQP_GET_CHANNEL(getThis());

    channel_resource = channel->channel_resource;
    PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not start wait loop for basic.return method.");

    if (timeout > 0) {
        tv.tv_sec = (long int) timeout;
        tv.tv_usec = (long int) ((timeout - tv.tv_sec) * 1000000);
    } else {
        tv_ptr = NULL;
    }

    assert(channel_resource->channel_id > 0);


    amqp_method_number_t expected_methods[] =
        {AMQP_BASIC_ACK_METHOD, AMQP_BASIC_NACK_METHOD, AMQP_BASIC_RETURN_METHOD, 0};

    while (1) {
        php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

        status = amqp_simple_wait_method_list_noblock(
            channel_resource->connection_resource->connection_state,
            channel_resource->channel_id,
            expected_methods,
            &method,
            tv_ptr
        );

        if (AMQP_STATUS_TIMEOUT == status) {
            zend_throw_exception(amqp_queue_exception_class_entry, "Wait timeout exceed", 0);

            php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
            return;
        }

        if (status != AMQP_STATUS_OK) {
            /* Emulate library error */
            amqp_rpc_reply_t res;

            if (AMQP_RESPONSE_SERVER_EXCEPTION == status) {
                res.reply_type = AMQP_RESPONSE_SERVER_EXCEPTION;
                res.reply = method;
            } else {
                res.reply_type = AMQP_RESPONSE_LIBRARY_EXCEPTION;
                res.library_error = status;
            }

            php_amqp_error(res, &PHP_AMQP_G(error_message), channel_resource->connection_resource, channel_resource);

            php_amqp_zend_throw_exception(
                res,
                amqp_channel_exception_class_entry,
                PHP_AMQP_G(error_message),
                PHP_AMQP_G(error_code)
            );
            php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
            return;
        }

        switch (method.id) {
            case AMQP_BASIC_ACK_METHOD:
                status = php_amqp_handle_basic_ack(&PHP_AMQP_G(error_message), channel, &method);
                break;
            case AMQP_BASIC_NACK_METHOD:
                status = php_amqp_handle_basic_nack(&PHP_AMQP_G(error_message), channel, &method);
                break;
            case AMQP_BASIC_RETURN_METHOD:
                status = php_amqp_handle_basic_return(&PHP_AMQP_G(error_message), channel, &method);
                break;
            default:
                status = AMQP_STATUS_WRONG_METHOD;
        }

        if (PHP_AMQP_RESOURCE_RESPONSE_BREAK == status) {
            php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
            break;
        }

        if (PHP_AMQP_RESOURCE_RESPONSE_OK != status) {
            /* Emulate library error */
            amqp_rpc_reply_t res;
            res.reply_type = AMQP_RESPONSE_LIBRARY_EXCEPTION;
            res.library_error = status;

            php_amqp_error(res, &PHP_AMQP_G(error_message), channel_resource->connection_resource, channel_resource);

            php_amqp_zend_throw_exception(
                res,
                amqp_queue_exception_class_entry,
                PHP_AMQP_G(error_message),
                PHP_AMQP_G(error_code)
            );
            php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
            return;
        }
    }
}
/* }}} */

/* {{{ proto AMQPChannel::getConsumers() */
static PHP_METHOD(amqp_channel_class, getConsumers)
{
    zval rv;
    PHP_AMQP_NOPARAMS()
    zval *tmp = zend_read_property(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("consumers"), 0, &rv);
    // Return a proper copy, so that the internal consumer map can be safely modified
    ZVAL_DUP(return_value, tmp);
}
/* }}} */


ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class__construct, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
    ZEND_ARG_OBJ_INFO(0, connection, AMQPConnection, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_channel_class_isConnected, ZEND_SEND_BY_VAL, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_channel_class_close, ZEND_SEND_BY_VAL, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_channel_class_getChannelId, ZEND_SEND_BY_VAL, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_channel_class_setPrefetchSize, ZEND_SEND_BY_VAL, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, size, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_channel_class_getPrefetchSize, ZEND_SEND_BY_VAL, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_channel_class_setPrefetchCount, ZEND_SEND_BY_VAL, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, count, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_channel_class_getPrefetchCount, ZEND_SEND_BY_VAL, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(
    arginfo_amqp_channel_class_setGlobalPrefetchSize,
    ZEND_SEND_BY_VAL,
    1,
    IS_VOID,
    0
)
    ZEND_ARG_TYPE_INFO(0, size, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(
    arginfo_amqp_channel_class_getGlobalPrefetchSize,
    ZEND_SEND_BY_VAL,
    0,
    IS_LONG,
    0
)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(
    arginfo_amqp_channel_class_setGlobalPrefetchCount,
    ZEND_SEND_BY_VAL,
    1,
    IS_VOID,
    0
)
    ZEND_ARG_TYPE_INFO(0, count, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(
    arginfo_amqp_channel_class_getGlobalPrefetchCount,
    ZEND_SEND_BY_VAL,
    0,
    IS_LONG,
    0
)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_channel_class_qos, ZEND_SEND_BY_VAL, 2, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, size, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, count, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, global, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_channel_class_startTransaction, ZEND_SEND_BY_VAL, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_channel_class_commitTransaction, ZEND_SEND_BY_VAL, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_channel_class_rollbackTransaction, ZEND_SEND_BY_VAL, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO(arginfo_amqp_channel_class_getConnection, AMQPConnection, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_channel_class_basicRecover, ZEND_SEND_BY_VAL, 0, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, requeue, _IS_BOOL, 0, "true")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_channel_class_confirmSelect, ZEND_SEND_BY_VAL, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_channel_class_setConfirmCallback, ZEND_SEND_BY_VAL, 1, IS_VOID, 0)
    ZEND_ARG_CALLABLE_INFO(0, ackCallback, 1)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, nackCallback, IS_CALLABLE, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_channel_class_waitForConfirm, ZEND_SEND_BY_VAL, 0, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timeout, IS_DOUBLE, 0, "0.0")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_channel_class_setReturnCallback, ZEND_SEND_BY_VAL, 1, IS_VOID, 0)
    ZEND_ARG_CALLABLE_INFO(0, returnCallback, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_channel_class_waitForBasicReturn, ZEND_SEND_BY_VAL, 0, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timeout, IS_DOUBLE, 0, "0.0")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_channel_class_getConsumers, ZEND_SEND_BY_VAL, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()


zend_function_entry amqp_channel_class_functions[] = {
    PHP_ME(amqp_channel_class, __construct, 	arginfo_amqp_channel_class__construct,		ZEND_ACC_PUBLIC)
    PHP_ME(amqp_channel_class, isConnected, 	arginfo_amqp_channel_class_isConnected,		ZEND_ACC_PUBLIC)
    PHP_ME(amqp_channel_class, close,       	arginfo_amqp_channel_class_close,			ZEND_ACC_PUBLIC)

    PHP_ME(amqp_channel_class, getChannelId,    arginfo_amqp_channel_class_getChannelId,    ZEND_ACC_PUBLIC)

    PHP_ME(amqp_channel_class, setPrefetchSize, 		arginfo_amqp_channel_class_setPrefetchSize,			ZEND_ACC_PUBLIC)
    PHP_ME(amqp_channel_class, getPrefetchSize, 		arginfo_amqp_channel_class_getPrefetchSize,			ZEND_ACC_PUBLIC)
    PHP_ME(amqp_channel_class, setPrefetchCount,		arginfo_amqp_channel_class_setPrefetchCount,		ZEND_ACC_PUBLIC)
    PHP_ME(amqp_channel_class, getPrefetchCount,		arginfo_amqp_channel_class_getPrefetchCount,		ZEND_ACC_PUBLIC)
    PHP_ME(amqp_channel_class, setGlobalPrefetchSize, 	arginfo_amqp_channel_class_setGlobalPrefetchSize,	ZEND_ACC_PUBLIC)
    PHP_ME(amqp_channel_class, getGlobalPrefetchSize, 	arginfo_amqp_channel_class_getGlobalPrefetchSize,	ZEND_ACC_PUBLIC)
    PHP_ME(amqp_channel_class, setGlobalPrefetchCount,	arginfo_amqp_channel_class_setGlobalPrefetchCount,	ZEND_ACC_PUBLIC)
    PHP_ME(amqp_channel_class, getGlobalPrefetchCount,	arginfo_amqp_channel_class_getGlobalPrefetchCount,	ZEND_ACC_PUBLIC)
    PHP_ME(amqp_channel_class, qos,						arginfo_amqp_channel_class_qos,						ZEND_ACC_PUBLIC)

    PHP_ME(amqp_channel_class, startTransaction,	arginfo_amqp_channel_class_startTransaction,	ZEND_ACC_PUBLIC)
    PHP_ME(amqp_channel_class, commitTransaction,	arginfo_amqp_channel_class_commitTransaction,	ZEND_ACC_PUBLIC)
    PHP_ME(amqp_channel_class, rollbackTransaction,	arginfo_amqp_channel_class_rollbackTransaction,	ZEND_ACC_PUBLIC)

    PHP_ME(amqp_channel_class, getConnection,	arginfo_amqp_channel_class_getConnection, ZEND_ACC_PUBLIC)

    PHP_ME(amqp_channel_class, basicRecover,	arginfo_amqp_channel_class_basicRecover, ZEND_ACC_PUBLIC)

    PHP_ME(amqp_channel_class, confirmSelect,	arginfo_amqp_channel_class_confirmSelect, ZEND_ACC_PUBLIC)
    PHP_ME(amqp_channel_class, waitForConfirm,	arginfo_amqp_channel_class_waitForConfirm, ZEND_ACC_PUBLIC)
    PHP_ME(amqp_channel_class, setConfirmCallback,	arginfo_amqp_channel_class_setConfirmCallback, ZEND_ACC_PUBLIC)

    PHP_ME(amqp_channel_class, setReturnCallback, arginfo_amqp_channel_class_setReturnCallback, ZEND_ACC_PUBLIC)
    PHP_ME(amqp_channel_class, waitForBasicReturn, arginfo_amqp_channel_class_waitForBasicReturn, ZEND_ACC_PUBLIC)

    PHP_ME(amqp_channel_class, getConsumers, 	arginfo_amqp_channel_class_getConsumers,		ZEND_ACC_PUBLIC)

    {NULL, NULL, NULL}
};

PHP_MINIT_FUNCTION(amqp_channel)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "AMQPChannel", amqp_channel_class_functions);
    ce.create_object = amqp_channel_ctor;
    amqp_channel_class_entry = zend_register_internal_class(&ce);

    PHP_AMQP_DECLARE_TYPED_PROPERTY_OBJ(this_ce, "connection", ZEND_ACC_PRIVATE, AMQPConnection, 0);
    PHP_AMQP_DECLARE_TYPED_PROPERTY(this_ce, "prefetchCount", ZEND_ACC_PRIVATE, IS_LONG, 1);
    PHP_AMQP_DECLARE_TYPED_PROPERTY(this_ce, "prefetchSize", ZEND_ACC_PRIVATE, IS_LONG, 1);
    PHP_AMQP_DECLARE_TYPED_PROPERTY(this_ce, "globalPrefetchCount", ZEND_ACC_PRIVATE, IS_LONG, 1);
    PHP_AMQP_DECLARE_TYPED_PROPERTY(this_ce, "globalPrefetchSize", ZEND_ACC_PRIVATE, IS_LONG, 1);
#if PHP_VERSION_ID >= 80000
    PHP_AMQP_DECLARE_TYPED_PROPERTY_WITH_DEFAULT(this_ce, "consumers", ZEND_ACC_PRIVATE, IS_ARRAY, 0, ZVAL_EMPTY_ARRAY);
#else
    PHP_AMQP_DECLARE_TYPED_PROPERTY_WITH_DEFAULT(this_ce, "consumers", ZEND_ACC_PRIVATE, IS_ARRAY, 0, ZVAL_NULL);
#endif

#if PHP_MAJOR_VERSION >= 7
    memcpy(&amqp_channel_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

    amqp_channel_object_handlers.offset = XtOffsetOf(amqp_channel_object, zo);
    amqp_channel_object_handlers.free_obj = amqp_channel_free;
#endif

#if ZEND_MODULE_API_NO >= 20100000
    amqp_channel_object_handlers.get_gc = amqp_channel_gc;
#endif

    return SUCCESS;
}
