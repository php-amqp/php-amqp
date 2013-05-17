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

/* $Id: php_amqp.h 327551 2012-09-09 03:49:34Z pdezwart $ */

#ifndef PHP_AMQP_H
#define PHP_AMQP_H



/* Add pseudo refcount macros for PHP version < 5.3 */
#ifndef Z_REFCOUNT_PP

#define Z_REFCOUNT_PP(ppz)				Z_REFCOUNT_P(*(ppz))
#define Z_SET_REFCOUNT_PP(ppz, rc)		Z_SET_REFCOUNT_P(*(ppz), rc)
#define Z_ADDREF_PP(ppz)				Z_ADDREF_P(*(ppz))
#define Z_DELREF_PP(ppz)				Z_DELREF_P(*(ppz))
#define Z_ISREF_PP(ppz)					Z_ISREF_P(*(ppz))
#define Z_SET_ISREF_PP(ppz)				Z_SET_ISREF_P(*(ppz))
#define Z_UNSET_ISREF_PP(ppz)			Z_UNSET_ISREF_P(*(ppz))
#define Z_SET_ISREF_TO_PP(ppz, isref)	Z_SET_ISREF_TO_P(*(ppz), isref)

#define Z_REFCOUNT_P(pz)				zval_refcount_p(pz)
#define Z_SET_REFCOUNT_P(pz, rc)		zval_set_refcount_p(pz, rc)
#define Z_ADDREF_P(pz)					zval_addref_p(pz)
#define Z_DELREF_P(pz)					zval_delref_p(pz)
#define Z_ISREF_P(pz)					zval_isref_p(pz)
#define Z_SET_ISREF_P(pz)				zval_set_isref_p(pz)
#define Z_UNSET_ISREF_P(pz)				zval_unset_isref_p(pz)
#define Z_SET_ISREF_TO_P(pz, isref)		zval_set_isref_to_p(pz, isref)

#define Z_REFCOUNT(z)					Z_REFCOUNT_P(&(z))
#define Z_SET_REFCOUNT(z, rc)			Z_SET_REFCOUNT_P(&(z), rc)
#define Z_ADDREF(z)						Z_ADDREF_P(&(z))
#define Z_DELREF(z)						Z_DELREF_P(&(z))
#define Z_ISREF(z)						Z_ISREF_P(&(z))
#define Z_SET_ISREF(z)					Z_SET_ISREF_P(&(z))
#define Z_UNSET_ISREF(z)				Z_UNSET_ISREF_P(&(z))
#define Z_SET_ISREF_TO(z, isref)		Z_SET_ISREF_TO_P(&(z), isref)

#if defined(__GNUC__)
#define zend_always_inline inline __attribute__((always_inline))
#elif defined(_MSC_VER)
#define zend_always_inline __forceinline
#else
#define zend_always_inline inline
#endif

static zend_always_inline zend_uint zval_refcount_p(zval* pz) {
	return pz->refcount;
}

static zend_always_inline zend_uint zval_set_refcount_p(zval* pz, zend_uint rc) {
	return pz->refcount = rc;
}

static zend_always_inline zend_uint zval_addref_p(zval* pz) {
	return ++pz->refcount;
}

static zend_always_inline zend_uint zval_delref_p(zval* pz) {
	return --pz->refcount;
}

static zend_always_inline zend_bool zval_isref_p(zval* pz) {
	return pz->is_ref;
}

static zend_always_inline zend_bool zval_set_isref_p(zval* pz) {
	return pz->is_ref = 1;
}

static zend_always_inline zend_bool zval_unset_isref_p(zval* pz) {
	return pz->is_ref = 0;
}

static zend_always_inline zend_bool zval_set_isref_to_p(zval* pz, zend_bool isref) {
	return pz->is_ref = isref;
}

#else

#define PHP_ATLEAST_5_3   true

#endif


#include "amqp.h"
#include "amqp_object_store.h"

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

#define AMQP_EX_TYPE_DIRECT		"direct"
#define AMQP_EX_TYPE_FANOUT		"fanout"
#define AMQP_EX_TYPE_TOPIC		"topic"
#define AMQP_EX_TYPE_HEADERS	"headers"

#define PHP_AMQP_CONNECTION_RES_NAME "AMQP Connection Resource"

PHP_MINIT_FUNCTION(amqp);
PHP_MSHUTDOWN_FUNCTION(amqp);
PHP_MINFO_FUNCTION(amqp);

void amqp_error(amqp_rpc_reply_t x, char ** pstr);
amqp_table_t *convert_zval_to_arguments(zval *zvalArguments);
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


#define FRAME_MAX							131072		/* max length (size) of frame */
#define CHANNEL_MAX							10			/* max number of channels allowed */
#define HEADER_FOOTER_SIZE					8			/*  7 bytes up front, then payload, then 1 byte footer */
#define AMQP_HEARTBEAT						0	   		/* heartbeat */

#define DEFAULT_NUM_CONNECTION_CHANNELS		255

#define DEFAULT_PORT						"5672"		/* default AMQP port */
#define DEFAULT_HOST						"localhost"
#define DEFAULT_TIMEOUT						NULL
#define DEFAULT_READ_TIMEOUT				"0"
#define DEFAULT_WRITE_TIMEOUT				"0"
#define DEFAULT_VHOST						"/"
#define DEFAULT_LOGIN						"guest"
#define DEFAULT_PASSWORD					"guest"
#define DEFAULT_AUTOACK						"0"			/* These are all strings to facilitate setting default ini values */
#define DEFAULT_PREFETCH_COUNT				"3"
#define DEFAULT_CHANNELS_PER_CONNECTION 	255

#define AMQP_READ_SUCCESS					1
#define AMQP_READ_NO_MESSAGES				0
#define AMQP_READ_ERROR						-1


#define EMPTY_ARGUMENTS			{0, NULL};
#define IS_PASSIVE(bitmask)		(AMQP_PASSIVE & (bitmask)) ? 1 : 0;
#define IS_DURABLE(bitmask)		(AMQP_DURABLE & (bitmask)) ? 1 : 0;
#define IS_EXCLUSIVE(bitmask)	(AMQP_EXCLUSIVE & (bitmask)) ? 1 : 0;
#define IS_AUTODELETE(bitmask)	(AMQP_AUTODELETE & (bitmask)) ? 1 : 0;
#define IS_NOWAIT(bitmask)		(AMQP_NOWAIT & (bitmask)) ? 1 : 0;



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

#define AMQP_EFREE_ARGUMENTS(object) \
	if ((object)->entries) { \
		int macroEntryCounter; \
		for (macroEntryCounter = 0; macroEntryCounter < (object)->num_entries; macroEntryCounter++) { \
			efree((object)->entries[macroEntryCounter].key.bytes); \
			if ((object)->entries[macroEntryCounter].value.kind == AMQP_FIELD_KIND_UTF8) { \
				efree((object)->entries[macroEntryCounter].value.value.bytes.bytes); \
			} \
		} \
		efree((object)->entries); \
	} \
	efree(object); \

#define AMQP_GET_CHANNEL(object) \
	(amqp_channel_object *) amqp_object_store_get_valid_object((object)->channel TSRMLS_CC);

#define AMQP_ASSIGN_CHANNEL(channel, object) \
	if (!(object)->channel) { \
		return; \
	} \
	channel = AMQP_GET_CHANNEL(object)

#define AMQP_GET_CONNECTION(object) \
	(amqp_connection_object *) amqp_object_store_get_valid_object((object)->connection TSRMLS_CC);

#define AMQP_ASSIGN_CONNECTION(connection, object) \
	if (!(object)->connection) { \
		return; \
	} \
	connection = AMQP_GET_CONNECTION(object)


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

#if ZEND_MODULE_API_NO >= 20100000
	#define AMQP_OBJECT_PROPERTIES_INIT(obj, ce) object_properties_init(&obj, ce);
#else
	#define AMQP_OBJECT_PROPERTIES_INIT(obj, ce) \
		do { \
			zval *tmp; \
			zend_hash_copy((obj).properties, &(ce)->default_properties, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *)); \
		} while (0);
#endif

extern int le_amqp_connection_resource;

typedef struct _amqp_channel_object {
	zend_object zo;
	zval *connection;
	int channel_id;
	char is_connected;
	int prefetch_count;
	int prefetch_size;
} amqp_channel_object;

typedef struct _amqp_connection_resource {
	int used_slots;
	amqp_channel_object **slots;
	int fd;
	int is_persistent;
	amqp_connection_state_t connection_state;
} amqp_connection_resource;

typedef struct _amqp_connection_object {
	zend_object zo;
	char is_connected;
	char *login;
	int login_len;
	char *password;
	int password_len;
	char *host;
	int host_len;
	char *vhost;
	int vhost_len;
	int port;
	double read_timeout;
	double write_timeout;
	amqp_connection_resource *connection_resource;
} amqp_connection_object;

typedef struct _amqp_queue_object {
	zend_object zo;
	zval *channel;
	char is_connected;
	char name[255];
	int name_len;
	char consumer_tag[255];
	int consumer_tag_len;
	int passive; /* @TODO: consider making these bit fields */
	int durable;
	int exclusive;
	int auto_delete; /* end @TODO */
	zval *arguments;
} amqp_queue_object;


typedef struct _amqp_exchange_object {
	zend_object zo;
	zval *channel;
	char is_connected;
	char name[255];
	int name_len;
	char type[255];
	int type_len;
	int passive; /* @TODO: consider making these bit fields */
	int durable;
	int auto_delete; /* end @TODO */
	zval *arguments;
} amqp_exchange_object;

typedef struct _amqp_envelope_object {
	zend_object zo;
	char *body;
	size_t body_len;
	char routing_key[255];
	uint delivery_tag;
	int delivery_mode;
	char exchange_name[255];
	int is_redelivery;
	char content_type[255];
	char content_encoding[255];
	char type[255];
	long timestamp;
	int priority;
	char expiration[255];
	char user_id[255];
	char app_id[255];
	char message_id[255];
	char reply_to[255];
	char correlation_id[255];
	zval *headers;
} amqp_envelope_object;


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

#define PHP_AMQP_VERSION "1.2.0"

#endif	/* PHP_AMQP_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
