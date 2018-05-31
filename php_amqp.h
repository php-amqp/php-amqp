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
#ifndef PHP_AMQP_H
#define PHP_AMQP_H

/* True global resources - no need for thread safety here */
extern zend_class_entry *amqp_exception_class_entry,
		*amqp_connection_exception_class_entry,
		*amqp_channel_exception_class_entry,
		*amqp_exchange_exception_class_entry,
		*amqp_queue_exception_class_entry,
		*amqp_envelope_exception_class_entry,
		*amqp_value_exception_class_entry;


typedef struct _amqp_connection_resource amqp_connection_resource;
typedef struct _amqp_connection_object amqp_connection_object;
typedef struct _amqp_channel_object amqp_channel_object;
typedef struct _amqp_channel_resource amqp_channel_resource;
typedef struct _amqp_channel_callbacks amqp_channel_callbacks;
typedef struct _amqp_callback_bucket amqp_callback_bucket;

#if PHP_VERSION_ID < 50600
// should never get her, but just in case
#error PHP >= 5.6 required
#endif

#if PHP_MAJOR_VERSION >= 7
	#include "php7_support.h"
#else
	#include "php5_support.h"
#endif

#include "amqp_connection_resource.h"

#include <amqp.h>

extern zend_module_entry amqp_module_entry;
#define phpext_amqp_ptr &amqp_module_entry

#ifdef PHP_WIN32
#define PHP_AMQP_API __declspec(dllexport)
#else
#define PHP_AMQP_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#define AMQP_NOPARAM		0
/* Where is 1?*/
#define AMQP_JUST_CONSUME	1
#define AMQP_DURABLE		2
#define AMQP_PASSIVE		4
#define AMQP_EXCLUSIVE		8
#define AMQP_AUTODELETE		16
#define AMQP_INTERNAL		32
#define AMQP_NOLOCAL		64
#define AMQP_AUTOACK		128
#define AMQP_IFEMPTY		256
#define AMQP_IFUNUSED		512
#define AMQP_MANDATORY		1024
#define AMQP_IMMEDIATE		2048
#define AMQP_MULTIPLE		4096
#define AMQP_NOWAIT			8192
#define AMQP_REQUEUE		16384

/* passive, durable, auto-delete, internal, no-wait (see https://www.rabbitmq.com/amqp-0-9-1-reference.html#exchange.declare) */
#define PHP_AMQP_EXCHANGE_FLAGS     (AMQP_PASSIVE | AMQP_DURABLE | AMQP_AUTODELETE | AMQP_INTERNAL)

/* passive, durable, exclusive, auto-delete, no-wait (see https://www.rabbitmq.com/amqp-0-9-1-reference.html#queue.declare) */
/* We don't support no-wait flag */
#define PHP_AMQP_QUEUE_FLAGS        (AMQP_PASSIVE | AMQP_DURABLE | AMQP_EXCLUSIVE | AMQP_AUTODELETE)

#define AMQP_EX_TYPE_DIRECT		"direct"
#define AMQP_EX_TYPE_FANOUT		"fanout"
#define AMQP_EX_TYPE_TOPIC		"topic"
#define AMQP_EX_TYPE_HEADERS	"headers"

#define PHP_AMQP_CONNECTION_RES_NAME "AMQP Connection Resource"

struct _amqp_channel_resource {
	char is_connected;
	amqp_channel_t channel_id;
	amqp_connection_resource *connection_resource;
    amqp_channel_object *parent;
};

struct _amqp_callback_bucket {
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
};

struct _amqp_channel_callbacks {
	amqp_callback_bucket basic_return;
	amqp_callback_bucket basic_ack;
	amqp_callback_bucket basic_nack;
};


/* NOTE: due to how internally PHP works with custom object, zend_object position in structure matters */
struct _amqp_channel_object {
#if PHP_MAJOR_VERSION >= 7
	amqp_channel_callbacks callbacks;
	zval *gc_data;
    int   gc_data_count;
	amqp_channel_resource *channel_resource;
	zend_object zo;
#else
	zend_object zo;
	zval *this_ptr;
	amqp_channel_resource *channel_resource;
	amqp_channel_callbacks callbacks;
	zval  **gc_data;
	long    gc_data_count;
#endif
};

struct _amqp_connection_resource {
	zend_bool is_connected;
	zend_bool is_persistent;
	zend_bool is_dirty;
	PHP5to7_zend_resource_t resource;
	amqp_connection_object *parent;
	amqp_channel_t max_slots;
	amqp_channel_t used_slots;
	amqp_channel_resource **slots;
	amqp_connection_state_t connection_state;
	amqp_socket_t *socket;
};

struct _amqp_connection_object {
#if PHP_MAJOR_VERSION >= 7
	amqp_connection_resource *connection_resource;
	zend_object zo;
#else
	zend_object zo;
	amqp_connection_resource *connection_resource;
#endif
};

#define DEFAULT_PORT						"5672"		/* default AMQP port */
#define DEFAULT_HOST						"localhost"
#define DEFAULT_TIMEOUT						""
#define DEFAULT_READ_TIMEOUT				"0"
#define DEFAULT_WRITE_TIMEOUT				"0"
#define DEFAULT_CONNECT_TIMEOUT				"0"
#define DEFAULT_VHOST						"/"
#define DEFAULT_LOGIN						"guest"
#define DEFAULT_PASSWORD					"guest"
#define DEFAULT_AUTOACK						"0"			/* These are all strings to facilitate setting default ini values */
#define DEFAULT_PREFETCH_COUNT				"3"
#define DEFAULT_SASL_METHOD                 "0"

/* Usually, default is 0 which means 65535, but underlying rabbitmq-c library pool allocates minimal pool for each channel,
 * so it takes a lot of memory to keep all that channels. Even after channel closing that buffer still keep memory allocation.
 */
/* #define DEFAULT_CHANNELS_PER_CONNECTION AMQP_DEFAULT_MAX_CHANNELS */
#define PHP_AMQP_PROTOCOL_MAX_CHANNELS 256

/* AMQP_DEFAULT_FRAME_SIZE 131072 */

#if PHP_AMQP_PROTOCOL_MAX_CHANNELS > 0
	#define PHP_AMQP_MAX_CHANNELS PHP_AMQP_PROTOCOL_MAX_CHANNELS
#else
	#define PHP_AMQP_MAX_CHANNELS 65535 // Note that the maximum number of channels the protocol supports is 65535 (2^16, with the 0-channel reserved)
#endif

#define PHP_AMQP_MAX_FRAME INT_MAX
#define PHP_AMQP_MAX_HEARTBEAT INT_MAX

#define PHP_AMQP_DEFAULT_CHANNEL_MAX PHP_AMQP_MAX_CHANNELS
#define PHP_AMQP_DEFAULT_FRAME_MAX AMQP_DEFAULT_FRAME_SIZE
#define PHP_AMQP_DEFAULT_HEARTBEAT AMQP_DEFAULT_HEARTBEAT

#define PHP_AMQP_STRINGIFY(value) PHP_AMQP_TO_STRING(value)
#define PHP_AMQP_TO_STRING(value) #value


#define DEFAULT_CHANNEL_MAX					PHP_AMQP_STRINGIFY(PHP_AMQP_MAX_CHANNELS)
#define DEFAULT_FRAME_MAX					PHP_AMQP_STRINGIFY(PHP_AMQP_DEFAULT_FRAME_MAX)
#define DEFAULT_HEARTBEAT					PHP_AMQP_STRINGIFY(PHP_AMQP_DEFAULT_HEARTBEAT)
#define DEFAULT_CACERT	""
#define DEFAULT_CERT	""
#define DEFAULT_KEY	""
#define DEFAULT_VERIFY	"1"


#define IS_PASSIVE(bitmask)		(AMQP_PASSIVE & (bitmask)) ? 1 : 0
#define IS_DURABLE(bitmask)		(AMQP_DURABLE & (bitmask)) ? 1 : 0
#define IS_EXCLUSIVE(bitmask)	(AMQP_EXCLUSIVE & (bitmask)) ? 1 : 0
#define IS_AUTODELETE(bitmask)	(AMQP_AUTODELETE & (bitmask)) ? 1 : 0
#define IS_INTERNAL(bitmask)	(AMQP_INTERNAL & (bitmask)) ? 1 : 0
#define IS_NOWAIT(bitmask)		(AMQP_NOWAIT & (bitmask)) ? 1 : 0 /* NOTE: always 0 in rabbitmq-c internals, so don't use it unless you are clearly understand aftermath*/

#define PHP_AMQP_NOPARAMS() if (zend_parse_parameters_none() == FAILURE) { return; }

#define PHP_AMQP_RETURN_THIS_PROP(prop_name) \
    zval * _zv = zend_read_property(this_ce, getThis(), ZEND_STRL(prop_name), 0 PHP5to7_READ_PROP_RV_PARAM_CC TSRMLS_CC); \
    RETURN_ZVAL(_zv, 1, 0);

#define PHP_AMQP_READ_OBJ_PROP(cls, obj, name) zend_read_property((cls), (obj), ZEND_STRL(name), 0 PHP5to7_READ_PROP_RV_PARAM_CC TSRMLS_CC)
#define PHP_AMQP_READ_OBJ_PROP_DOUBLE(cls, obj, name) Z_DVAL_P(PHP_AMQP_READ_OBJ_PROP((cls), (obj), (name)))

#define PHP_AMQP_READ_THIS_PROP_CE(name, ce) zend_read_property((ce), getThis(), ZEND_STRL(name), 0 PHP5to7_READ_PROP_RV_PARAM_CC TSRMLS_CC)
#define PHP_AMQP_READ_THIS_PROP(name) zend_read_property(this_ce, getThis(), ZEND_STRL(name), 0 PHP5to7_READ_PROP_RV_PARAM_CC TSRMLS_CC)
#define PHP_AMQP_READ_THIS_PROP_BOOL(name) Z_BVAL_P(PHP_AMQP_READ_THIS_PROP(name))
#define PHP_AMQP_READ_THIS_PROP_STR(name) Z_STRVAL_P(PHP_AMQP_READ_THIS_PROP(name))
#define PHP_AMQP_READ_THIS_PROP_STRLEN(name) (Z_TYPE_P(PHP_AMQP_READ_THIS_PROP(name)) == IS_STRING ? Z_STRLEN_P(PHP_AMQP_READ_THIS_PROP(name)) : 0)
#define PHP_AMQP_READ_THIS_PROP_ARR(name) Z_ARRVAL_P(PHP_AMQP_READ_THIS_PROP(name))
#define PHP_AMQP_READ_THIS_PROP_LONG(name) Z_LVAL_P(PHP_AMQP_READ_THIS_PROP(name))
#define PHP_AMQP_READ_THIS_PROP_DOUBLE(name) Z_DVAL_P(PHP_AMQP_READ_THIS_PROP(name))


#if PHP_MAJOR_VERSION >= 7
	static inline amqp_connection_object *php_amqp_connection_object_fetch(zend_object *obj) {
		return (amqp_connection_object *)((char *)obj - XtOffsetOf(amqp_connection_object, zo));
	}

	static inline amqp_channel_object *php_amqp_channel_object_fetch(zend_object *obj) {
		return (amqp_channel_object *)((char *)obj - XtOffsetOf(amqp_channel_object, zo));
	}

	#define PHP_AMQP_GET_CONNECTION(obj) php_amqp_connection_object_fetch(Z_OBJ_P(obj))
	#define PHP_AMQP_GET_CHANNEL(obj) php_amqp_channel_object_fetch(Z_OBJ_P(obj))

	#define PHP_AMQP_FETCH_CONNECTION(obj) php_amqp_connection_object_fetch(obj)
	#define PHP_AMQP_FETCH_CHANNEL(obj) php_amqp_channel_object_fetch(obj)

#else
	#define PHP_AMQP_GET_CONNECTION(obj) (amqp_connection_object *)zend_object_store_get_object((obj) TSRMLS_CC)
	#define PHP_AMQP_GET_CHANNEL(obj) (amqp_channel_object *)zend_object_store_get_object((obj) TSRMLS_CC)

	#define PHP_AMQP_FETCH_CONNECTION(obj) (amqp_connection_object*)(obj)
	#define PHP_AMQP_FETCH_CHANNEL(obj) (amqp_channel_object*)(obj)
#endif


#define PHP_AMQP_GET_CHANNEL_RESOURCE(obj) (IS_OBJECT == Z_TYPE_P(obj) ? (PHP_AMQP_GET_CHANNEL(obj))->channel_resource : NULL)

#define PHP_AMQP_VERIFY_CONNECTION_ERROR(error, reason) \
		char verify_connection_error_tmp[255]; \
		snprintf(verify_connection_error_tmp, 255, "%s %s", error, reason); \
		zend_throw_exception(amqp_connection_exception_class_entry, verify_connection_error_tmp, 0 TSRMLS_CC); \
		return; \

#define PHP_AMQP_VERIFY_CONNECTION(connection, error) \
	if (!connection) { \
		PHP_AMQP_VERIFY_CONNECTION_ERROR(error, "Stale reference to the connection object.") \
	} \
	if (!(connection)->connection_resource || !(connection)->connection_resource->is_connected) { \
		PHP_AMQP_VERIFY_CONNECTION_ERROR(error, "No connection available.") \
	} \

#define PHP_AMQP_VERIFY_CHANNEL_ERROR(error, reason) \
		char verify_channel_error_tmp[255]; \
		snprintf(verify_channel_error_tmp, 255, "%s %s", error, reason); \
		zend_throw_exception(amqp_channel_exception_class_entry, verify_channel_error_tmp, 0 TSRMLS_CC); \
		return; \

#define PHP_AMQP_VERIFY_CHANNEL_RESOURCE(resource, error) \
	if (!resource) { \
		PHP_AMQP_VERIFY_CHANNEL_ERROR(error, "Stale reference to the channel object.") \
	} \
	if (!(resource)->is_connected) { \
		PHP_AMQP_VERIFY_CHANNEL_ERROR(error, "No channel available.") \
	} \
	if (!(resource)->connection_resource) { \
		PHP_AMQP_VERIFY_CONNECTION_ERROR(error, "Stale reference to the connection object.") \
	} \
	if (!(resource)->connection_resource->is_connected) { \
		PHP_AMQP_VERIFY_CONNECTION_ERROR(error, "No connection available.") \
	} \

#define PHP_AMQP_VERIFY_CHANNEL_CONNECTION_RESOURCE(resource, error) \
	if (!resource) { \
		PHP_AMQP_VERIFY_CHANNEL_ERROR(error, "Stale reference to the channel object.") \
	} \
	if (!(resource)->connection_resource) { \
		PHP_AMQP_VERIFY_CONNECTION_ERROR(error, "Stale reference to the connection object.") \
	} \
	if (!(resource)->connection_resource->is_connected) { \
		PHP_AMQP_VERIFY_CONNECTION_ERROR(error, "No connection available.") \
	} \

#define PHP_AMQP_MAYBE_ERROR(res, channel_resource) (\
	  (AMQP_RESPONSE_NORMAL != (res).reply_type) \
	&& \
	  PHP_AMQP_RESOURCE_RESPONSE_OK != php_amqp_error(res, &PHP_AMQP_G(error_message), (channel_resource)->connection_resource, (channel_resource) TSRMLS_CC) \
	)

#define PHP_AMQP_MAYBE_ERROR_RECOVERABLE(res, channel_resource) (\
	  (AMQP_RESPONSE_NORMAL != (res).reply_type) \
	&& \
	  PHP_AMQP_RESOURCE_RESPONSE_OK != php_amqp_error_advanced(res, &PHP_AMQP_G(error_message), (channel_resource)->connection_resource, (channel_resource), 0 TSRMLS_CC) \
	)

#define PHP_AMQP_IS_ERROR_RECOVERABLE(res, channel_resource, channel_object) ( \
	AMQP_RESPONSE_LIBRARY_EXCEPTION == (res).reply_type && AMQP_STATUS_UNEXPECTED_STATE == (res).library_error \
	&& (0 <= php_amqp_connection_resource_error_advanced(res, &PHP_AMQP_G(error_message), (channel_resource)->connection_resource, (amqp_channel_t)(channel_resource ? (channel_resource)->channel_id : 0), (channel_object) TSRMLS_CC)) \
)


#if ZEND_MODULE_API_NO >= 20100000
	#define AMQP_OBJECT_PROPERTIES_INIT(obj, ce) object_properties_init(&(obj), ce);
#else
	#define AMQP_OBJECT_PROPERTIES_INIT(obj, ce) \
		do { \
			zval *tmp; \
			zend_hash_copy((obj).properties, &(ce)->default_properties, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *)); \
		} while (0);
#endif


#define AMQP_ERROR_CATEGORY_MASK (1 << 29)


#ifdef PHP_WIN32
# define AMQP_OS_SOCKET_TIMEOUT_ERRNO AMQP_ERROR_CATEGORY_MASK | WSAETIMEDOUT
#else
# define AMQP_OS_SOCKET_TIMEOUT_ERRNO AMQP_ERROR_CATEGORY_MASK | EAGAIN
#endif

ZEND_BEGIN_MODULE_GLOBALS(amqp)
    char *error_message;
    PHP5to7_param_long_type_t error_code;
ZEND_END_MODULE_GLOBALS(amqp)

ZEND_EXTERN_MODULE_GLOBALS(amqp);

#ifdef ZEND_MODULE_GLOBALS_ACCESSOR
	#define PHP_AMQP_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(amqp, v)

	#if defined(ZTS) && defined(COMPILE_DL_WEAK)
		ZEND_TSRMLS_CACHE_EXTERN();
	#endif
#else
	#ifdef ZTS
		#define PHP_AMQP_G(v) TSRMG(amqp_globals_id, zend_amqp_globals *, v)
	#else
		#define PHP_AMQP_G(v) (amqp_globals.v)
	#endif
#endif

#ifndef PHP_AMQP_VERSION
#define PHP_AMQP_VERSION "1.9.4-dev"
#endif

#ifndef PHP_AMQP_REVISION
#define PHP_AMQP_REVISION "release"
#endif

int php_amqp_error(amqp_rpc_reply_t reply, char **message, amqp_connection_resource *connection_resource, amqp_channel_resource *channel_resource TSRMLS_DC);
int php_amqp_error_advanced(amqp_rpc_reply_t reply, char **message, amqp_connection_resource *connection_resource, amqp_channel_resource *channel_resource, int fail_on_errors TSRMLS_DC);

/**
 * @deprecated
 */
void php_amqp_zend_throw_exception(amqp_rpc_reply_t reply, zend_class_entry *exception_ce, const char *message, PHP5to7_param_long_type_t code TSRMLS_DC);
void php_amqp_zend_throw_exception_short(amqp_rpc_reply_t reply, zend_class_entry *exception_ce TSRMLS_DC);
void php_amqp_maybe_release_buffers_on_channel(amqp_connection_resource *connection_resource, amqp_channel_resource *channel_resource);

#endif	/* PHP_AMQP_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
