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

#include "amqp.h"

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
#define PHP_AMQP_QUEUE_FLAGS        (AMQP_PASSIVE | AMQP_DURABLE | AMQP_EXCLUSIVE | AMQP_AUTODELETE | AMQP_EXCLUSIVE)

#define AMQP_EX_TYPE_DIRECT		"direct"
#define AMQP_EX_TYPE_FANOUT		"fanout"
#define AMQP_EX_TYPE_TOPIC		"topic"
#define AMQP_EX_TYPE_HEADERS	"headers"

#define PHP_AMQP_CONNECTION_RES_NAME "AMQP Connection Resource"

PHP_MINIT_FUNCTION(amqp);
PHP_MSHUTDOWN_FUNCTION(amqp);
PHP_MINFO_FUNCTION(amqp);

amqp_table_t *convert_zval_to_amqp_table(zval *zvalArguments TSRMLS_DC);
void php_amqp_free_amqp_table(amqp_table_t * table);

char *stringify_bytes(amqp_bytes_t bytes);

/* True global resources - no need for thread safety here */
extern zend_class_entry *amqp_connection_class_entry;
extern zend_class_entry *amqp_channel_class_entry;
extern zend_class_entry *amqp_queue_class_entry;
extern zend_class_entry *amqp_exchange_class_entry;
extern zend_class_entry *amqp_envelope_class_entry;

extern zend_class_entry *amqp_exception_class_entry,
	*amqp_connection_exception_class_entry,
	*amqp_channel_exception_class_entry,
	*amqp_exchange_exception_class_entry,
	*amqp_queue_exception_class_entry;

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

/* Usually, default is 0 which means 65535, but underlying rabbitmq-c library pool allocates minimal pool for each channel,
 * so it takes a lot of memory to keep all that channels. Even after channel closing that buffer still keep memory allocation.
 */
/* #define DEFAULT_CHANNELS_PER_CONNECTION AMQP_DEFAULT_MAX_CHANNELS */
#define PHP_AMQP_PROTOCOL_MAX_CHANNELS 256

//AMQP_DEFAULT_FRAME_SIZE 131072

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

#define AMQP_READ_SUCCESS					1
#define AMQP_READ_NO_MESSAGES				0
#define AMQP_READ_ERROR						-1

#define IS_PASSIVE(bitmask)		(AMQP_PASSIVE & (bitmask)) ? 1 : 0
#define IS_DURABLE(bitmask)		(AMQP_DURABLE & (bitmask)) ? 1 : 0
#define IS_EXCLUSIVE(bitmask)	(AMQP_EXCLUSIVE & (bitmask)) ? 1 : 0
#define IS_AUTODELETE(bitmask)	(AMQP_AUTODELETE & (bitmask)) ? 1 : 0
#define IS_INTERNAL(bitmask)	(AMQP_INTERNAL & (bitmask)) ? 1 : 0
#define IS_NOWAIT(bitmask)		(AMQP_NOWAIT & (bitmask)) ? 1 : 0 /* NOTE: always 0 in rabbitmq-c internals, so don't use it unless you are clearly understand aftermath*/

#define AMQP_SET_NAME(object, str) \
	(object)->name_len = strlen(str) >= sizeof((object)->name) ? sizeof((object)->name) - 1 : strlen(str); \
	strncpy((object)->name, name, (object)->name_len); \
	(object)->name[(object)->name_len] = '\0';

#define AMQP_SET_TYPE(object, str) \
	(object)->type_len = strlen(str) >= sizeof((object)->type) ? sizeof((object)->type) - 1 : strlen(str); \
	strncpy((object)->type, type, (object)->type_len); \
	(object)->type[(object)->type_len] = '\0';

#define AMQP_SET_LONG_PROPERTY(object, value) \
	(object) = (value);

#define AMQP_SET_BOOL_PROPERTY(object, value) \
	(object) = (value) == 0 ? 0 : 1;

#define AMQP_SET_STR_PROPERTY(object, str, len) \
	strncpy((object), (str), (len) >= sizeof(object) ? sizeof(object) - 1 : (len)); \
	(object)[(len) >= sizeof(object) ? sizeof(object) - 1 : (len)] = '\0';

#define AMQP_VERIFY_CHANNEL_ERROR(error, reason) \
		char verify_channel_error_tmp[255]; \
		snprintf(verify_channel_error_tmp, 255, "%s %s", error, reason); \
		zend_throw_exception(amqp_channel_exception_class_entry, verify_channel_error_tmp, 0 TSRMLS_CC); \
		return; \

#define AMQP_VERIFY_CHANNEL(channel, error) \
	if (!channel) { \
		AMQP_VERIFY_CHANNEL_ERROR(error, "Stale reference to the channel object.") \
	} \
	if ((channel)->is_connected != '\1') { \
		AMQP_VERIFY_CHANNEL_ERROR(error, "No channel available.") \
	} \

#define AMQP_VERIFY_CONNECTION_ERROR(error, reason) \
		char verify_connection_error_tmp[255]; \
		snprintf(verify_connection_error_tmp, 255, "%s %s", error, reason); \
		zend_throw_exception(amqp_connection_exception_class_entry, verify_connection_error_tmp, 0 TSRMLS_CC); \
		return; \

#define AMQP_VERIFY_CONNECTION(connection, error) \
	if (!connection) { \
		AMQP_VERIFY_CONNECTION_ERROR(error, "Stale reference to the connection object.") \
	} \
	if ((connection)->is_connected != '\1') { \
		AMQP_VERIFY_CONNECTION_ERROR(error, "No connection available.") \
	} \

#define PHP_AMQP_ERROR_MESSAGE_PTR  &php_amqp_internal_error_message
#define PHP_AMQP_ERROR_MESSAGE       php_amqp_internal_error_message

#define PHP_AMQP_INIT_ERROR_MESSAGE()\
	char *PHP_AMQP_ERROR_MESSAGE = NULL;

#define PHP_AMQP_DESTROY_ERROR_MESSAGE()\
	if (PHP_AMQP_ERROR_MESSAGE != NULL) { efree(PHP_AMQP_ERROR_MESSAGE); }

#define AMQP_OBJECT_PROPERTIES_INIT(obj, ce) object_properties_init(&obj, ce);

typedef struct _amqp_channel_object {
	zval connection;
	amqp_channel_t channel_id;
	char is_connected;
	int prefetch_count;
	int prefetch_size;
	zend_object zo;
} amqp_channel_object;

static inline amqp_channel_object * amqp_channel_object_fetch_object(zend_object *obj) {
	return (amqp_channel_object *)((char *)obj - XtOffsetOf(amqp_channel_object, zo));
}

#define AMQP_CHANNEL_OBJ_P(zv) amqp_channel_object_fetch_object(Z_OBJ_P(zv));
#define AMQP_CHANNEL_OBJ(zv) amqp_channel_object_fetch_object(Z_OBJ(zv));

typedef struct _amqp_connection_resource {
	zend_bool is_connected;
	zend_resource* resource;
	amqp_channel_t max_slots;
	amqp_channel_t used_slots;
	amqp_channel_object **slots;
	zend_string* resource_key;
	amqp_connection_state_t connection_state;
	amqp_socket_t *socket;
} amqp_connection_resource;

typedef struct _amqp_connection_object {
	char is_connected;
	char is_persistent;
	char *login;
	char *password;
	char *host;
	char *vhost;
	int port;
	int channel_max;
	int frame_max;
	int heartbeat;
	double read_timeout;
	double write_timeout;
	double connect_timeout;
	amqp_connection_resource *connection_resource;
	zend_object zo;
} amqp_connection_object;

static inline amqp_connection_object * amqp_connection_object_fetch_object(zend_object *obj) {
	return (amqp_connection_object *)((char *)obj - XtOffsetOf(amqp_connection_object, zo));
}

#define Z_AMQP_CONNECTION_OBJ_P(zv) amqp_connection_object_fetch_object(Z_OBJ_P(zv))
#define Z_AMQP_CONNECTION_OBJ(zv) amqp_connection_object_fetch_object(Z_OBJ(zv))

typedef struct _amqp_queue_object {
	zval channel;
	char name[256];
	int name_len;
	char consumer_tag[256];
	int consumer_tag_len;
	int flags;
	zval arguments;
	zend_object zo;
} amqp_queue_object;

static inline amqp_queue_object * amqp_queue_object_fetch_object(zend_object *obj) {
	return (amqp_queue_object *)((char *)obj - XtOffsetOf(amqp_queue_object, zo));
}

#define AMQP_QUEUE_OBJ_P(zv) amqp_queue_object_fetch_object(Z_OBJ_P(zv));
#define AMQP_QUEUE_OBJ(zv) amqp_queue_object_fetch_object(Z_OBJ(zv));

typedef struct _amqp_exchange_object {
	zval channel;
	char is_connected;
	char name[256];
	int name_len;
	char type[256];
	int type_len;
	int flags;
	zval arguments;
	zend_object zo;
} amqp_exchange_object;

static inline amqp_exchange_object * amqp_exchange_object_fetch_object(zend_object *obj) {
	return (amqp_exchange_object *)((char *)obj - XtOffsetOf(amqp_exchange_object, zo));
}

#define AMQP_EXCHANGE_OBJ_P(zv) amqp_exchange_object_fetch_object(Z_OBJ_P(zv));

typedef struct _amqp_envelope_object {
	char *body;
	size_t body_len;
	char routing_key[256];
	uint delivery_tag;
	int delivery_mode;
	char exchange_name[256];
	int is_redelivery;
	char content_type[256];
	char content_encoding[256];
	char type[256];
	long timestamp;
	int priority;
	char expiration[256];
	char user_id[256];
	char app_id[256];
	char message_id[256];
	char reply_to[256];
	char correlation_id[256];
	zval headers;
	zend_object zo;
} amqp_envelope_object;

static inline amqp_envelope_object * amqp_envelope_object_fetch_object(zend_object *obj) {
	return (amqp_envelope_object *)((char *)obj - XtOffsetOf(amqp_envelope_object, zo));
}

#define AMQP_ENVELOPE_OBJ_P(zv) amqp_envelope_object_fetch_object(Z_OBJ_P(zv));
#define AMQP_ENVELOPE_OBJ(zv) amqp_envelope_object_fetch_object(Z_OBJ(zv));

#define AMQP_ERROR_CATEGORY_MASK (1 << 29)

#ifdef PHP_WIN32
# define AMQP_RPC_REPLY_T_CAST
#else
# define AMQP_RPC_REPLY_T_CAST (amqp_rpc_reply_t)
#endif

#ifdef PHP_WIN32
# define AMQP_CLOSE_SOCKET(fd) closesocket(fd);
#else
# define AMQP_CLOSE_SOCKET(fd) close(fd);
#endif

#ifdef PHP_WIN32
# define AMQP_OS_SOCKET_TIMEOUT_ERRNO AMQP_ERROR_CATEGORY_MASK | WSAETIMEDOUT
#else
# define AMQP_OS_SOCKET_TIMEOUT_ERRNO AMQP_ERROR_CATEGORY_MASK | EAGAIN
#endif

#ifdef ZTS
#define AMQP_G(v) TSRMG(amqp_globals_id, zend_amqp_globals *, v)
#else
#define AMQP_G(v) (amqp_globals.v)
#endif

#ifndef PHP_AMQP_VERSION
#define PHP_AMQP_VERSION "1.6.0beta3"
#endif

#ifndef PHP_AMQP_REVISION
#define PHP_AMQP_REVISION "release"
#endif

void php_amqp_error(amqp_rpc_reply_t reply, char **message, amqp_connection_object *connection, amqp_channel_object *channel TSRMLS_DC);
void php_amqp_zend_throw_exception(amqp_rpc_reply_t reply, zend_class_entry *exception_ce, const char *message, long code TSRMLS_DC);
void php_amqp_maybe_release_buffers_on_channel(amqp_connection_object *connection, amqp_channel_object *channel);
amqp_bytes_t php_amqp_long_string(const char *cstr, size_t len);

static inline amqp_bytes_t php_amqp_zend_string(zend_string* str) {
	amqp_bytes_t bytes_t;
	bytes_t.len = str->len;
	bytes_t.bytes = str->val;
	return bytes_t;
}

static inline amqp_bytes_t php_amqp_zend_string_copy(zend_string* str) {
	if (str->len < 1) {
		return amqp_empty_bytes;
	}

	amqp_bytes_t bytes_t;
	bytes_t.len = str->len;
	bytes_t.bytes = estrndup(str->val, str->len);
	return bytes_t;
}

static inline void php_amqp_safe_free_bytes(amqp_bytes_t* bytes) {
	if(bytes->bytes != NULL) {
		efree(bytes->bytes);
		bytes->bytes = NULL;
	}
}

#endif	/* PHP_AMQP_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
