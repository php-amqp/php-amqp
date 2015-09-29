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

/* $Id: amqp.c 327551 2012-09-09 03:49:34Z pdezwart $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "zend_ini.h"
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

#include "php_amqp.h"
#include "amqp_connection.h"
#include "amqp_channel.h"
#include "amqp_queue.h"
#include "amqp_exchange.h"
#include "amqp_envelope.h"
#include "amqp_connection_resource.h"


#ifdef PHP_WIN32
# include "win32/unistd.h"
#else
# include <unistd.h>
#endif

#include "amqp_connection.h"
#include "amqp_connection_resource.h"
#include "amqp_channel.h"
#include "amqp_envelope.h"
#include "amqp_exchange.h"
#include "amqp_queue.h"

/* True global resources - no need for thread safety here */

zend_class_entry *amqp_exception_class_entry,
				 *amqp_connection_exception_class_entry,
				 *amqp_channel_exception_class_entry,
				 *amqp_queue_exception_class_entry,
				 *amqp_exchange_exception_class_entry;

/* {{{ amqp_functions[]
*
*Every user visible function must have an entry in amqp_functions[].
*/
zend_function_entry amqp_functions[] = {
	{NULL, NULL, NULL}	/* Must be the last line in amqp_functions[] */
};
/* }}} */

/* {{{ amqp_module_entry
*/
zend_module_entry amqp_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"amqp",
	amqp_functions,
	PHP_MINIT(amqp),
	PHP_MSHUTDOWN(amqp),
	NULL,
	NULL,
	PHP_MINFO(amqp),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_AMQP_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_AMQP
	ZEND_GET_MODULE(amqp)
#endif

void php_amqp_error(amqp_rpc_reply_t reply, char **message, amqp_connection_resource *connection_resource, amqp_channel_resource *channel_resource TSRMLS_DC)
{
	assert(connection_resource != NULL);

	switch (php_amqp_connection_resource_error(reply, message, connection_resource, (amqp_channel_t)(channel_resource ? channel_resource->channel_id : 0) TSRMLS_CC)) {
		case PHP_AMQP_RESOURCE_RESPONSE_OK:
			break;
		case PHP_AMQP_RESOURCE_RESPONSE_ERROR:
			/* Library or other non-protocol or even protocol related errors may be here, do nothing with this for now. */
			break;
		case PHP_AMQP_RESOURCE_RESPONSE_ERROR_CHANNEL_CLOSED:
			/* Mark channel as closed to prevent sending channel.close request */
			assert(channel_resource != NULL);
			if (channel_resource) {
				channel_resource->is_connected = '\0';

				/* Close channel */
				php_amqp_close_channel(channel_resource TSRMLS_CC);
			}
			/* No more error handling necessary, returning. */
			break;
		case PHP_AMQP_RESOURCE_RESPONSE_ERROR_CONNECTION_CLOSED:
			/* Mark connection as closed to prevent sending any further requests */
			connection_resource->is_connected = '\0';

			/* Close connection with all its channels */
			php_amqp_prepare_for_disconnect(connection_resource TSRMLS_CC);
			connection_resource->is_dirty = '\1';

			/* No more error handling necessary, returning. */
			break;
		default:
			spprintf(message, 0, "Unknown server error, method id 0x%08X (not handled by extension)", reply.reply.id);
			break;
	}
}

void php_amqp_zend_throw_exception(amqp_rpc_reply_t reply, zend_class_entry *exception_ce, const char *message, long code TSRMLS_DC)
{
	switch (reply.reply_type) {
		case AMQP_RESPONSE_NORMAL:
			break;
		case AMQP_RESPONSE_NONE:
			exception_ce = amqp_exception_class_entry;
			break;
		case AMQP_RESPONSE_LIBRARY_EXCEPTION:
			exception_ce = amqp_exception_class_entry;
			break;
		case AMQP_RESPONSE_SERVER_EXCEPTION:
			switch (reply.reply.id) {
				case AMQP_CONNECTION_CLOSE_METHOD:
					/* Fatal errors - pass them to connection level */
					exception_ce = amqp_connection_exception_class_entry;
					break;
				case AMQP_CHANNEL_CLOSE_METHOD:
					/* Most channel-level errors occurs due to previously known action and thus their kind can be predicted. */
					/* exception_ce = amqp_channel_exception_class_entry; */
					break;
			}
			break;
		/* Default for the above switch should be handled by the below default. */
		default:
			exception_ce = amqp_exception_class_entry;
			break;
	}

	zend_throw_exception(exception_ce, message, code TSRMLS_CC);
}


void php_amqp_maybe_release_buffers_on_channel(amqp_connection_resource *connection_resource, amqp_channel_resource *channel_resource)
{
	assert(connection_resource != NULL);
	assert(channel_resource != NULL);
	assert(channel_resource->channel_id > 0);

	if (connection_resource) {
		amqp_maybe_release_buffers_on_channel(connection_resource->connection_state, channel_resource->channel_id);
	}
}

amqp_bytes_t php_amqp_long_string(char const *cstr, int len)
{
	if (len < 1) {
		return amqp_empty_bytes;
	}

	amqp_bytes_t result;
	result.len   = (size_t)len;
	result.bytes = (void *) cstr;
	return result;
}

char *stringify_bytes(amqp_bytes_t bytes)
{
/* We will need up to 4 chars per byte, plus the terminating 0 */
	char *res = emalloc(bytes.len * 4 + 1);
	uint8_t *data = bytes.bytes;
	char *p = res;
	size_t i;

	for (i = 0; i < bytes.len; i++) {
		if (data[i] >= 32 && data[i] != 127) {
			*p++ = data[i];
		} else {
			*p++ = '\\';
			*p++ = '0' + (data[i] >> 6);
			*p++ = '0' + (data[i] >> 3 & 0x7);
			*p++ = '0' + (data[i] & 0x7);
		}
	}

	*p = 0;
	return res;
}

void internal_convert_zval_to_amqp_table(zval *zvalArguments, amqp_table_t *arguments, char allow_int_keys TSRMLS_DC)
{
	HashTable *argumentHash;
	HashPosition pos;
	zval **data;
	char type[16];
	amqp_table_t *inner_table;

	argumentHash = Z_ARRVAL_P(zvalArguments);

	/* Allocate all the memory necessary for storing the arguments */
	arguments->entries = (amqp_table_entry_t *)ecalloc(zend_hash_num_elements(argumentHash), sizeof(amqp_table_entry_t));
	arguments->num_entries = 0;

	for (zend_hash_internal_pointer_reset_ex(argumentHash, &pos);
		zend_hash_get_current_data_ex(argumentHash, (void**) &data, &pos) == SUCCESS;
		zend_hash_move_forward_ex(argumentHash, &pos)) {

		zval value;
		char *key;
		uint key_len;
		ulong index;
		char *strKey;
		char *strValue;
		amqp_table_entry_t *table;
		amqp_field_value_t *field;


		/* Make a copy of the value: */
		value = **data;
		zval_copy_ctor(&value);

		/* Now pull the key */

		if (zend_hash_get_current_key_ex(argumentHash, &key, &key_len, &index, 0, &pos) != HASH_KEY_IS_STRING) {

			if (allow_int_keys) {
				/* Convert to strings non-string keys */
				char str[32];

				key_len = sprintf(str, "%lu", index);
				key     = str;
			} else {
				/* Skip things that are not strings */
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Ignoring non-string header field '%lu'", index);

				continue;
			}

		}

		/* Build the value */
		table = &arguments->entries[arguments->num_entries++];
		field = &table->value;

		switch (Z_TYPE_P(&value)) {
			case IS_BOOL:
				field->kind          = AMQP_FIELD_KIND_BOOLEAN;
				field->value.boolean = (amqp_boolean_t)Z_LVAL_P(&value);
				break;
			case IS_DOUBLE:
				field->kind      = AMQP_FIELD_KIND_F64;
				field->value.f64 = Z_DVAL_P(&value);
				break;
			case IS_LONG:
				field->kind      = AMQP_FIELD_KIND_I64;
				field->value.i64 = Z_LVAL_P(&value);
				break;
			case IS_STRING:
				field->kind        = AMQP_FIELD_KIND_UTF8;

				if (Z_STRLEN_P(&value)) {
					amqp_bytes_t bytes;
					bytes.len = (size_t) Z_STRLEN_P(&value);
					bytes.bytes = estrndup(Z_STRVAL_P(&value), (uint)Z_STRLEN_P(&value));

					field->value.bytes = bytes;
				} else {
					field->value.bytes = amqp_empty_bytes;
				}

				break;
			case IS_ARRAY:
				field->kind = AMQP_FIELD_KIND_TABLE;
				internal_convert_zval_to_amqp_table(&value, &field->value.table, 1 TSRMLS_CC);

				break;
			default:
				switch(Z_TYPE_P(&value)) {
					case IS_NULL:     strcpy(type, "null"); break;
					case IS_OBJECT:   strcpy(type, "object"); break;
					case IS_RESOURCE: strcpy(type, "resource"); break;
					default:          strcpy(type, "unknown");
				}

				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Ignoring field '%s' due to unsupported value type (%s)", key, type);

				/* Reset entries counter back */
				arguments->num_entries --;
				continue;
		}

		strKey     = estrndup(key, key_len);
		table->key = amqp_cstring_bytes(strKey);

		/* Clean up the zval */
		zval_dtor(&value);
	}
}

inline amqp_table_t *convert_zval_to_amqp_table(zval *zvalArguments TSRMLS_DC)
{
	amqp_table_t *arguments;
	/* In setArguments, we are overwriting all the existing values */
	arguments = (amqp_table_t *)emalloc(sizeof(amqp_table_t));

	internal_convert_zval_to_amqp_table(zvalArguments, arguments, 0 TSRMLS_CC);

	return arguments;
}




void internal_php_amqp_free_amqp_table(amqp_table_t *object, char clear_root)
{
	if (!object) {
		return;
	}

	if (object->entries) {
		int macroEntryCounter;
		for (macroEntryCounter = 0; macroEntryCounter < object->num_entries; macroEntryCounter++) {

			amqp_table_entry_t *entry = &object->entries[macroEntryCounter];
			efree(entry->key.bytes);

			switch (entry->value.kind) {
				case AMQP_FIELD_KIND_TABLE:
					internal_php_amqp_free_amqp_table(&entry->value.value.table, 0);
					break;
				case AMQP_FIELD_KIND_UTF8:
					if (entry->value.value.bytes.bytes) {
						efree(entry->value.value.bytes.bytes);
					}
					break;
				default:
					break;
			}
		}
		efree(object->entries);
	}

	if (clear_root) {
		efree(object);
	}
}

void php_amqp_free_amqp_table(amqp_table_t *object)
{
	internal_php_amqp_free_amqp_table(object, 1);
}


PHP_INI_BEGIN()
	PHP_INI_ENTRY("amqp.host",				DEFAULT_HOST,				PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.vhost",				DEFAULT_VHOST,				PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.port",				DEFAULT_PORT,				PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.timeout",			DEFAULT_TIMEOUT,			PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.read_timeout",		DEFAULT_READ_TIMEOUT,		PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.write_timeout",		DEFAULT_WRITE_TIMEOUT,		PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.connect_timeout",	DEFAULT_CONNECT_TIMEOUT,		PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.login",				DEFAULT_LOGIN,				PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.password",			DEFAULT_PASSWORD,			PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.auto_ack",			DEFAULT_AUTOACK,			PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.prefetch_count",	DEFAULT_PREFETCH_COUNT,		PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.channel_max",		DEFAULT_CHANNEL_MAX,		PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.frame_max",			DEFAULT_FRAME_MAX,			PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.heartbeat",			DEFAULT_HEARTBEAT,			PHP_INI_ALL, NULL)
PHP_INI_END()

/* {{{ PHP_MINIT_FUNCTION
*/
PHP_MINIT_FUNCTION(amqp)
{
	zend_class_entry ce;

	/* Set up the connection resource */
	le_amqp_connection_resource = zend_register_list_destructors_ex(amqp_connection_resource_dtor, NULL, PHP_AMQP_CONNECTION_RES_NAME, module_number);
	le_amqp_connection_resource_persistent = zend_register_list_destructors_ex(NULL, amqp_connection_resource_dtor_persistent, PHP_AMQP_CONNECTION_RES_NAME, module_number);

	PHP_MINIT(amqp_connection)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(amqp_channel)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(amqp_queue)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(amqp_exchange)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(amqp_envelope)(INIT_FUNC_ARGS_PASSTHRU);

	/* Class Exceptions */
	INIT_CLASS_ENTRY(ce, "AMQPException", NULL);
	amqp_exception_class_entry = zend_register_internal_class_ex(&ce, zend_exception_get_default(TSRMLS_C), NULL TSRMLS_CC);

	INIT_CLASS_ENTRY(ce, "AMQPConnectionException", NULL);
	amqp_connection_exception_class_entry = zend_register_internal_class_ex(&ce, amqp_exception_class_entry, NULL TSRMLS_CC);

	INIT_CLASS_ENTRY(ce, "AMQPChannelException", NULL);
	amqp_channel_exception_class_entry = zend_register_internal_class_ex(&ce, amqp_exception_class_entry, NULL TSRMLS_CC);

	INIT_CLASS_ENTRY(ce, "AMQPQueueException", NULL);
	amqp_queue_exception_class_entry = zend_register_internal_class_ex(&ce, amqp_exception_class_entry, NULL TSRMLS_CC);

	INIT_CLASS_ENTRY(ce, "AMQPExchangeException", NULL);
	amqp_exchange_exception_class_entry = zend_register_internal_class_ex(&ce, amqp_exception_class_entry, NULL TSRMLS_CC);

	REGISTER_INI_ENTRIES();

	REGISTER_LONG_CONSTANT("AMQP_NOPARAM",			AMQP_NOPARAM,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AMQP_JUST_CONSUME",		AMQP_JUST_CONSUME,	CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AMQP_DURABLE",			AMQP_DURABLE,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AMQP_PASSIVE",			AMQP_PASSIVE,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AMQP_EXCLUSIVE",		AMQP_EXCLUSIVE,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AMQP_AUTODELETE",		AMQP_AUTODELETE,	CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AMQP_INTERNAL",			AMQP_INTERNAL,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AMQP_NOLOCAL",			AMQP_NOLOCAL,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AMQP_AUTOACK",			AMQP_AUTOACK,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AMQP_IFEMPTY",			AMQP_IFEMPTY,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AMQP_IFUNUSED",			AMQP_IFUNUSED,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AMQP_MANDATORY",		AMQP_MANDATORY,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AMQP_IMMEDIATE",		AMQP_IMMEDIATE,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AMQP_MULTIPLE",			AMQP_MULTIPLE,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AMQP_NOWAIT",			AMQP_NOWAIT,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AMQP_REQUEUE",			AMQP_REQUEUE,		CONST_CS | CONST_PERSISTENT);

	REGISTER_STRING_CONSTANT("AMQP_EX_TYPE_DIRECT",	AMQP_EX_TYPE_DIRECT,	CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("AMQP_EX_TYPE_FANOUT",	AMQP_EX_TYPE_FANOUT,	CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("AMQP_EX_TYPE_TOPIC",	AMQP_EX_TYPE_TOPIC,		CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("AMQP_EX_TYPE_HEADERS",AMQP_EX_TYPE_HEADERS,	CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("AMQP_OS_SOCKET_TIMEOUT_ERRNO",	AMQP_OS_SOCKET_TIMEOUT_ERRNO,	CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PHP_AMQP_MAX_CHANNELS",			PHP_AMQP_MAX_CHANNELS,			CONST_CS | CONST_PERSISTENT);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
*/
PHP_MSHUTDOWN_FUNCTION(amqp)
{
	UNREGISTER_INI_ENTRIES();

	return SUCCESS;
}
/* }}} */


/* {{{ PHP_MINFO_FUNCTION
*/
PHP_MINFO_FUNCTION(amqp)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "Version",					PHP_AMQP_VERSION);
	php_info_print_table_header(2, "Revision",					PHP_AMQP_REVISION);
	php_info_print_table_header(2, "Compiled",					__DATE__ " @ "  __TIME__);
	php_info_print_table_header(2, "AMQP protocol version", 	"0-9-1");
	php_info_print_table_header(2, "librabbitmq version", amqp_version());
	php_info_print_table_header(2, "Default max channels per connection",	DEFAULT_CHANNEL_MAX);
	php_info_print_table_header(2, "Default max frame size",	DEFAULT_FRAME_MAX);
	php_info_print_table_header(2, "Default heartbeats interval",	DEFAULT_HEARTBEAT);
	DISPLAY_INI_ENTRIES();
}
/* }}} */

/*
*Local variables:
*tab-width: 4
*c-basic-offset: 4
*End:
*vim600: noet sw=4 ts=4 fdm=marker
*vim<600: noet sw=4 ts=4
*/
