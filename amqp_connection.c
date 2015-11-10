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
# include <signal.h>
# include <stdint.h>
#endif
#include <amqp.h>
#include <amqp_framing.h>
#include <amqp_tcp_socket.h>

#ifdef PHP_WIN32
# include "win32/unistd.h"
#else
# include <unistd.h>
#endif

#include "php_amqp.h"
#include "amqp_channel.h"
#include "amqp_connection_resource.h"
#include "amqp_connection.h"

#ifndef E_DEPRECATED
#define E_DEPRECATED E_WARNING
#endif

zend_class_entry *amqp_connection_class_entry;
#define this_ce amqp_connection_class_entry

zend_object_handlers amqp_connection_object_handlers;


static void php_amqp_cleanup_connection_resource(amqp_connection_resource *connection_resource TSRMLS_DC)
{
	if (!connection_resource) {
		return;
	}

	PHP5to7_zend_resource_t resource = connection_resource->resource;

	connection_resource->parent = NULL;

	if (connection_resource->is_dirty) {
		if (connection_resource->is_persistent) {

			PHP5to7_zend_resource_le_t *le = PHP5to7_ZEND_RESOURCE_LE_EMPTY;

			if (PHP5to7_ZEND_HASH_FIND(&EG(persistent_list), connection_resource->resource_key, connection_resource->resource_key_len + 1, le)) {

				if (PHP5to7_ZEND_RSRC_TYPE_P(Z_RES_P(le)) == le_amqp_connection_resource_persistent) {
					PHP5to7_ZEND_HASH_DEL(&EG(persistent_list), connection_resource->resource_key, (uint)(connection_resource->resource_key_len + 1));
				}
			}
		}

		zend_list_delete(resource);
	} else {
		if (connection_resource->is_persistent) {
			connection_resource->resource = PHP5to7_ZEND_RESOURCE_EMPTY;
		}

		if (connection_resource->resource != PHP5to7_ZEND_RESOURCE_EMPTY) {
			zend_list_delete(resource);
		}
	}
}

static void php_amqp_disconnect(amqp_connection_resource *resource TSRMLS_DC)
{
	php_amqp_prepare_for_disconnect(resource TSRMLS_CC);
	php_amqp_cleanup_connection_resource(resource TSRMLS_CC);
}


static void php_amqp_disconnect_force(amqp_connection_resource *resource TSRMLS_DC)
{
	php_amqp_prepare_for_disconnect(resource TSRMLS_CC);
	resource->is_dirty = '\1';
	php_amqp_cleanup_connection_resource(resource TSRMLS_CC);
}

/**
 * 	php_amqp_connect
 *	handles connecting to amqp
 *	called by connect(), pconnect(), reconnect(), preconnect()
 */
int php_amqp_connect(amqp_connection_object *connection, zend_bool persistent, INTERNAL_FUNCTION_PARAMETERS)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;

	char *key;	PHP5to7_param_str_len_type_t key_len;

	if (connection->connection_resource) {
		/* Clean up old memory allocations which are now invalid (new connection) */
		php_amqp_cleanup_connection_resource(connection->connection_resource TSRMLS_CC);

		connection->connection_resource = NULL;
	}

	assert(connection->connection_resource == NULL);

	amqp_connection_params connection_params;

	connection_params.host = PHP_AMQP_READ_THIS_PROP_STR("host");
	connection_params.port = (int)PHP_AMQP_READ_THIS_PROP_LONG("port");
	connection_params.vhost = PHP_AMQP_READ_THIS_PROP_STR("vhost");
	connection_params.login = PHP_AMQP_READ_THIS_PROP_STR("login");
	connection_params.password = PHP_AMQP_READ_THIS_PROP_STR("password");
	connection_params.frame_max = (int) PHP_AMQP_READ_THIS_PROP_LONG("frame_max");
	connection_params.channel_max = (int) PHP_AMQP_READ_THIS_PROP_LONG("channel_max");
	connection_params.heartbeat = (int) PHP_AMQP_READ_THIS_PROP_LONG("heartbeat");
	connection_params.read_timeout = (int) PHP_AMQP_READ_THIS_PROP_DOUBLE("read_timeout");
	connection_params.write_timeout = (int) PHP_AMQP_READ_THIS_PROP_DOUBLE("write_timeout");
	connection_params.connect_timeout = (int) PHP_AMQP_READ_THIS_PROP_DOUBLE("connect_timeout");


	if (persistent) {
		PHP5to7_zend_resource_store_t *le = PHP5to7_ZEND_RESOURCE_EMPTY;

		/* Look for an established resource */
		key_len = spprintf(&key, 0,
						   "amqp_conn_res_%s_%d_%s_%s_%s_%d_%d_%d",
						   connection_params.host,
						   connection_params.port,
						   connection_params.vhost,
						   connection_params.login,
						   connection_params.password,
						   connection_params.frame_max,
						   connection_params.channel_max,
						   connection_params.heartbeat
		);

		if (PHP5to7_ZEND_HASH_STR_FIND_PTR(&EG(persistent_list), key, key_len + 1, le)) {
			efree(key);

			if (le->type != le_amqp_connection_resource_persistent) {
				/* hash conflict, given name associate with non-amqp persistent connection resource */
				return 0;
			}

			/* An entry for this connection resource already exists */
			/* Stash the connection resource in the connection */
			connection->connection_resource = le->ptr;

			if (connection->connection_resource->resource != PHP5to7_ZEND_RESOURCE_EMPTY) {
				/*  resource in use! */
				connection->connection_resource = NULL;

				zend_throw_exception(amqp_connection_exception_class_entry, "There are already established persistent connection to the same resource.", 0 TSRMLS_CC);
				return 0;
			}

			connection->connection_resource->resource = PHP5to7_ZEND_REGISTER_RESOURCE(connection->connection_resource, persistent ? le_amqp_connection_resource_persistent : le_amqp_connection_resource);
			connection->connection_resource->parent = connection;

			/* Set desired timeouts */
			if (php_amqp_set_resource_read_timeout(connection->connection_resource, PHP_AMQP_READ_THIS_PROP_DOUBLE("read_timeout") TSRMLS_CC) == 0
			    || php_amqp_set_resource_write_timeout(connection->connection_resource, PHP_AMQP_READ_THIS_PROP_DOUBLE("write_timeout") TSRMLS_CC) == 0) {

				php_amqp_disconnect_force(connection->connection_resource TSRMLS_CC);
				connection->connection_resource = NULL;
			   return 0;
			}

			/* Set connection status to connected */
			connection->connection_resource->is_connected = '\1';
			connection->connection_resource->is_persistent = persistent;

			return 1;
		}

		efree(key);
	}

	connection->connection_resource = connection_resource_constructor(&connection_params, persistent TSRMLS_CC);

	if (connection->connection_resource == NULL) {
		return 0;
	}

	connection->connection_resource->resource = PHP5to7_ZEND_REGISTER_RESOURCE(connection->connection_resource, persistent ? le_amqp_connection_resource_persistent : le_amqp_connection_resource);
	connection->connection_resource->parent = connection;

	/* Set connection status to connected */
	connection->connection_resource->is_connected = '\1';

	if (persistent) {
		connection->connection_resource->is_persistent = persistent;

		key_len = spprintf(&key, 0,
						   "amqp_conn_res_%s_%d_%s_%s_%s_%d_%d_%d",
						   connection_params.host,
						   connection_params.port,
						   connection_params.vhost,
						   connection_params.login,
						   connection_params.password,
						   connection_params.frame_max,
						   connection_params.channel_max,
						   connection_params.heartbeat
		);

		connection->connection_resource->resource_key     = pestrndup(key, (uint) key_len, persistent);
		connection->connection_resource->resource_key_len = key_len;

		efree(key);

		PHP5to7_zend_resource_store_t new_le;

		/* Store a reference in the persistence list */
		new_le.ptr  = connection->connection_resource;
		new_le.type = persistent ? le_amqp_connection_resource_persistent : le_amqp_connection_resource;

		if (!PHP5to7_ZEND_HASH_STR_UPD_MEM(&EG(persistent_list), connection->connection_resource->resource_key, connection->connection_resource->resource_key_len + 1, new_le, sizeof(PHP5to7_zend_resource_store_t))) {
			php_amqp_disconnect_force(connection->connection_resource TSRMLS_CC);
			connection->connection_resource = NULL;
			return 0;
		}
	}

	return 1;
}

void amqp_connection_free(PHP5to7_obj_free_zend_object *object TSRMLS_DC)
{
	amqp_connection_object *connection = PHP_AMQP_FETCH_CONNECTION(object);

	if (connection->connection_resource) {
		php_amqp_disconnect(connection->connection_resource TSRMLS_CC);
		connection->connection_resource = NULL;
	}

	zend_object_std_dtor(&connection->zo TSRMLS_CC);

#if PHP_MAJOR_VERSION < 7
	efree(object);
#endif
}

PHP5to7_zend_object_value amqp_connection_ctor(zend_class_entry *ce TSRMLS_DC)
{
	amqp_connection_object* connection = PHP5to7_ECALLOC_CONNECTION_OBJECT(ce);

	zend_object_std_init(&connection->zo, ce TSRMLS_CC);
	AMQP_OBJECT_PROPERTIES_INIT(connection->zo, ce);

#if PHP_MAJOR_VERSION >=7
	connection->zo.handlers = &amqp_connection_object_handlers;

	return &connection->zo;
#else
	PHP5to7_zend_object_value new_value;

	new_value.handle = zend_objects_store_put(
			connection,
			NULL,
			(zend_objects_free_object_storage_t) amqp_connection_free,
			NULL TSRMLS_CC
	);

	new_value.handlers = zend_get_std_object_handlers();

	return new_value;
#endif
}


/* {{{ proto AMQPConnection::__construct([array optional])
 * The array can contain 'host', 'port', 'login', 'password', 'vhost', 'read_timeout', 'write_timeout', 'connect_timeout' and 'timeout' (deprecated) indexes
 */
PHP_METHOD(amqp_connection_class, __construct)
{
	zval* ini_arr = NULL;

	PHP5to7_zval_t *zdata = NULL;

	/* Parse out the method parameters */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|a", &ini_arr) == FAILURE) {
		return;
	}

	/* Pull the login out of the $params array */
	zdata = NULL;
	if (ini_arr && PHP5to7_ZEND_HASH_FIND(HASH_OF(ini_arr), "login", sizeof("login"), zdata)) {
		// TODO: check whether we need separate zval
		convert_to_string(PHP5to7_MAYBE_DEREF(zdata));
	}
	/* Validate the given login */
	if (zdata && Z_STRLEN_P(PHP5to7_MAYBE_DEREF(zdata)) > 0) {
		if (Z_STRLEN_P(PHP5to7_MAYBE_DEREF(zdata)) < 128) {
			zend_update_property(this_ce, getThis(), ZEND_STRL("login"), PHP5to7_MAYBE_DEREF(zdata)TSRMLS_CC);
		} else {
			zend_throw_exception(amqp_connection_exception_class_entry, "Parameter 'login' exceeds 128 character limit.", 0 TSRMLS_CC);
			return;
		}
	} else {
		zend_update_property_stringl(this_ce, getThis(), ZEND_STRL("login"), INI_STR("amqp.login"), (PHP5to7_param_str_len_type_t) (strlen(INI_STR("amqp.login")) > 128 ? 128 : strlen(INI_STR("amqp.login"))) TSRMLS_CC);
	}

	/* Pull the password out of the $params array */
	zdata = NULL;
	if (ini_arr && PHP5to7_ZEND_HASH_FIND(HASH_OF(ini_arr), "password", sizeof("password"), zdata)) {
		convert_to_string(PHP5to7_MAYBE_DEREF(zdata));
	}
	/* Validate the given password */
	if (zdata && Z_STRLEN_P(PHP5to7_MAYBE_DEREF(zdata)) > 0) {
		if (Z_STRLEN_P(PHP5to7_MAYBE_DEREF(zdata)) < 128) {
			zend_update_property_stringl(this_ce, getThis(), ZEND_STRL("password"), Z_STRVAL_P(PHP5to7_MAYBE_DEREF(zdata)), Z_STRLEN_P(PHP5to7_MAYBE_DEREF(zdata)) TSRMLS_CC);
		} else {
			zend_throw_exception(amqp_connection_exception_class_entry, "Parameter 'password' exceeds 128 character limit.", 0 TSRMLS_CC);
			return;
		}
	} else {
		zend_update_property_stringl(this_ce, getThis(), ZEND_STRL("password"), INI_STR("amqp.password"), (PHP5to7_param_str_len_type_t) (strlen(INI_STR("amqp.password")) > 128 ? 128 : strlen(INI_STR("amqp.login"))) TSRMLS_CC);
	}

	/* Pull the host out of the $params array */
	zdata = NULL;
	if (ini_arr && PHP5to7_ZEND_HASH_FIND(HASH_OF(ini_arr), "host", sizeof("host"), zdata)) {
		convert_to_string(PHP5to7_MAYBE_DEREF(zdata));
	}
	/* Validate the given host */
	if (zdata && Z_STRLEN_P(PHP5to7_MAYBE_DEREF(zdata)) > 0) {
		if (Z_STRLEN_P(PHP5to7_MAYBE_DEREF(zdata)) < 128) {
			zend_update_property_stringl(this_ce, getThis(), ZEND_STRL("host"), Z_STRVAL_P(PHP5to7_MAYBE_DEREF(zdata)), Z_STRLEN_P(PHP5to7_MAYBE_DEREF(zdata)) TSRMLS_CC);
		} else {
			zend_throw_exception(amqp_connection_exception_class_entry, "Parameter 'host' exceeds 128 character limit.", 0 TSRMLS_CC);
			return;
		}
	} else {
		zend_update_property_stringl(this_ce, getThis(), ZEND_STRL("host"), INI_STR("amqp.host"), (PHP5to7_param_str_len_type_t) (strlen(INI_STR("amqp.host")) > 128 ? 128 : strlen(INI_STR("amqp.host"))) TSRMLS_CC);
	}

	/* Pull the vhost out of the $params array */
	zdata = NULL;
	if (ini_arr && PHP5to7_ZEND_HASH_FIND(HASH_OF(ini_arr), "vhost", sizeof("vhost"), zdata)) {
		convert_to_string(PHP5to7_MAYBE_DEREF(zdata));
	}
	/* Validate the given vhost */
	if (zdata && Z_STRLEN_P(PHP5to7_MAYBE_DEREF(zdata)) > 0) {
		if (Z_STRLEN_P(PHP5to7_MAYBE_DEREF(zdata)) < 128) {
			zend_update_property_stringl(this_ce, getThis(), ZEND_STRL("vhost"), Z_STRVAL_P(PHP5to7_MAYBE_DEREF(zdata)), Z_STRLEN_P(PHP5to7_MAYBE_DEREF(zdata)) TSRMLS_CC);
		} else {
			zend_throw_exception(amqp_connection_exception_class_entry, "Parameter 'vhost' exceeds 128 character limit.", 0 TSRMLS_CC);
			return;
		}
	} else {
		zend_update_property_stringl(this_ce, getThis(), ZEND_STRL("vhost"), INI_STR("amqp.vhost"), (PHP5to7_param_str_len_type_t) (strlen(INI_STR("amqp.vhost")) > 128 ? 128 : strlen(INI_STR("amqp.vhost"))) TSRMLS_CC);

	}

	zend_update_property_long(this_ce, getThis(), ZEND_STRL("port"), INI_INT("amqp.port") TSRMLS_CC);

	if (ini_arr && PHP5to7_ZEND_HASH_FIND(HASH_OF(ini_arr), "port", sizeof("port"), zdata)) {
		convert_to_long(PHP5to7_MAYBE_DEREF(zdata));
		zend_update_property_long(this_ce, getThis(), ZEND_STRL("port"), Z_LVAL_P(PHP5to7_MAYBE_DEREF(zdata)) TSRMLS_CC);
	}

	zend_update_property_double(this_ce, getThis(), ZEND_STRL("read_timeout"), INI_FLT("amqp.read_timeout") TSRMLS_CC);

	if (ini_arr && PHP5to7_ZEND_HASH_FIND(HASH_OF(ini_arr), "read_timeout", sizeof("read_timeout"), zdata)) {
		convert_to_double(PHP5to7_MAYBE_DEREF(zdata));
		if (Z_DVAL_P(PHP5to7_MAYBE_DEREF(zdata)) < 0) {
			zend_throw_exception(amqp_connection_exception_class_entry, "Parameter 'read_timeout' must be greater than or equal to zero.", 0 TSRMLS_CC);
		} else {
			zend_update_property_double(this_ce, getThis(), ZEND_STRL("read_timeout"), Z_DVAL_P(PHP5to7_MAYBE_DEREF(zdata)) TSRMLS_CC);
		}

		if (ini_arr && PHP5to7_ZEND_HASH_FIND(HASH_OF(ini_arr), "timeout", sizeof("timeout"), zdata)) {
			/* 'read_timeout' takes precedence on 'timeout' but users have to know this */
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Parameter 'timeout' is deprecated, 'read_timeout' used instead");
		}

	} else if (ini_arr && PHP5to7_ZEND_HASH_FIND(HASH_OF(ini_arr), "timeout", sizeof("timeout"), zdata)) {

		php_error_docref(NULL TSRMLS_CC, E_DEPRECATED, "Parameter 'timeout' is deprecated; use 'read_timeout' instead");

		convert_to_double(PHP5to7_MAYBE_DEREF(zdata));
		if (Z_DVAL_P(PHP5to7_MAYBE_DEREF(zdata)) < 0) {
			zend_throw_exception(amqp_connection_exception_class_entry, "Parameter 'timeout' must be greater than or equal to zero.", 0 TSRMLS_CC);
		} else {
			zend_update_property_double(this_ce, getThis(), ZEND_STRL("read_timeout"), Z_DVAL_P(PHP5to7_MAYBE_DEREF(zdata)) TSRMLS_CC);
		}
	} else {

		assert(DEFAULT_TIMEOUT != NULL);
		if (strcmp(DEFAULT_TIMEOUT, INI_STR("amqp.timeout")) != 0) {
			php_error_docref(NULL TSRMLS_CC, E_DEPRECATED, "INI setting 'amqp.timeout' is deprecated; use 'amqp.read_timeout' instead");

			if (strcmp(DEFAULT_READ_TIMEOUT, INI_STR("amqp.read_timeout")) == 0) {
				zend_update_property_double(this_ce, getThis(), ZEND_STRL("read_timeout"), INI_FLT("amqp.timeout") TSRMLS_CC);
			} else {
				php_error_docref(NULL TSRMLS_CC, E_NOTICE, "INI setting 'amqp.read_timeout' will be used instead of 'amqp.timeout'");
				zend_update_property_double(this_ce, getThis(), ZEND_STRL("read_timeout"), INI_FLT("amqp.read_timeout") TSRMLS_CC);
			}
		} else {
			zend_update_property_double(this_ce, getThis(), ZEND_STRL("read_timeout"), INI_FLT("amqp.read_timeout") TSRMLS_CC);
		}
	}

	zend_update_property_double(this_ce, getThis(), ZEND_STRL("write_timeout"), INI_FLT("amqp.write_timeout") TSRMLS_CC);

	if (ini_arr && PHP5to7_ZEND_HASH_FIND(HASH_OF(ini_arr), "write_timeout", sizeof("write_timeout"), zdata)) {
		convert_to_double(PHP5to7_MAYBE_DEREF(zdata));
		if (Z_DVAL_P(PHP5to7_MAYBE_DEREF(zdata)) < 0) {
			zend_throw_exception(amqp_connection_exception_class_entry, "Parameter 'write_timeout' must be greater than or equal to zero.", 0 TSRMLS_CC);
		} else {
			zend_update_property_double(this_ce, getThis(), ZEND_STRL("write_timeout"), Z_DVAL_P(PHP5to7_MAYBE_DEREF(zdata)) TSRMLS_CC);
		}
	}

	zend_update_property_double(this_ce, getThis(), ZEND_STRL("connect_timeout"), INI_FLT("amqp.connect_timeout") TSRMLS_CC);

	if (ini_arr && PHP5to7_ZEND_HASH_FIND(HASH_OF(ini_arr), "connect_timeout", sizeof("connect_timeout"), zdata)) {
		convert_to_double(PHP5to7_MAYBE_DEREF(zdata));
		if (Z_DVAL_P(PHP5to7_MAYBE_DEREF(zdata)) < 0) {
			zend_throw_exception(amqp_connection_exception_class_entry, "Parameter 'connect_timeout' must be greater than or equal to zero.", 0 TSRMLS_CC);
		} else {
			zend_update_property_double(this_ce, getThis(), ZEND_STRL("connect_timeout"), Z_DVAL_P(PHP5to7_MAYBE_DEREF(zdata)) TSRMLS_CC);

		}
	}

	zend_update_property_long(this_ce, getThis(), ZEND_STRL("channel_max"), INI_INT("amqp.channel_max") TSRMLS_CC);

	if (ini_arr && PHP5to7_ZEND_HASH_FIND(HASH_OF(ini_arr), "channel_max", sizeof("channel_max"), zdata)) {
		convert_to_long(PHP5to7_MAYBE_DEREF(zdata));
		if (Z_LVAL_P(PHP5to7_MAYBE_DEREF(zdata)) < 0 || Z_LVAL_P(PHP5to7_MAYBE_DEREF(zdata)) > PHP_AMQP_MAX_CHANNELS) {
			zend_throw_exception(amqp_connection_exception_class_entry, "Parameter 'channel_max' is out of range.", 0 TSRMLS_CC);
		} else {
			if(Z_LVAL_P(PHP5to7_MAYBE_DEREF(zdata)) == 0) {
				zend_update_property_long(this_ce, getThis(), ZEND_STRL("channel_max"), PHP_AMQP_DEFAULT_CHANNEL_MAX TSRMLS_CC);
			} else {
				zend_update_property_long(this_ce, getThis(), ZEND_STRL("channel_max"), Z_LVAL_P(PHP5to7_MAYBE_DEREF(zdata)) TSRMLS_CC);
			}
		}
	}

	zend_update_property_long(this_ce, getThis(), ZEND_STRL("frame_max"), INI_INT("amqp.frame_max") TSRMLS_CC);

	if (ini_arr && PHP5to7_ZEND_HASH_FIND(HASH_OF(ini_arr), "frame_max", sizeof("frame_max"), zdata)) {
		convert_to_long(PHP5to7_MAYBE_DEREF(zdata));
		if (Z_LVAL_P(PHP5to7_MAYBE_DEREF(zdata)) < 0 || Z_LVAL_P(PHP5to7_MAYBE_DEREF(zdata)) > PHP_AMQP_MAX_FRAME) {
			zend_throw_exception(amqp_connection_exception_class_entry, "Parameter 'frame_max' is out of range.", 0 TSRMLS_CC);
		} else {
			if(Z_LVAL_P(PHP5to7_MAYBE_DEREF(zdata)) == 0) {
				zend_update_property_long(this_ce, getThis(), ZEND_STRL("frame_max"), PHP_AMQP_DEFAULT_FRAME_MAX TSRMLS_CC);
			} else {
				zend_update_property_long(this_ce, getThis(), ZEND_STRL("frame_max"), Z_LVAL_P(PHP5to7_MAYBE_DEREF(zdata)) TSRMLS_CC);
			}
		}
	}

	zend_update_property_long(this_ce, getThis(), ZEND_STRL("heartbeat"), INI_INT("amqp.heartbeat") TSRMLS_CC);

	if (ini_arr && PHP5to7_ZEND_HASH_FIND(HASH_OF(ini_arr), "heartbeat", sizeof("heartbeat"), zdata)) {
		convert_to_long(PHP5to7_MAYBE_DEREF(zdata));
		if (Z_LVAL_P(PHP5to7_MAYBE_DEREF(zdata)) < 0 || Z_LVAL_P(PHP5to7_MAYBE_DEREF(zdata)) > PHP_AMQP_MAX_HEARTBEAT) {
			zend_throw_exception(amqp_connection_exception_class_entry, "Parameter 'heartbeat' is out of range.", 0 TSRMLS_CC);
		} else {
			zend_update_property_long(this_ce, getThis(), ZEND_STRL("heartbeat"), Z_LVAL_P(PHP5to7_MAYBE_DEREF(zdata)) TSRMLS_CC);
		}
	}

}
/* }}} */


/* {{{ proto amqp::isConnected()
check amqp connection */
PHP_METHOD(amqp_connection_class, isConnected)
{
	amqp_connection_object *connection;

	PHP_AMQP_NOPARAMS();

	/* Get the connection object out of the store */
	connection = PHP_AMQP_GET_CONNECTION(getThis());

	/* If the channel_connect is 1, we have a connection */
	if (connection->connection_resource != NULL && connection->connection_resource->is_connected) {
		RETURN_TRUE;
	}

	/* We have no connection */
	RETURN_FALSE;
}
/* }}} */


/* {{{ proto amqp::connect()
create amqp connection */
PHP_METHOD(amqp_connection_class, connect)
{
	amqp_connection_object *connection;

	PHP_AMQP_NOPARAMS();

	/* Get the connection object out of the store */
	connection = PHP_AMQP_GET_CONNECTION(getThis());

	if (connection->connection_resource && connection->connection_resource->is_connected) {
		if (connection->connection_resource->is_persistent) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Attempt to start transient connection while persistent transient one already established. Continue.");
		}

		RETURN_TRUE;
	}

	/* Actually connect this resource to the broker */
	RETURN_BOOL(php_amqp_connect(connection, 0, INTERNAL_FUNCTION_PARAM_PASSTHRU));
}
/* }}} */


/* {{{ proto amqp::connect()
create amqp connection */
PHP_METHOD(amqp_connection_class, pconnect)
{
	amqp_connection_object *connection;

	PHP_AMQP_NOPARAMS();

	/* Get the connection object out of the store */
	connection = PHP_AMQP_GET_CONNECTION(getThis());

	if (connection->connection_resource && connection->connection_resource->is_connected) {

		assert(connection->connection_resource != NULL);
		if (!connection->connection_resource->is_persistent) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Attempt to start persistent connection while transient one already established. Continue.");
		}

		RETURN_TRUE;
	}

	/* Actually connect this resource to the broker or use stored connection */
	RETURN_BOOL(php_amqp_connect(connection, 1, INTERNAL_FUNCTION_PARAM_PASSTHRU));
}
/* }}} */


/* {{{ proto amqp:pdisconnect()
destroy amqp persistent connection */
PHP_METHOD(amqp_connection_class, pdisconnect)
{
	amqp_connection_object *connection;

	PHP_AMQP_NOPARAMS();

	/* Get the connection object out of the store */
	connection = PHP_AMQP_GET_CONNECTION(getThis());

	if (!connection->connection_resource || !connection->connection_resource->is_connected) {
		RETURN_TRUE;
	}

	assert(connection->connection_resource != NULL);

	if (!connection->connection_resource->is_persistent) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Attempt to close persistent connection while transient one already established. Abort.");

		RETURN_FALSE;
	}

	php_amqp_disconnect_force(connection->connection_resource TSRMLS_CC);
	connection->connection_resource = NULL;

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto amqp::disconnect()
destroy amqp connection */
PHP_METHOD(amqp_connection_class, disconnect)
{
	amqp_connection_object *connection;

	PHP_AMQP_NOPARAMS();

	/* Get the connection object out of the store */
	connection = PHP_AMQP_GET_CONNECTION(getThis());

	if (!connection->connection_resource || !connection->connection_resource->is_connected) {
		RETURN_TRUE;
	}

	if (connection->connection_resource->is_persistent) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Attempt to close transient connection while persistent one already established. Abort.");

		RETURN_FALSE;
	}

	assert(connection->connection_resource != NULL);

	php_amqp_disconnect(connection->connection_resource TSRMLS_CC);
	connection->connection_resource = NULL;

	RETURN_TRUE;
}

/* }}} */

/* {{{ proto amqp::reconnect()
recreate amqp connection */
PHP_METHOD(amqp_connection_class, reconnect)
{
	amqp_connection_object *connection;

	PHP_AMQP_NOPARAMS();

	/* Get the connection object out of the store */
	connection = PHP_AMQP_GET_CONNECTION(getThis());

	if (connection->connection_resource && connection->connection_resource->is_connected) {

		assert(connection->connection_resource != NULL);

		if (connection->connection_resource->is_persistent) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Attempt to reconnect persistent connection while transient one already established. Abort.");

			RETURN_FALSE;
		}

		php_amqp_disconnect(connection->connection_resource TSRMLS_CC);
		connection->connection_resource = NULL;
	}

	RETURN_BOOL(php_amqp_connect(connection, 0, INTERNAL_FUNCTION_PARAM_PASSTHRU));
}
/* }}} */

/* {{{ proto amqp::preconnect()
recreate amqp connection */
PHP_METHOD(amqp_connection_class, preconnect)
{
	amqp_connection_object *connection;

	PHP_AMQP_NOPARAMS();

	/* Get the connection object out of the store */
	connection = PHP_AMQP_GET_CONNECTION(getThis());


	if (connection->connection_resource && connection->connection_resource->is_connected) {

		assert(connection->connection_resource != NULL);

		if (!connection->connection_resource->is_persistent) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Attempt to reconnect transient connection while persistent one already established. Abort.");

			RETURN_FALSE;
		}

		php_amqp_disconnect_force(connection->connection_resource TSRMLS_CC);
		connection->connection_resource = NULL;
	}

	RETURN_BOOL(php_amqp_connect(connection, 1, INTERNAL_FUNCTION_PARAM_PASSTHRU));
}
/* }}} */


/* {{{ proto amqp::getLogin()
get the login */
PHP_METHOD(amqp_connection_class, getLogin)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;
	PHP_AMQP_NOPARAMS();
	PHP_AMQP_RETURN_THIS_PROP("login");
}
/* }}} */


/* {{{ proto amqp::setLogin(string login)
set the login */
PHP_METHOD(amqp_connection_class, setLogin)
{
	char *login = NULL;	PHP5to7_param_str_len_type_t login_len = 0;

	/* Get the login from the method params */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &login, &login_len) == FAILURE) {
		return;
	}

	/* Validate login length */
	if (login_len > 128) {
		zend_throw_exception(amqp_connection_exception_class_entry, "Invalid 'login' given, exceeds 128 characters limit.", 0 TSRMLS_CC);
		return;
	}

	zend_update_property_stringl(this_ce, getThis(), ZEND_STRL("login"), login, login_len TSRMLS_CC);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto amqp::getPassword()
get the password */
PHP_METHOD(amqp_connection_class, getPassword)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;
	PHP_AMQP_NOPARAMS();
	PHP_AMQP_RETURN_THIS_PROP("password");
}
/* }}} */


/* {{{ proto amqp::setPassword(string password)
set the password */
PHP_METHOD(amqp_connection_class, setPassword)
{
	char *password = NULL;	PHP5to7_param_str_len_type_t password_len = 0;

	/* Get the password from the method params */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &password, &password_len) == FAILURE) {
		return;
	}

	/* Validate password length */
	if (password_len > 128) {
		zend_throw_exception(amqp_connection_exception_class_entry, "Invalid 'password' given, exceeds 128 characters limit.", 0 TSRMLS_CC);
		return;
	}

	zend_update_property_stringl(this_ce, getThis(), ZEND_STRL("password"), password, password_len TSRMLS_CC);

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto amqp::getHost()
get the host */
PHP_METHOD(amqp_connection_class, getHost)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;
	PHP_AMQP_NOPARAMS();
	PHP_AMQP_RETURN_THIS_PROP("host");
}
/* }}} */


/* {{{ proto amqp::setHost(string host)
set the host */
PHP_METHOD(amqp_connection_class, setHost)
{
	char *host = NULL;	PHP5to7_param_str_len_type_t host_len = 0;

	/* Get the host from the method params */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &host, &host_len) == FAILURE) {
		return;
	}

	/* Validate host length */
	if (host_len > 1024) {
		zend_throw_exception(amqp_connection_exception_class_entry, "Invalid 'host' given, exceeds 1024 character limit.", 0 TSRMLS_CC);
		return;
	}

	zend_update_property_stringl(this_ce, getThis(), ZEND_STRL("host"), host, host_len TSRMLS_CC);

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto amqp::getPort()
get the port */
PHP_METHOD(amqp_connection_class, getPort)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;
	PHP_AMQP_NOPARAMS();
	PHP_AMQP_RETURN_THIS_PROP("port");
}
/* }}} */


/* {{{ proto amqp::setPort(mixed port)
set the port */
PHP_METHOD(amqp_connection_class, setPort)
{
	zval *zvalPort;
	int port;

	/* Get the port from the method params */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &zvalPort) == FAILURE) {
		return;
	}

	/* Parse out the port*/
	switch (Z_TYPE_P(zvalPort)) {
		case IS_DOUBLE:
			port = (int)Z_DVAL_P(zvalPort);
			break;
		case IS_LONG:
			port = (int)Z_LVAL_P(zvalPort);
			break;
		case IS_STRING:
			convert_to_long(zvalPort);
			port = (int)Z_LVAL_P(zvalPort);
			break;
		default:
			port = 0;
	}

	/* Check the port value */
	if (port <= 0 || port > 65535) {
		zend_throw_exception(amqp_connection_exception_class_entry, "Invalid port given. Value must be between 1 and 65535.", 0 TSRMLS_CC);
		return;
	}

	zend_update_property_long(this_ce, getThis(), ZEND_STRL("read_timeout"), port TSRMLS_CC);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto amqp::getVhost()
get the vhost */
PHP_METHOD(amqp_connection_class, getVhost)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;
	PHP_AMQP_NOPARAMS();
	PHP_AMQP_RETURN_THIS_PROP("vhost");
}
/* }}} */


/* {{{ proto amqp::setVhost(string vhost)
set the vhost */
PHP_METHOD(amqp_connection_class, setVhost)
{
	char *vhost = NULL;	PHP5to7_param_str_len_type_t vhost_len = 0;

	/* Get the vhost from the method params */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &vhost, &vhost_len) == FAILURE) {
		return;
	}

	/* Validate vhost length */
	if (vhost_len > 128) {
		zend_throw_exception(amqp_connection_exception_class_entry, "Parameter 'vhost' exceeds 128 characters limit.", 0 TSRMLS_CC);
		return;
	}

	zend_update_property_stringl(this_ce, getThis(), ZEND_STRL("vhost"), vhost, vhost_len TSRMLS_CC);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto amqp::getTimeout()
@deprecated
get the timeout */
PHP_METHOD(amqp_connection_class, getTimeout)
{
	php_error_docref(NULL TSRMLS_CC, E_DEPRECATED, "AMQPConnection::getTimeout() method is deprecated; use AMQPConnection::getReadTimeout() instead");

	PHP5to7_READ_PROP_RV_PARAM_DECL;
	PHP_AMQP_NOPARAMS();
	PHP_AMQP_RETURN_THIS_PROP("read_timeout");
}
/* }}} */

/* {{{ proto amqp::setTimeout(double timeout)
@deprecated
set the timeout */
PHP_METHOD(amqp_connection_class, setTimeout)
{
	amqp_connection_object *connection;
	double read_timeout;

	php_error_docref(NULL TSRMLS_CC, E_DEPRECATED, "AMQPConnection::setTimeout($timeout) method is deprecated; use AMQPConnection::setReadTimeout($timeout) instead");

	/* Get the timeout from the method params */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d", &read_timeout) == FAILURE) {
		return;
	}

	/* Validate timeout */
	if (read_timeout < 0) {
		zend_throw_exception(amqp_connection_exception_class_entry, "Parameter 'timeout' must be greater than or equal to zero.", 0 TSRMLS_CC);
		return;
	}

	/* Get the connection object out of the store */
	connection = PHP_AMQP_GET_CONNECTION(getThis());

	zend_update_property_double(this_ce, getThis(), ZEND_STRL("read_timeout"), read_timeout TSRMLS_CC);

	if (connection->connection_resource && connection->connection_resource->is_connected) {
		if (php_amqp_set_resource_read_timeout(connection->connection_resource, read_timeout TSRMLS_CC) == 0) {

			php_amqp_disconnect_force(connection->connection_resource TSRMLS_CC);
			connection->connection_resource = NULL;

			RETURN_FALSE;
		}
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto amqp::getReadTimeout()
get the read timeout */
PHP_METHOD(amqp_connection_class, getReadTimeout)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;
	PHP_AMQP_NOPARAMS();
	PHP_AMQP_RETURN_THIS_PROP("read_timeout");
}
/* }}} */

/* {{{ proto amqp::setReadTimeout(double timeout)
set read timeout */
PHP_METHOD(amqp_connection_class, setReadTimeout)
{
	amqp_connection_object *connection;
	double read_timeout;

	/* Get the timeout from the method params */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d", &read_timeout) == FAILURE) {
		return;
	}

	/* Validate timeout */
	if (read_timeout < 0) {
		zend_throw_exception(amqp_connection_exception_class_entry, "Parameter 'read_timeout' must be greater than or equal to zero.", 0 TSRMLS_CC);
		return;
	}

	/* Get the connection object out of the store */
	connection = PHP_AMQP_GET_CONNECTION(getThis());

	zend_update_property_double(this_ce, getThis(), ZEND_STRL("read_timeout"), read_timeout TSRMLS_CC);

	if (connection->connection_resource && connection->connection_resource->is_connected) {
		if (php_amqp_set_resource_read_timeout(connection->connection_resource, read_timeout TSRMLS_CC) == 0) {

			php_amqp_disconnect_force(connection->connection_resource TSRMLS_CC);
			connection->connection_resource = NULL;

			RETURN_FALSE;
		}
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto amqp::getWriteTimeout()
get write timeout */
PHP_METHOD(amqp_connection_class, getWriteTimeout)
{
	PHP5to7_READ_PROP_RV_PARAM_DECL;
	PHP_AMQP_NOPARAMS();
	PHP_AMQP_RETURN_THIS_PROP("write_timeout");
}
/* }}} */

/* {{{ proto amqp::setWriteTimeout(double timeout)
set write timeout */
PHP_METHOD(amqp_connection_class, setWriteTimeout)
{
	amqp_connection_object *connection;
	double write_timeout;

	/* Get the timeout from the method params */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d", &write_timeout) == FAILURE) {
		return;
	}

	/* Validate timeout */
	if (write_timeout < 0) {
		zend_throw_exception(amqp_connection_exception_class_entry, "Parameter 'write_timeout' must be greater than or equal to zero.", 0 TSRMLS_CC);
		return;
	}

	/* Get the connection object out of the store */
	connection = PHP_AMQP_GET_CONNECTION(getThis());

	zend_update_property_double(this_ce, getThis(), ZEND_STRL("write_timeout"), write_timeout TSRMLS_CC);

	if (connection->connection_resource && connection->connection_resource->is_connected) {
		if (php_amqp_set_resource_write_timeout(connection->connection_resource, write_timeout TSRMLS_CC) == 0) {

			php_amqp_disconnect_force(connection->connection_resource TSRMLS_CC);
			connection->connection_resource = NULL;

			RETURN_FALSE;
		}
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto amqp::getUsedChannels()
Get max used channels number */
PHP_METHOD(amqp_connection_class, getUsedChannels)
{
	amqp_connection_object *connection;

	/* Get the timeout from the method params */
	PHP_AMQP_NOPARAMS();

	/* Get the connection object out of the store */
	connection = PHP_AMQP_GET_CONNECTION(getThis());

	if (!connection->connection_resource || !connection->connection_resource->is_connected) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Connection is not connected.");

		RETURN_LONG(0);
	}

	RETURN_LONG(connection->connection_resource->used_slots);
}
/* }}} */

/* {{{ proto amqp::getMaxChannels()
Get max supported channels number per connection */
PHP_METHOD(amqp_connection_class, getMaxChannels)
{
	amqp_connection_object *connection;

	PHP_AMQP_NOPARAMS();

	/* Get the connection object out of the store */
	connection = PHP_AMQP_GET_CONNECTION(getThis());

	if (!connection->connection_resource || !connection->connection_resource->is_connected) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Connection is not connected.");

		RETURN_NULL();
	}

	RETURN_LONG(connection->connection_resource->max_slots);
}
/* }}} */

#if AMQP_VERSION_MAJOR * 100 + AMQP_VERSION_MINOR * 10 + AMQP_VERSION_PATCH > 52
/* {{{ proto amqp::getMaxFrameSize()
Get max supported frame size per connection in bytes */
PHP_METHOD(amqp_connection_class, getMaxFrameSize)
{
	amqp_connection_object *connection;

	PHP_AMQP_NOPARAMS();

	/* Get the connection object out of the store */
	connection = PHP_AMQP_GET_CONNECTION(getThis());

	if (!connection->connection_resource || !connection->connection_resource->is_connected) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Connection is not connected.");

		RETURN_NULL();
	}

	RETURN_LONG(amqp_get_frame_max(connection->connection_resource->connection_state));
}
/* }}} */

/* {{{ proto amqp::getHeartbeatInterval()
Get number of seconds between heartbeats of the connection in seconds */
PHP_METHOD(amqp_connection_class, getHeartbeatInterval)
{
	amqp_connection_object *connection;

	PHP_AMQP_NOPARAMS();

	/* Get the connection object out of the store */
	connection = PHP_AMQP_GET_CONNECTION(getThis());

	if (!connection->connection_resource || !connection->connection_resource->is_connected) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Connection is not connected.");

		RETURN_NULL();
	}

	RETURN_LONG(amqp_get_heartbeat(connection->connection_resource->connection_state));
}
/* }}} */
#endif

/* {{{ proto amqp::isPersistent()
check whether amqp connection is persistent */
PHP_METHOD(amqp_connection_class, isPersistent)
{
	amqp_connection_object *connection;

	PHP_AMQP_NOPARAMS();

	connection = PHP_AMQP_GET_CONNECTION(getThis());

	RETURN_BOOL(connection->connection_resource && connection->connection_resource->is_persistent);
}
/* }}} */


/* amqp_connection_class ARG_INFO definition */
ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class__construct, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
				ZEND_ARG_ARRAY_INFO(0, credentials, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_isConnected, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_connect, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_pconnect, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_pdisconnect, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_disconnect, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_reconnect, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_preconnect, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_getLogin, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_setLogin, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
				ZEND_ARG_INFO(0, login)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_getPassword, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_setPassword, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
				ZEND_ARG_INFO(0, password)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_getHost, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_setHost, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
				ZEND_ARG_INFO(0, host)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_getPort, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_setPort, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
				ZEND_ARG_INFO(0, port)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_getVhost, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_setVhost, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
				ZEND_ARG_INFO(0, vhost)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_getTimeout, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_setTimeout, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
				ZEND_ARG_INFO(0, timeout)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_getReadTimeout, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_setReadTimeout, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
				ZEND_ARG_INFO(0, timeout)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_getWriteTimeout, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_setWriteTimeout, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
				ZEND_ARG_INFO(0, timeout)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_getUsedChannels, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_getMaxChannels, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

#if AMQP_VERSION_MAJOR * 100 + AMQP_VERSION_MINOR * 10 + AMQP_VERSION_PATCH > 52
ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_getMaxFrameSize, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_getHeartbeatInterval, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class_isPersistent, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()


zend_function_entry amqp_connection_class_functions[] = {
		PHP_ME(amqp_connection_class, __construct, 	arginfo_amqp_connection_class__construct,	ZEND_ACC_PUBLIC)
		PHP_ME(amqp_connection_class, isConnected, 	arginfo_amqp_connection_class_isConnected,	ZEND_ACC_PUBLIC)
		PHP_ME(amqp_connection_class, connect, 		arginfo_amqp_connection_class_connect, 		ZEND_ACC_PUBLIC)
		PHP_ME(amqp_connection_class, pconnect, 	arginfo_amqp_connection_class_pconnect, 	ZEND_ACC_PUBLIC)
		PHP_ME(amqp_connection_class, pdisconnect, 	arginfo_amqp_connection_class_pdisconnect,	ZEND_ACC_PUBLIC)
		PHP_ME(amqp_connection_class, disconnect, 	arginfo_amqp_connection_class_disconnect,	ZEND_ACC_PUBLIC)
		PHP_ME(amqp_connection_class, reconnect, 	arginfo_amqp_connection_class_reconnect,	ZEND_ACC_PUBLIC)
		PHP_ME(amqp_connection_class, preconnect, 	arginfo_amqp_connection_class_preconnect,	ZEND_ACC_PUBLIC)

		PHP_ME(amqp_connection_class, getLogin, 	arginfo_amqp_connection_class_getLogin,		ZEND_ACC_PUBLIC)
		PHP_ME(amqp_connection_class, setLogin, 	arginfo_amqp_connection_class_setLogin,		ZEND_ACC_PUBLIC)

		PHP_ME(amqp_connection_class, getPassword, 	arginfo_amqp_connection_class_getPassword,	ZEND_ACC_PUBLIC)
		PHP_ME(amqp_connection_class, setPassword, 	arginfo_amqp_connection_class_setPassword,	ZEND_ACC_PUBLIC)

		PHP_ME(amqp_connection_class, getHost, 		arginfo_amqp_connection_class_getHost,		ZEND_ACC_PUBLIC)
		PHP_ME(amqp_connection_class, setHost, 		arginfo_amqp_connection_class_setHost,		ZEND_ACC_PUBLIC)

		PHP_ME(amqp_connection_class, getPort, 		arginfo_amqp_connection_class_getPort,		ZEND_ACC_PUBLIC)
		PHP_ME(amqp_connection_class, setPort, 		arginfo_amqp_connection_class_setPort,		ZEND_ACC_PUBLIC)

		PHP_ME(amqp_connection_class, getVhost, 	arginfo_amqp_connection_class_getVhost,		ZEND_ACC_PUBLIC)
		PHP_ME(amqp_connection_class, setVhost, 	arginfo_amqp_connection_class_setVhost,		ZEND_ACC_PUBLIC)

		PHP_ME(amqp_connection_class, getTimeout, 	arginfo_amqp_connection_class_getTimeout,	ZEND_ACC_PUBLIC)
		PHP_ME(amqp_connection_class, setTimeout, 	arginfo_amqp_connection_class_setTimeout,	ZEND_ACC_PUBLIC)

		PHP_ME(amqp_connection_class, getReadTimeout, 	arginfo_amqp_connection_class_getReadTimeout,	ZEND_ACC_PUBLIC)
		PHP_ME(amqp_connection_class, setReadTimeout, 	arginfo_amqp_connection_class_setReadTimeout,	ZEND_ACC_PUBLIC)

		PHP_ME(amqp_connection_class, getWriteTimeout, 	arginfo_amqp_connection_class_getWriteTimeout,	ZEND_ACC_PUBLIC)
		PHP_ME(amqp_connection_class, setWriteTimeout, 	arginfo_amqp_connection_class_setWriteTimeout,	ZEND_ACC_PUBLIC)

		PHP_ME(amqp_connection_class, getUsedChannels, arginfo_amqp_connection_class_getUsedChannels,	ZEND_ACC_PUBLIC)
		PHP_ME(amqp_connection_class, getMaxChannels,  arginfo_amqp_connection_class_getMaxChannels,	ZEND_ACC_PUBLIC)
		PHP_ME(amqp_connection_class, isPersistent, 	arginfo_amqp_connection_class_isPersistent,		ZEND_ACC_PUBLIC)
#if AMQP_VERSION_MAJOR * 100 + AMQP_VERSION_MINOR * 10 + AMQP_VERSION_PATCH > 52
		PHP_ME(amqp_connection_class, getHeartbeatInterval,  arginfo_amqp_connection_class_getHeartbeatInterval,	ZEND_ACC_PUBLIC)
		PHP_ME(amqp_connection_class, getMaxFrameSize,  arginfo_amqp_connection_class_getMaxFrameSize,	ZEND_ACC_PUBLIC)
#endif
		{NULL, NULL, NULL}	/* Must be the last line in amqp_functions[] */
};


PHP_MINIT_FUNCTION(amqp_connection)
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "AMQPConnection", amqp_connection_class_functions);
	ce.create_object = amqp_connection_ctor;
	this_ce = zend_register_internal_class(&ce TSRMLS_CC);

	zend_declare_property_null(this_ce, ZEND_STRL("login"), ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(this_ce, ZEND_STRL("password"), ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(this_ce, ZEND_STRL("host"), ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(this_ce, ZEND_STRL("vhost"), ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(this_ce, ZEND_STRL("port"), ZEND_ACC_PRIVATE TSRMLS_CC);

	zend_declare_property_null(this_ce, ZEND_STRL("read_timeout"), ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(this_ce, ZEND_STRL("write_timeout"), ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(this_ce, ZEND_STRL("connect_timeout"), ZEND_ACC_PRIVATE TSRMLS_CC);

	zend_declare_property_null(this_ce, ZEND_STRL("channel_max"), ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(this_ce, ZEND_STRL("frame_max"), ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(this_ce, ZEND_STRL("heartbeat"), ZEND_ACC_PRIVATE TSRMLS_CC);

#if PHP_MAJOR_VERSION >=7
	memcpy(&amqp_connection_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

	amqp_connection_object_handlers.offset = XtOffsetOf(amqp_connection_object, zo);
	amqp_connection_object_handlers.free_obj = amqp_connection_free;
#endif

	return SUCCESS;
}

/*
*Local variables:
*tab-width: 4
*c-basic-offset: 4
*End:
*vim600: noet sw=4 ts=4 fdm=marker
*vim<6
*/
