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
#include "amqp_basic_properties.h"
#include "amqp_connection_resource.h"
#include "amqp_channel.h"
#include "amqp_envelope.h"
#include "amqp_exchange.h"
#include "amqp_queue.h"
#include "amqp_timestamp.h"
#include "amqp_decimal.h"

#ifdef PHP_WIN32
# include "win32/unistd.h"
#else
# include <unistd.h>
#endif

/* True global resources - no need for thread safety here */

zend_class_entry *amqp_exception_class_entry,
				 *amqp_connection_exception_class_entry,
				 *amqp_channel_exception_class_entry,
				 *amqp_queue_exception_class_entry,
				 *amqp_exchange_exception_class_entry,
				 *amqp_envelope_exception_class_entry,
				 *amqp_value_exception_class_entry;

/* {{{ amqp_functions[]
*
*Every user visible function must have an entry in amqp_functions[].
*/
zend_function_entry amqp_functions[] = {
	{NULL, NULL, NULL}
};
/* }}} */

PHP_INI_BEGIN()
	PHP_INI_ENTRY("amqp.host",					DEFAULT_HOST,					PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.vhost",					DEFAULT_VHOST,					PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.port",					DEFAULT_PORT,					PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.timeout",				DEFAULT_TIMEOUT,				PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.read_timeout",			DEFAULT_READ_TIMEOUT,			PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.write_timeout",			DEFAULT_WRITE_TIMEOUT,			PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.connect_timeout",		DEFAULT_CONNECT_TIMEOUT,		PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.rpc_timeout",			DEFAULT_RPC_TIMEOUT,			PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.login",					DEFAULT_LOGIN,					PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.password",				DEFAULT_PASSWORD,				PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.auto_ack",				DEFAULT_AUTOACK,				PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.prefetch_count",		DEFAULT_PREFETCH_COUNT,			PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.prefetch_size",			DEFAULT_PREFETCH_SIZE,			PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.global_prefetch_count",	DEFAULT_GLOBAL_PREFETCH_COUNT,	PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.global_prefetch_size",	DEFAULT_GLOBAL_PREFETCH_SIZE,	PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.channel_max",			DEFAULT_CHANNEL_MAX,			PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.frame_max",				DEFAULT_FRAME_MAX,				PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.heartbeat",				DEFAULT_HEARTBEAT,				PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.cacert",				DEFAULT_CACERT,					PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.cert",					DEFAULT_CERT,					PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.key",					DEFAULT_KEY,					PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.verify",				DEFAULT_VERIFY,					PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("amqp.sasl_method",			DEFAULT_SASL_METHOD,			PHP_INI_ALL, NULL)
PHP_INI_END()

ZEND_DECLARE_MODULE_GLOBALS(amqp);

static PHP_GINIT_FUNCTION(amqp) /* {{{ */
{
	amqp_globals->error_message = NULL;
	amqp_globals->error_code = 0;
} /* }}} */

static PHP_MINIT_FUNCTION(amqp) /* {{{ */
{
	zend_class_entry ce;

	/* Set up the connection resource */
	le_amqp_connection_resource = zend_register_list_destructors_ex(amqp_connection_resource_dtor, NULL, PHP_AMQP_CONNECTION_RES_NAME, module_number);
	le_amqp_connection_resource_persistent = zend_register_list_destructors_ex(NULL, amqp_connection_resource_dtor_persistent, PHP_AMQP_CONNECTION_RES_NAME, module_number);

	PHP_MINIT(amqp_connection)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(amqp_channel)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(amqp_queue)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(amqp_exchange)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(amqp_basic_properties)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(amqp_envelope)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(amqp_timestamp)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(amqp_decimal)(INIT_FUNC_ARGS_PASSTHRU);

	/* Class Exceptions */
	INIT_CLASS_ENTRY(ce, "AMQPException", NULL);
	amqp_exception_class_entry = PHP5to7_zend_register_internal_class_ex(&ce, zend_exception_get_default(TSRMLS_C));

	INIT_CLASS_ENTRY(ce, "AMQPConnectionException", NULL);
	amqp_connection_exception_class_entry = PHP5to7_zend_register_internal_class_ex(&ce, amqp_exception_class_entry);

	INIT_CLASS_ENTRY(ce, "AMQPChannelException", NULL);
	amqp_channel_exception_class_entry = PHP5to7_zend_register_internal_class_ex(&ce, amqp_exception_class_entry);

	INIT_CLASS_ENTRY(ce, "AMQPQueueException", NULL);
	amqp_queue_exception_class_entry = PHP5to7_zend_register_internal_class_ex(&ce, amqp_exception_class_entry);

	INIT_CLASS_ENTRY(ce, "AMQPEnvelopeException", NULL);
	amqp_envelope_exception_class_entry = PHP5to7_zend_register_internal_class_ex(&ce, amqp_exception_class_entry);
	zend_declare_property_null(amqp_envelope_exception_class_entry, ZEND_STRL("envelope"), ZEND_ACC_PUBLIC TSRMLS_CC);

	INIT_CLASS_ENTRY(ce, "AMQPExchangeException", NULL);
	amqp_exchange_exception_class_entry = PHP5to7_zend_register_internal_class_ex(&ce, amqp_exception_class_entry);

	INIT_CLASS_ENTRY(ce, "AMQPValueException", NULL);
	amqp_value_exception_class_entry = PHP5to7_zend_register_internal_class_ex(&ce, amqp_exception_class_entry);

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

	REGISTER_LONG_CONSTANT("AMQP_SASL_METHOD_PLAIN",		AMQP_SASL_METHOD_PLAIN,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AMQP_SASL_METHOD_EXTERNAL",		AMQP_SASL_METHOD_EXTERNAL,	CONST_CS | CONST_PERSISTENT);
	return SUCCESS;
} /* }}} */

static PHP_MSHUTDOWN_FUNCTION(amqp) /* {{{ */
{
	UNREGISTER_INI_ENTRIES();

	return SUCCESS;
} /* }}} */

static PHP_RSHUTDOWN_FUNCTION(amqp) /* {{{ */
{
	if (NULL != PHP_AMQP_G(error_message)) {
		efree(PHP_AMQP_G(error_message));
		PHP_AMQP_G(error_message) = NULL;
	}

	PHP_AMQP_G(error_code) = 0;

	return SUCCESS;
} /* }}} */

static PHP_MINFO_FUNCTION(amqp) /* {{{ */
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
} /* }}} */

/* {{{ amqp_module_entry
*/
zend_module_entry amqp_module_entry = {
	STANDARD_MODULE_HEADER,
	"amqp",
	amqp_functions,
	PHP_MINIT(amqp),
	PHP_MSHUTDOWN(amqp),
	NULL,
	PHP_RSHUTDOWN(amqp),
	PHP_MINFO(amqp),
	PHP_AMQP_VERSION,
	PHP_MODULE_GLOBALS(amqp),
	PHP_GINIT(amqp),
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_AMQP
	ZEND_GET_MODULE(amqp)
#endif

int php_amqp_error(amqp_rpc_reply_t reply, char **message, amqp_connection_resource *connection_resource, amqp_channel_resource *channel_resource TSRMLS_DC)
{
	return php_amqp_error_advanced(reply, message, connection_resource, channel_resource, 1 TSRMLS_CC);
}

int php_amqp_error_advanced(amqp_rpc_reply_t reply, char **message, amqp_connection_resource *connection_resource, amqp_channel_resource *channel_resource, int fail_on_errors TSRMLS_DC)
{
	assert(connection_resource != NULL);

	PHP_AMQP_G(error_code) = 0;
	if (*message != NULL) {
		efree(*message);
	}

	int res = php_amqp_connection_resource_error(reply, message, connection_resource, (amqp_channel_t)(channel_resource ? channel_resource->channel_id : 0) TSRMLS_CC);

	switch (res) {
		case PHP_AMQP_RESOURCE_RESPONSE_OK:
			break;
		case PHP_AMQP_RESOURCE_RESPONSE_ERROR:
			if (!fail_on_errors) {
				break;
			}
			/* Library or other non-protocol or even protocol related errors may be here. */
			/* In most cases it designate some underlying hard errors. Fail fast. */
		case PHP_AMQP_RESOURCE_RESPONSE_ERROR_CONNECTION_CLOSED:
			/* Mark connection resource as closed to prevent sending any further requests */
			connection_resource->is_connected = '\0';

			/* Close connection with all its channels */
			php_amqp_disconnect_force(connection_resource TSRMLS_CC);

			break;
		case PHP_AMQP_RESOURCE_RESPONSE_ERROR_CHANNEL_CLOSED:
			/* Mark channel as closed to prevent sending channel.close request */
			assert(channel_resource != NULL);
			if (channel_resource) {
				channel_resource->is_connected = '\0';

				/* Close channel */
				php_amqp_close_channel(channel_resource, 1 TSRMLS_CC);
			}
			/* No more error handling necessary, returning. */
			break;
		default:
			spprintf(message, 0, "Unknown server error, method id 0x%08X (not handled by extension)", reply.reply.id);
			break;
	}

	return res;
}

void php_amqp_zend_throw_exception_short(amqp_rpc_reply_t reply, zend_class_entry *exception_ce TSRMLS_DC) {
	php_amqp_zend_throw_exception(reply, exception_ce, PHP_AMQP_G(error_message), PHP_AMQP_G(error_code) TSRMLS_CC);
}

void php_amqp_zend_throw_exception(amqp_rpc_reply_t reply, zend_class_entry *exception_ce, const char *message, PHP5to7_param_long_type_t code TSRMLS_DC)
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
	assert(channel_resource != NULL);
	assert(channel_resource->channel_id > 0);

	if (connection_resource) {
		amqp_maybe_release_buffers_on_channel(connection_resource->connection_state, channel_resource->channel_id);
	}
}

/*
*Local variables:
*tab-width: 4
*c-basic-offset: 4
*End:
*vim600: noet sw=4 ts=4 fdm=marker
*vim<600: noet sw=4 ts=4
*/
