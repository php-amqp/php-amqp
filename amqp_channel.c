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
# include "win32/php_stdint.h"
# include "win32/signal.h"
#else
# include <stdint.h>
# include <signal.h>
#endif

#include <amqp.h>
#include <amqp_framing.h>

#ifdef PHP_WIN32
# include "win32/unistd.h"
#else
# include <unistd.h>
#endif

#include "php_amqp.h"
#include "amqp_connection.h"
#include "amqp_methods_handling.h"
#include "amqp_connection_resource.h"
#include "amqp_channel.h"

zend_class_entry *amqp_channel_class_entry;
#define this_ce amqp_channel_class_entry

zend_object_handlers amqp_channel_object_handlers;

void php_amqp_close_channel(amqp_channel_resource *channel_resource, zend_bool check_errors TSRMLS_DC)
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

		amqp_channel_close(connection_resource->connection_state, channel_resource->channel_id, AMQP_REPLY_SUCCESS);

		amqp_rpc_reply_t res = amqp_get_rpc_reply(connection_resource->connection_state);

		if (check_errors && PHP_AMQP_MAYBE_ERROR(res, channel_resource)) {
			php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry TSRMLS_CC);
			return;
		}

		php_amqp_maybe_release_buffers_on_channel(connection_resource, channel_resource);
	}
}

#if PHP_MAJOR_VERSION >= 7

static void php_amqp_destroy_fci(zend_fcall_info *fci) {
    if (fci->size > 0) {
        zval_ptr_dtor(&fci->function_name);
        if (fci->object != NULL) {
#if PHP_VERSION_ID >= 70300
            GC_DELREF(fci->object);
#else
            GC_REFCOUNT(fci->object)--;
#endif
        }
        fci->size = 0;
    }
}

static void php_amqp_duplicate_fci(zend_fcall_info *source) {
    if (source->size > 0) {

        zval_add_ref(&source->function_name);
        if (source->object != NULL) {
#if PHP_VERSION_ID >= 70300
            GC_ADDREF(source->object);
#else
            GC_REFCOUNT(source->object)++;
#endif
        }
    }
}

static int php_amqp_get_fci_gc_data_count(zend_fcall_info *fci) {
    int cnt = 0;

    if (fci->size > 0) {
        cnt ++;

        if (fci->object != NULL) {
            cnt++;
        }
    }

    return cnt;
}

static zval * php_amqp_get_fci_gc_data(zend_fcall_info *fci, zval *gc_data) {
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
	int basic_ack_cnt    = php_amqp_get_fci_gc_data_count(&channel->callbacks.basic_ack.fci);
	int basic_nack_cnt   = php_amqp_get_fci_gc_data_count(&channel->callbacks.basic_nack.fci);

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
	*n     = cnt;

	return zend_std_get_properties(object TSRMLS_CC);
} /* }}} */

#else
static void php_amqp_destroy_fci(zend_fcall_info *fci) {
	if (fci->size > 0) {
		zval_ptr_dtor(&fci->function_name);
		if (fci->object_ptr != NULL) {
			zval_ptr_dtor(&fci->object_ptr);
		}
		fci->size = 0;
	}
}

static void php_amqp_duplicate_fci(zend_fcall_info *source) {
	if (source->size > 0) {

		zval_add_ref(&source->function_name);
		if (source->object_ptr != NULL) {
			zval_add_ref(&source->object_ptr);
		}
	}
}

static int php_amqp_get_fci_gc_data_count(zend_fcall_info *fci) {
	int cnt = 0;

	if (fci->size > 0) {
		cnt ++;

		if (fci->object_ptr != NULL) {
			cnt++;
		}
	}

	return cnt;
}

static int php_amqp_get_fci_gc_data(zend_fcall_info *fci, zval **gc_data, int offset) {

	if (ZEND_FCI_INITIALIZED(*fci)) {
		gc_data[offset++] = fci->function_name;

		if (fci->object_ptr != NULL) {
			gc_data[offset++] = fci->object_ptr;
		}
	}

	return offset;
}

static HashTable *amqp_channel_gc(zval *object, zval ***table, int *n TSRMLS_DC) /* {{{ */
{
	amqp_channel_object *channel = PHP_AMQP_GET_CHANNEL(object);

	int basic_return_cnt = php_amqp_get_fci_gc_data_count(&channel->callbacks.basic_return.fci);
	int basic_ack_cnt    = php_amqp_get_fci_gc_data_count(&channel->callbacks.basic_ack.fci);
	int basic_nack_cnt   = php_amqp_get_fci_gc_data_count(&channel->callbacks.basic_nack.fci);

	int cnt = basic_return_cnt + basic_ack_cnt + basic_nack_cnt;

	if (cnt > channel->gc_data_count) {
		channel->gc_data_count = cnt;
		channel->gc_data = (zval **) erealloc(channel->gc_data, sizeof(zval *) * channel->gc_data_count);
	}

	php_amqp_get_fci_gc_data(&channel->callbacks.basic_return.fci, channel->gc_data, 0);
	php_amqp_get_fci_gc_data(&channel->callbacks.basic_ack.fci, channel->gc_data, basic_return_cnt);
	php_amqp_get_fci_gc_data(&channel->callbacks.basic_nack.fci, channel->gc_data, basic_return_cnt + basic_ack_cnt);

	*table = channel->gc_data;
	*n     = cnt;

	return zend_std_get_properties(PHP5to8_OBJ_PROP(object) TSRMLS_CC);
} /* }}} */

#endif

static void php_amqp_clean_callbacks(amqp_channel_callbacks *callbacks) {
	php_amqp_destroy_fci(&callbacks->basic_return.fci);
	php_amqp_destroy_fci(&callbacks->basic_ack.fci);
	php_amqp_destroy_fci(&callbacks->basic_nack.fci);
}


void amqp_channel_free(PHP5to7_obj_free_zend_object *object TSRMLS_DC)
{
	amqp_channel_object *channel = PHP_AMQP_FETCH_CHANNEL(object);

	if (channel->channel_resource != NULL) {
		php_amqp_close_channel(channel->channel_resource, 0 TSRMLS_CC);

		efree(channel->channel_resource);
		channel->channel_resource = NULL;
	}

	if (channel->gc_data) {
		efree(channel->gc_data);
	}

	php_amqp_clean_callbacks(&channel->callbacks);

	zend_object_std_dtor(&channel->zo TSRMLS_CC);

#if PHP_MAJOR_VERSION < 7
    if (channel->this_ptr) {
        channel->this_ptr = NULL;
    }

	efree(object);
#endif
}


PHP5to7_zend_object_value amqp_channel_ctor(zend_class_entry *ce TSRMLS_DC)
{
	amqp_channel_object *channel = PHP5to7_ECALLOC_CHANNEL_OBJECT(ce);

	zend_object_std_init(&channel->zo, ce TSRMLS_CC);
	AMQP_OBJECT_PROPERTIES_INIT(channel->zo, ce);

#if PHP_MAJOR_VERSION >=7
	channel->zo.handlers = &amqp_channel_object_handlers;

	return &channel->zo;
#else
	PHP5to7_zend_object_value new_value;

	new_value.handle = zend_objects_store_put(
			channel,
			NULL,
			(zend_objects_free_object_storage_t) amqp_channel_free,
			NULL TSRMLS_CC
	);

	new_value.handlers = zend_get_std_object_handlers();

	return new_value;
#endif
}


/* {{{ proto AMQPChannel::__construct(AMQPConnection obj)
 */
static PHP_METHOD(amqp_channel_class, __construct)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;

	zval *connection_object = NULL;

	amqp_channel_resource *channel_resource;
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	/* Parse out the method parameters */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &connection_object, amqp_connection_class_entry) == FAILURE) {
		zend_throw_exception(amqp_channel_exception_class_entry, "Parameter must be an instance of AMQPConnection.", 0 TSRMLS_CC);
		RETURN_NULL();
	}

	PHP5to7_zval_t consumers PHP5to7_MAYBE_SET_TO_NULL;

	PHP5to7_MAYBE_INIT(consumers);
	PHP5to7_ARRAY_INIT(consumers);

	zend_update_property(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("consumers"), PHP5to7_MAYBE_PTR(consumers) TSRMLS_CC);

	PHP5to7_MAYBE_DESTROY(consumers);

	channel = PHP_AMQP_GET_CHANNEL(getThis());
#if PHP_MAJOR_VERSION < 7
    channel->this_ptr = getThis();
#endif

	/* Set the prefetch count */
	zend_update_property_long(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("prefetch_count"), INI_INT("amqp.prefetch_count") TSRMLS_CC);

	/* Set the prefetch size */
	zend_update_property_long(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("prefetch_size"), INI_INT("amqp.prefetch_size") TSRMLS_CC);

	/* Set the global prefetch count */
	zend_update_property_long(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("global_prefetch_count"), INI_INT("amqp.global_prefetch_count") TSRMLS_CC);

	/* Set the global prefetch size */
	zend_update_property_long(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("global_prefetch_size"), INI_INT("amqp.global_prefetch_size") TSRMLS_CC);

	/* Pull out and verify the connection */
	connection = PHP_AMQP_GET_CONNECTION(connection_object);
	PHP_AMQP_VERIFY_CONNECTION(connection, "Could not create channel.");

	if (!connection->connection_resource) {
		zend_throw_exception(amqp_channel_exception_class_entry, "Could not create channel. No connection resource.", 0 TSRMLS_CC);
		return;
	}

	if (!connection->connection_resource->is_connected) {
		zend_throw_exception(amqp_channel_exception_class_entry, "Could not create channel. Connection resource is not connected.", 0 TSRMLS_CC);
		return;
	}

	zend_update_property(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("connection"), connection_object TSRMLS_CC);

	channel_resource = (amqp_channel_resource*)ecalloc(1, sizeof(amqp_channel_resource));
	channel->channel_resource = channel_resource;
    channel_resource->parent = channel;

	/* Figure out what the next available channel is on this connection */
	channel_resource->channel_id = php_amqp_connection_resource_get_available_channel_id(connection->connection_resource);

	/* Check that we got a valid channel */
	if (!channel_resource->channel_id) {
		zend_throw_exception(amqp_channel_exception_class_entry, "Could not create channel. Connection has no open channel slots remaining.", 0 TSRMLS_CC);
		return;
	}

	if (php_amqp_connection_resource_register_channel(connection->connection_resource, channel_resource, channel_resource->channel_id) == FAILURE) {
		zend_throw_exception(amqp_channel_exception_class_entry, "Could not create channel. Failed to add channel to connection slot.", 0 TSRMLS_CC);
	}

	/* Open up the channel for use */
	amqp_channel_open_ok_t *r = amqp_channel_open(channel_resource->connection_resource->connection_state, channel_resource->channel_id);


	if (!r) {
		amqp_rpc_reply_t res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

		php_amqp_error(res, &PHP_AMQP_G(error_message), channel_resource->connection_resource, channel_resource TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_channel_exception_class_entry, PHP_AMQP_G(error_message), PHP_AMQP_G(error_code) TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

		/* Prevent double free, it may happens in case case channel resource was already freed due to some hard error. */
		if (channel_resource->connection_resource) {
			php_amqp_connection_resource_unregister_channel(channel_resource->connection_resource, channel_resource->channel_id);
			channel_resource->channel_id = 0;
		}

		return;
	}

	/* r->channel_id is a 16-bit channel number insibe amqp_bytes_t, assertion below will without converting to uint16_t*/
	/* assert (r->channel_id == channel_resource->channel_id);*/
	php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

	channel_resource->is_connected = '\1';

	/* Set the prefetch count: */
	amqp_basic_qos(
		channel_resource->connection_resource->connection_state,
		channel_resource->channel_id,
		0,							/* prefetch window size */
		(uint16_t)PHP_AMQP_READ_THIS_PROP_LONG("prefetch_count"),	/* prefetch message count */
		/* NOTE that RabbitMQ has reinterpreted global flag field. See https://www.rabbitmq.com/amqp-0-9-1-reference.html#basic.qos.global for details */
		0							/* global flag */
	);

	amqp_rpc_reply_t res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

	if (PHP_AMQP_MAYBE_ERROR(res, channel_resource)) {
		php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
		return;
	}

	php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

	uint16_t global_prefetch_size = (uint16_t)PHP_AMQP_READ_THIS_PROP_LONG("global_prefetch_size");
	uint16_t global_prefetch_count = (uint16_t)PHP_AMQP_READ_THIS_PROP_LONG("global_prefetch_count");

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

		if (PHP_AMQP_MAYBE_ERROR(res, channel_resource)) {
			php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry TSRMLS_CC);
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

	PHP_AMQP_NOPARAMS();

	channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(getThis());

	RETURN_BOOL(channel_resource && channel_resource->is_connected);
}
/* }}} */

/* {{{ proto bool AMQPChannel::close()
Close amqp channel */
static PHP_METHOD(amqp_channel_class, close)
{
    amqp_channel_resource *channel_resource;

    PHP_AMQP_NOPARAMS();

    channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(getThis());

    if(channel_resource && channel_resource->is_connected) {
        php_amqp_close_channel(channel_resource, 1 TSRMLS_CC);
    }
}
/* }}} */

/* {{{ proto bool amqp::getChannelId()
get amqp channel ID */
static PHP_METHOD(amqp_channel_class, getChannelId)
{
	amqp_channel_resource *channel_resource;

	PHP_AMQP_NOPARAMS();

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
	PHP5to7_READ_PROP_RV_PARAM_DECL;
	
	amqp_channel_resource *channel_resource;
	PHP5to7_param_long_type_t prefetch_count;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &prefetch_count) == FAILURE) {
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
			(uint16_t)prefetch_count,
			0
		);

		amqp_rpc_reply_t res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

		if (PHP_AMQP_MAYBE_ERROR(res, channel_resource)) {
			php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry TSRMLS_CC);
			php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
			return;
		}

		php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

		uint16_t global_prefetch_size = (uint16_t)PHP_AMQP_READ_THIS_PROP_LONG("global_prefetch_size");
		uint16_t global_prefetch_count = (uint16_t)PHP_AMQP_READ_THIS_PROP_LONG("global_prefetch_count");

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

			if (PHP_AMQP_MAYBE_ERROR(res, channel_resource)) {
				php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry TSRMLS_CC);
				php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
				return;
			}

			php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
		}
	}

	/* Set the prefetch count - the implication is to disable the size */
	zend_update_property_long(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("prefetch_count"), prefetch_count TSRMLS_CC);
	zend_update_property_long(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("prefetch_size"), 0 TSRMLS_CC);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto long amqp::getPrefetchCount()
get the number of prefetches */
static PHP_METHOD(amqp_channel_class, getPrefetchCount)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;
	PHP_AMQP_NOPARAMS();
	PHP_AMQP_RETURN_THIS_PROP("prefetch_count")
}
/* }}} */

/* {{{ proto bool amqp::setPrefetchSize(long size)
set the number of prefetches */
static PHP_METHOD(amqp_channel_class, setPrefetchSize)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;

	amqp_channel_resource *channel_resource;
	PHP5to7_param_long_type_t prefetch_size;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &prefetch_size) == FAILURE) {
		return;
	}

	channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(getThis());
	PHP_AMQP_VERIFY_CHANNEL_CONNECTION_RESOURCE(channel_resource, "Could not set prefetch size.");

	/* If we are already connected, set the new prefetch count */
	if (channel_resource->is_connected) {
		amqp_basic_qos(
			channel_resource->connection_resource->connection_state,
			channel_resource->channel_id,
			(uint16_t)prefetch_size,
			0,
			0
		);

		amqp_rpc_reply_t res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

		if (PHP_AMQP_MAYBE_ERROR(res, channel_resource)) {
			php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry TSRMLS_CC);
			php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
			return;
		}

		php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

		uint16_t global_prefetch_size = (uint16_t)PHP_AMQP_READ_THIS_PROP_LONG("global_prefetch_size");
		uint16_t global_prefetch_count = (uint16_t)PHP_AMQP_READ_THIS_PROP_LONG("global_prefetch_count");

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

			if (PHP_AMQP_MAYBE_ERROR(res, channel_resource)) {
				php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry TSRMLS_CC);
				php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
				return;
			}

			php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
		}
	}

	/* Set the prefetch size - the implication is to disable the count */
	zend_update_property_long(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("prefetch_count"), 0 TSRMLS_CC);
	zend_update_property_long(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("prefetch_size"), prefetch_size TSRMLS_CC);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto long amqp::getPrefetchSize()
get the number of prefetches */
static PHP_METHOD(amqp_channel_class, getPrefetchSize)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;
	PHP_AMQP_NOPARAMS();
	PHP_AMQP_RETURN_THIS_PROP("prefetch_size")
}
/* }}} */

/* {{{ proto bool amqp::setGlobalPrefetchCount(long count)
set the number of prefetches */
static PHP_METHOD(amqp_channel_class, setGlobalPrefetchCount)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;

	amqp_channel_resource *channel_resource;
	PHP5to7_param_long_type_t global_prefetch_count;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &global_prefetch_count) == FAILURE) {
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
			(uint16_t)global_prefetch_count,
			1
		);

		amqp_rpc_reply_t res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

		if (PHP_AMQP_MAYBE_ERROR(res, channel_resource)) {
			php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry TSRMLS_CC);
			php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
			return;
		}

		php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
	}

	/* Set the global prefetch count - the implication is to disable the size */
	zend_update_property_long(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("global_prefetch_count"), global_prefetch_count TSRMLS_CC);
	zend_update_property_long(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("global_prefetch_size"), 0 TSRMLS_CC);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto long amqp::getGlobalPrefetchCount()
get the number of prefetches */
static PHP_METHOD(amqp_channel_class, getGlobalPrefetchCount)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;
	PHP_AMQP_NOPARAMS();
	PHP_AMQP_RETURN_THIS_PROP("global_prefetch_count")
}
/* }}} */

/* {{{ proto bool amqp::setGlobalPrefetchSize(long size)
set the number of prefetches */
static PHP_METHOD(amqp_channel_class, setGlobalPrefetchSize)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;

	amqp_channel_resource *channel_resource;
	PHP5to7_param_long_type_t global_prefetch_size;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &global_prefetch_size) == FAILURE) {
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
			(uint16_t)global_prefetch_size,
			0,
			1
		);

		amqp_rpc_reply_t res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

		if (PHP_AMQP_MAYBE_ERROR(res, channel_resource)) {
			php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry TSRMLS_CC);
			php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
			return;
		}

		php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
	}

	/* Set the global prefetch size - the implication is to disable the count */
	zend_update_property_long(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("global_prefetch_count"), 0 TSRMLS_CC);
	zend_update_property_long(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("global_prefetch_size"), global_prefetch_size TSRMLS_CC);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto long amqp::getGlobalPrefetchSize()
get the number of prefetches */
static PHP_METHOD(amqp_channel_class, getGlobalPrefetchSize)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;
	PHP_AMQP_NOPARAMS();
	PHP_AMQP_RETURN_THIS_PROP("global_prefetch_size")
}
/* }}} */

/* {{{ proto amqp::qos(long size, long count, bool global)
set the number of prefetches */
static PHP_METHOD(amqp_channel_class, qos)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;

	amqp_channel_resource *channel_resource;
	PHP5to7_param_long_type_t prefetch_size;
	PHP5to7_param_long_type_t prefetch_count;
	zend_bool global = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll|b", &prefetch_size, &prefetch_count, &global) == FAILURE) {
		return;
	}

	channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(getThis());
	PHP_AMQP_VERIFY_CHANNEL_CONNECTION_RESOURCE(channel_resource, "Could not set qos parameters.");

	/* Set the prefetch size and prefetch count */
	if (global) {
		zend_update_property_long(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("global_prefetch_size"), prefetch_size TSRMLS_CC);
		zend_update_property_long(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("global_prefetch_count"), prefetch_count TSRMLS_CC);
	} else {
		zend_update_property_long(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("prefetch_size"), prefetch_size TSRMLS_CC);
		zend_update_property_long(this_ce, PHP5to8_OBJ_PROP(getThis()), ZEND_STRL("prefetch_count"), prefetch_count TSRMLS_CC);
	}

	/* If we are already connected, set the new prefetch count */
	if (channel_resource->is_connected) {
		amqp_basic_qos(
			channel_resource->connection_resource->connection_state,
			channel_resource->channel_id,
			(uint16_t)PHP_AMQP_READ_THIS_PROP_LONG("prefetch_size"),
			(uint16_t)PHP_AMQP_READ_THIS_PROP_LONG("prefetch_count"),
			/* NOTE that RabbitMQ has reinterpreted global flag field. See https://www.rabbitmq.com/amqp-0-9-1-reference.html#basic.qos.global for details */
			0							/* Global flag - whether this change should affect every channel_resource */
		);

		amqp_rpc_reply_t res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

		if (PHP_AMQP_MAYBE_ERROR(res, channel_resource)) {
			php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry TSRMLS_CC);
			php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
			return;
		}

		php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

		uint16_t global_prefetch_size = (uint16_t)PHP_AMQP_READ_THIS_PROP_LONG("global_prefetch_size");
		uint16_t global_prefetch_count = (uint16_t)PHP_AMQP_READ_THIS_PROP_LONG("global_prefetch_count");

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

			if (PHP_AMQP_MAYBE_ERROR(res, channel_resource)) {
				php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry TSRMLS_CC);
				php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
				return;
			}

			php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
		}
	}

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto amqp::startTransaction()
start a transaction on the given channel */
static PHP_METHOD(amqp_channel_class, startTransaction)
{
	amqp_channel_resource *channel_resource;

	amqp_rpc_reply_t res;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
		return;
	}

	channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(getThis());
	PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not start the transaction.");

	amqp_tx_select(
		channel_resource->connection_resource->connection_state,
		channel_resource->channel_id
	);

	res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

	if (PHP_AMQP_MAYBE_ERROR(res, channel_resource)) {
		php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
		return;
	}

	php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto amqp::startTransaction()
start a transaction on the given channel */
static PHP_METHOD(amqp_channel_class, commitTransaction)
{
	amqp_channel_resource *channel_resource;

	amqp_rpc_reply_t res;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
		return;
	}

	channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(getThis());
	PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not start the transaction.");

	amqp_tx_commit(
		channel_resource->connection_resource->connection_state,
		channel_resource->channel_id
	);

	res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

	if (PHP_AMQP_MAYBE_ERROR(res, channel_resource)) {
		php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
		return;
	}

	php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto amqp::startTransaction()
start a transaction on the given channel */
static PHP_METHOD(amqp_channel_class, rollbackTransaction)
{
	amqp_channel_resource *channel_resource;

	amqp_rpc_reply_t res;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
		return;
	}

	
	channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(getThis());
	PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not rollback the transaction.");

	amqp_tx_rollback(
		channel_resource->connection_resource->connection_state,
		channel_resource->channel_id
	);

	res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

	if (PHP_AMQP_MAYBE_ERROR(res, channel_resource)) {
		php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
		return;
	}

	php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto AMQPChannel::getConnection()
Get the AMQPConnection object in use */
static PHP_METHOD(amqp_channel_class, getConnection)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;
	PHP_AMQP_NOPARAMS();
	PHP_AMQP_RETURN_THIS_PROP("connection")
}
/* }}} */

/* {{{ proto bool amqp::basicRecover([bool requeue=TRUE])
Redeliver unacknowledged messages */
static PHP_METHOD(amqp_channel_class, basicRecover)
{
	amqp_channel_resource *channel_resource;

	amqp_rpc_reply_t res;

	zend_bool requeue = 1;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|b", &requeue) == FAILURE) {
		return;
	}

	channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(getThis());
	PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not redeliver unacknowledged messages.");

	amqp_basic_recover(
		channel_resource->connection_resource->connection_state,
		channel_resource->channel_id,
		(amqp_boolean_t) requeue
	);

	res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

	if (PHP_AMQP_MAYBE_ERROR(res, channel_resource)) {
		php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
		return;
	}

	php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool amqp::confirmSelect()
Redeliver unacknowledged messages */
PHP_METHOD(amqp_channel_class, confirmSelect)
{
	amqp_channel_resource *channel_resource;
	amqp_rpc_reply_t res;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
		return;
	}

	channel_resource = PHP_AMQP_GET_CHANNEL_RESOURCE(getThis());
	PHP_AMQP_VERIFY_CHANNEL_RESOURCE(channel_resource, "Could not enable confirms mode.");

	amqp_confirm_select(
			channel_resource->connection_resource->connection_state,
			channel_resource->channel_id
	);

	res = amqp_get_rpc_reply(channel_resource->connection_resource->connection_state);

	if (PHP_AMQP_MAYBE_ERROR(res, channel_resource)) {
		php_amqp_zend_throw_exception_short(res, amqp_channel_exception_class_entry TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
		return;
	}

	php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool AMQPChannel::setReturnCallback(callable return_callback)
Set callback for basic.return server method handling */
PHP_METHOD(amqp_channel_class, setReturnCallback)
{
	zend_fcall_info fci = empty_fcall_info;
	zend_fcall_info_cache fcc = empty_fcall_info_cache;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f!", &fci, &fcc) == FAILURE) {
		return;
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

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|d", &timeout) == FAILURE) {
		return;
	}

	if (timeout < 0) {
		zend_throw_exception(amqp_channel_exception_class_entry, "Timeout must be greater than or equal to zero.", 0 TSRMLS_CC);
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

	while(1) {
		php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

		status = amqp_simple_wait_method_noblock(channel_resource->connection_resource->connection_state, channel_resource->channel_id, AMQP_BASIC_RETURN_METHOD, &method, tv_ptr);

		if (AMQP_STATUS_TIMEOUT == status) {
			zend_throw_exception(amqp_queue_exception_class_entry, "Wait timeout exceed", 0 TSRMLS_CC);

			php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
			return;
		}

		if (status != AMQP_STATUS_OK) {
			/* Emulate library error */
			amqp_rpc_reply_t res;

			if (AMQP_RESPONSE_SERVER_EXCEPTION == status) {
				res.reply_type = AMQP_RESPONSE_SERVER_EXCEPTION;
				res.reply      = method;
			} else {
				res.reply_type 	  = AMQP_RESPONSE_LIBRARY_EXCEPTION;
				res.library_error = status;
			}

			php_amqp_error(res, &PHP_AMQP_G(error_message), channel_resource->connection_resource, channel_resource TSRMLS_CC);

			php_amqp_zend_throw_exception(res, amqp_queue_exception_class_entry, PHP_AMQP_G(error_message), PHP_AMQP_G(error_code) TSRMLS_CC);
			php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
			return;
		}

		status = php_amqp_handle_basic_return(&PHP_AMQP_G(error_message), channel_resource->connection_resource, channel_resource->channel_id, channel, &method TSRMLS_CC);

		if (PHP_AMQP_RESOURCE_RESPONSE_BREAK == status) {
			php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
			break;
		}

		if (PHP_AMQP_RESOURCE_RESPONSE_OK != status) {
			/* Emulate library error */
			amqp_rpc_reply_t res;
			res.reply_type 	  = AMQP_RESPONSE_LIBRARY_EXCEPTION;
			res.library_error = status;

			php_amqp_error(res, &PHP_AMQP_G(error_message), channel_resource->connection_resource, channel_resource TSRMLS_CC);

			php_amqp_zend_throw_exception(res, amqp_channel_exception_class_entry, PHP_AMQP_G(error_message), PHP_AMQP_G(error_code) TSRMLS_CC);
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

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f!|f!", &ack_fci, &ack_fcc, &nack_fci, &nack_fcc) == FAILURE) {
		return;
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

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|d", &timeout) == FAILURE) {
		return;
	}

	if (timeout < 0) {
		zend_throw_exception(amqp_channel_exception_class_entry, "Timeout must be greater than or equal to zero.", 0 TSRMLS_CC);
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


	amqp_method_number_t expected_methods[] = { AMQP_BASIC_ACK_METHOD, AMQP_BASIC_NACK_METHOD, AMQP_BASIC_RETURN_METHOD, 0 };

	while(1) {
		php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);

		status = amqp_simple_wait_method_list_noblock(channel_resource->connection_resource->connection_state, channel_resource->channel_id, expected_methods, &method, tv_ptr);

		if (AMQP_STATUS_TIMEOUT == status) {
			zend_throw_exception(amqp_queue_exception_class_entry, "Wait timeout exceed", 0 TSRMLS_CC);

			php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
			return;
		}

		if (status != AMQP_STATUS_OK) {
			/* Emulate library error */
			amqp_rpc_reply_t res;

			if (AMQP_RESPONSE_SERVER_EXCEPTION == status) {
				res.reply_type = AMQP_RESPONSE_SERVER_EXCEPTION;
				res.reply      = method;
			} else {
				res.reply_type 	  = AMQP_RESPONSE_LIBRARY_EXCEPTION;
				res.library_error = status;
			}

			php_amqp_error(res, &PHP_AMQP_G(error_message), channel_resource->connection_resource, channel_resource TSRMLS_CC);

			php_amqp_zend_throw_exception(res, amqp_channel_exception_class_entry, PHP_AMQP_G(error_message), PHP_AMQP_G(error_code) TSRMLS_CC);
			php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
			return;
		}

		switch(method.id) {
			case AMQP_BASIC_ACK_METHOD:
				status = php_amqp_handle_basic_ack(&PHP_AMQP_G(error_message), channel_resource->connection_resource, channel_resource->channel_id, channel, &method TSRMLS_CC);
				break;
			case AMQP_BASIC_NACK_METHOD:
				status = php_amqp_handle_basic_nack(&PHP_AMQP_G(error_message), channel_resource->connection_resource, channel_resource->channel_id, channel, &method TSRMLS_CC);
				break;
			case AMQP_BASIC_RETURN_METHOD:
				status = php_amqp_handle_basic_return(&PHP_AMQP_G(error_message), channel_resource->connection_resource, channel_resource->channel_id, channel, &method TSRMLS_CC);
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
			res.reply_type 	  = AMQP_RESPONSE_LIBRARY_EXCEPTION;
			res.library_error = status;

			php_amqp_error(res, &PHP_AMQP_G(error_message), channel_resource->connection_resource, channel_resource TSRMLS_CC);

			php_amqp_zend_throw_exception(res, amqp_queue_exception_class_entry, PHP_AMQP_G(error_message), PHP_AMQP_G(error_code) TSRMLS_CC);
			php_amqp_maybe_release_buffers_on_channel(channel_resource->connection_resource, channel_resource);
			return;
		}
	}
}
/* }}} */

/* {{{ proto amqp::getConsumers() */
static PHP_METHOD(amqp_channel_class, getConsumers)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;
	PHP_AMQP_NOPARAMS();
	PHP_AMQP_RETURN_THIS_PROP("consumers");
}
/* }}} */


ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class__construct, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
				ZEND_ARG_OBJ_INFO(0, amqp_connection, AMQPConnection, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_isConnected, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_close, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_getChannelId, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_setPrefetchSize, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
				ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_getPrefetchSize, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_setPrefetchCount, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
				ZEND_ARG_INFO(0, count)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_getPrefetchCount, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_setGlobalPrefetchSize, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
				ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_getGlobalPrefetchSize, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_setGlobalPrefetchCount, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
				ZEND_ARG_INFO(0, count)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_getGlobalPrefetchCount, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_qos, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 2)
				ZEND_ARG_INFO(0, size)
				ZEND_ARG_INFO(0, count)
				ZEND_ARG_INFO(0, global)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_startTransaction, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_commitTransaction, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_rollbackTransaction, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_getConnection, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_basicRecover, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
				ZEND_ARG_INFO(0, requeue)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_confirmSelect, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_setConfirmCallback, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
				ZEND_ARG_INFO(0, ack_callback)
				ZEND_ARG_INFO(0, nack_callback)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_waitForConfirm, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
				ZEND_ARG_INFO(0, timeout)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_setReturnCallback, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
				ZEND_ARG_INFO(0, return_callback)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_waitForBasicReturn, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
				ZEND_ARG_INFO(0, timeout)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_channel_class_getConsumers, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

//setConfirmsCallback


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
	amqp_channel_class_entry = zend_register_internal_class(&ce TSRMLS_CC);

	zend_declare_property_null(this_ce, ZEND_STRL("connection"), ZEND_ACC_PRIVATE TSRMLS_CC);

	zend_declare_property_null(this_ce, ZEND_STRL("prefetch_count"), ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(this_ce, ZEND_STRL("prefetch_size"), 0, ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(this_ce, ZEND_STRL("global_prefetch_count"), ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(this_ce, ZEND_STRL("global_prefetch_size"), ZEND_ACC_PRIVATE TSRMLS_CC);

	zend_declare_property_null(this_ce, ZEND_STRL("consumers"), ZEND_ACC_PRIVATE TSRMLS_CC);

#if PHP_MAJOR_VERSION >=7
	memcpy(&amqp_channel_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

	amqp_channel_object_handlers.offset = XtOffsetOf(amqp_channel_object, zo);
	amqp_channel_object_handlers.free_obj = amqp_channel_free;
#endif

#if ZEND_MODULE_API_NO >= 20100000
	amqp_channel_object_handlers.get_gc = amqp_channel_gc;
#endif

	return SUCCESS;
}
/*
*Local variables:
*tab-width: 4
*c-basic-offset: 4
*End:
*vim600: noet sw=4 ts=4 fdm=marker
*vim<600: noet sw=4 ts=4
*/
