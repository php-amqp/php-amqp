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

/* $Id: amqp_connection.c 327551 2012-09-09 03:49:34Z pdezwart $ */

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
#include "amqp_connection_resource.h"

#ifndef E_DEPRECATED
#define E_DEPRECATED E_WARNING
#endif

#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3
zend_object_handlers amqp_connection_object_handlers;
HashTable *amqp_connection_object_get_debug_info(zval *object, int *is_temp TSRMLS_DC) {
	zval *value;
	HashTable *debug_info;
	amqp_connection_object *connection;

	/* Let zend clean up for us: */
	*is_temp = 1;

	/* Get the envelope object from which to read */
	connection = (amqp_connection_object *)zend_object_store_get_object(object TSRMLS_CC);

	/* Keep the first number matching the number of entries in this table*/
	ALLOC_HASHTABLE(debug_info);
	ZEND_INIT_SYMTABLE_EX(debug_info, 13 + 1, 0);

	/* Start adding values */
	MAKE_STD_ZVAL(value);
	ZVAL_STRINGL(value, connection->login, strlen(connection->login), 1);
	zend_hash_add(debug_info, "login", sizeof("login"), &value, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(value);
	ZVAL_STRINGL(value, connection->password, strlen(connection->password), 1);
	zend_hash_add(debug_info, "password", sizeof("password"), &value, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(value);
	ZVAL_STRINGL(value, connection->host, strlen(connection->host), 1);
	zend_hash_add(debug_info, "host", sizeof("host"), &value, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(value);
	ZVAL_STRINGL(value, connection->vhost, strlen(connection->vhost), 1);
	zend_hash_add(debug_info, "vhost", sizeof("vhost"), &value, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(value);
	ZVAL_LONG(value, connection->port);
	zend_hash_add(debug_info, "port", sizeof("port"), &value, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(value);
	ZVAL_DOUBLE(value, connection->read_timeout);
	zend_hash_add(debug_info, "read_timeout", sizeof("read_timeout"), &value, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(value);
	ZVAL_DOUBLE(value, connection->write_timeout);
	zend_hash_add(debug_info, "write_timeout", sizeof("write_timeout"), &value, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(value);
	ZVAL_DOUBLE(value, connection->connect_timeout);
	zend_hash_add(debug_info, "connect_timeout", sizeof("connect_timeout"), &value, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(value);
	ZVAL_BOOL(value, connection->is_connected);
	zend_hash_add(debug_info, "is_connected", sizeof("is_connected"), &value, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(value);
	ZVAL_BOOL(value, connection->is_persistent);
	zend_hash_add(debug_info, "is_persistent", sizeof("is_persistent"), &value, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(value);

	if (connection && connection->connection_resource) {
		ZVAL_RESOURCE(value, connection->connection_resource->resource_id);
        zend_list_addref(connection->connection_resource->resource_id);
	} else {
		ZVAL_NULL(value);
	}
	zend_hash_add(debug_info, "connection_resource", sizeof("connection_resource"), &value, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(value);
	if (connection->connection_resource) {
		ZVAL_LONG(value, connection->connection_resource->used_slots);
	} else {
		ZVAL_NULL(value);
	}
	zend_hash_add(debug_info, "used_channels", sizeof("used_channels"), &value, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(value);
	if (connection->connection_resource) {
		ZVAL_LONG(value, amqp_get_channel_max(connection->connection_resource->connection_state));
	} else {
		ZVAL_NULL(value);
	}
	zend_hash_add(debug_info, "max_channel_id", sizeof("max_channel_id"), &value, sizeof(zval *), NULL);

	/* Start adding values */
	return debug_info;
}
#endif


static void php_amqp_prepare_for_disconnect(amqp_connection_object *connection TSRMLS_DC)
{
	amqp_channel_object *channel;

	/* Pull the connection resource out for easy access */
	amqp_connection_resource *resource = connection->connection_resource;

	if (!resource) {
		return;
	}

	resource->resource_id = 0;

	assert(resource->slots != NULL);

	/* NOTE: when we have persistent connection we do not move channels between php requests
	 *       due to current php-amqp extension limitation in AMQPChannel where __construct == channel.open AMQP method call
	 *       and __destruct = channel.close AMQP method call
	 */

	/* Clean up old memory allocations which are now invalid (new connection) */
	amqp_channel_t slot;

	for (slot = 1; slot < PHP_AMQP_MAX_CHANNELS + 1; slot++) {
		if (resource->slots[slot] != 0) {
			php_amqp_close_channel(resource->slots[slot] TSRMLS_CC);
		}
	}

	/* If it's persistent connection do not destroy connection resource (this keep connection alive) */
	if (connection->is_persistent) {
		/* Cleanup buffers to reduce memory usage in idle mode */
		amqp_maybe_release_buffers(resource->connection_state);
	}

	return;
}

void php_amqp_disconnect_safe(amqp_connection_object *connection TSRMLS_DC)
{
	php_amqp_prepare_for_disconnect(connection TSRMLS_CC);

	if(!connection->is_persistent && connection->connection_resource && connection->connection_resource->resource_id > 0) {
		zend_list_delete(connection->connection_resource->resource_id);
	}

	/* Mark connection as closed */
	connection->connection_resource = NULL;
	connection->is_connected = '\0';
	connection->is_persistent = '\0';
}

void php_amqp_disconnect_force(amqp_connection_object *connection TSRMLS_DC)
{
	php_amqp_prepare_for_disconnect(connection TSRMLS_CC);

	if(connection->connection_resource) {

		if (connection->connection_resource->resource_id > 0) {
			zend_list_delete(connection->connection_resource->resource_id);
		}

		if (connection->is_persistent) {
			zend_rsrc_list_entry *le;

			if (zend_hash_find(&EG(persistent_list), connection->connection_resource->resource_key, connection->connection_resource->resource_key_len + 1, (void **)&le) == SUCCESS) {
				if (Z_TYPE_P(le) == le_amqp_connection_resource_persistent) {
					zend_hash_del(&EG(persistent_list), connection->connection_resource->resource_key, connection->connection_resource->resource_key_len + 1);
				}
			}
		}
	}

	/* Mark connection as closed */
	connection->connection_resource = NULL;
	connection->is_connected = '\0';
	connection->is_persistent = '\0';
}

/**
 * 	php_amqp_connect
 *	handles connecting to amqp
 *	called by connect(), pconnect(), reconnect(), preconnect()
 */
int php_amqp_connect(amqp_connection_object *connection, int persistent TSRMLS_DC)
{
	char *key;
	int key_len;

	/* Clean up old memory allocations which are now invalid (new connection) */
	assert(connection->connection_resource == NULL);
	assert(!connection->is_connected);

	if (persistent) {
		zend_rsrc_list_entry *le;
		/* Look for an established resource */
    	key_len = spprintf(&key, 0, "amqp_conn_res_%s_%d_%s_%s_%s", connection->host, connection->port, connection->vhost, connection->login, connection->password);

		if (zend_hash_find(&EG(persistent_list), key, key_len + 1, (void **)&le) == SUCCESS) {
			efree(key);

			if (Z_TYPE_P(le) != le_amqp_connection_resource_persistent) {
				/* hash conflict, given name associate with non-amqp persistent connection resource */
				return 0;
			}

			/* An entry for this connection resource already exists */
			/* Stash the connection resource in the connection */
			connection->connection_resource = le->ptr;

			if (connection->connection_resource->resource_id > 0) {
				/*  resource in use! */
				connection->connection_resource = NULL;

				zend_throw_exception(amqp_connection_exception_class_entry, "There are already established persistent connection to the same resource.", 0 TSRMLS_CC);
				return 0;
			}

			connection->connection_resource->resource_id = ZEND_REGISTER_RESOURCE(NULL, connection->connection_resource, persistent ? le_amqp_connection_resource_persistent : le_amqp_connection_resource);

			/* Set desired timeouts */
			if (php_amqp_set_resource_read_timeout(connection->connection_resource, connection->read_timeout TSRMLS_CC) == 0
			    || php_amqp_set_resource_write_timeout(connection->connection_resource, connection->write_timeout TSRMLS_CC) == 0) {

			   php_amqp_disconnect_force(connection TSRMLS_CC);

			   connection->connection_resource = NULL;
			   return 0;
			}


			/* Set connection status to connected */
			connection->is_connected = '\1';
			connection->is_persistent = persistent;

			return 1;
		}

		efree(key);
	}

	connection->connection_resource = connection_resource_constructor(connection, persistent TSRMLS_CC);

	if (!connection->connection_resource) {
		return 0;
	}

	connection->connection_resource->resource_id = ZEND_REGISTER_RESOURCE(NULL, connection->connection_resource, persistent ? le_amqp_connection_resource_persistent : le_amqp_connection_resource);

	/* Set connection status to connected */
	connection->is_connected = '\1';

	if (persistent) {
		zend_rsrc_list_entry new_le;

		connection->is_persistent = persistent;

		key_len = spprintf(&key, 0, "amqp_conn_res_%s_%d_%s_%s_%s", connection->host, connection->port, connection->vhost, connection->login, connection->password);

		connection->connection_resource->resource_key     = pestrndup(key, key_len, persistent);
		connection->connection_resource->resource_key_len = key_len;

		efree(key);

		/* Store a reference in the persistence list */
		new_le.ptr  = connection->connection_resource;
		new_le.type = persistent ? le_amqp_connection_resource_persistent : le_amqp_connection_resource;

		if (FAILURE == zend_hash_add(&EG(persistent_list), connection->connection_resource->resource_key, connection->connection_resource->resource_key_len + 1, &new_le, sizeof(zend_rsrc_list_entry), NULL)) {
			php_amqp_disconnect_force(connection TSRMLS_CC);
			return 0;
		}
	}

	return 1;
}

void amqp_connection_dtor(void *object TSRMLS_DC)
{
	amqp_connection_object *connection = (amqp_connection_object*)object;

	php_amqp_disconnect_safe(connection TSRMLS_CC);

	assert(connection->connection_resource == NULL);

	/* Clean up all the strings */
	if (connection->host) {
		efree(connection->host);
	}

	if (connection->vhost) {
		efree(connection->vhost);
	}

	if (connection->login) {
		efree(connection->login);
	}

	if (connection->password) {
		efree(connection->password);
	}

	zend_object_std_dtor(&connection->zo TSRMLS_CC);

	efree(object);
}

zend_object_value amqp_connection_ctor(zend_class_entry *ce TSRMLS_DC)
{
	zend_object_value new_value;

	amqp_connection_object* connection = (amqp_connection_object*)emalloc(sizeof(amqp_connection_object));
	memset(connection, 0, sizeof(amqp_connection_object));

	zend_object_std_init(&connection->zo, ce TSRMLS_CC);
	AMQP_OBJECT_PROPERTIES_INIT(connection->zo, ce);

	new_value.handle = zend_objects_store_put(
		connection,
		(zend_objects_store_dtor_t)zend_objects_destroy_object,
		(zend_objects_free_object_storage_t)amqp_connection_dtor,
		NULL TSRMLS_CC
	);

#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3
	memcpy((void *)&amqp_connection_object_handlers, (void *)zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	amqp_connection_object_handlers.get_debug_info = amqp_connection_object_get_debug_info;
	new_value.handlers = &amqp_connection_object_handlers;
#else
	new_value.handlers = zend_get_std_object_handlers();
#endif
	return new_value;
}


/* {{{ proto AMQPConnection::__construct([array optional])
 * The array can contain 'host', 'port', 'login', 'password', 'vhost', 'read_timeout', 'write_timeout', 'connect_timeout' and 'timeout' (deprecated) indexes
 */
PHP_METHOD(amqp_connection_class, __construct)
{
	zval *id;
	amqp_connection_object *connection;

	zval* ini_arr = NULL;
	zval** zdata;

	/* Parse out the method parameters */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O|a", &id, amqp_connection_class_entry, &ini_arr) == FAILURE) {
		return;
	}

	/* Pull the connection off of the store */
	connection = (amqp_connection_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Pull the login out of the $params array */
	zdata = NULL;
	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "login", sizeof("login"), (void*)&zdata)) {
		convert_to_string(*zdata);
	}
	/* Validate the given login */
	if (zdata && Z_STRLEN_PP(zdata) > 0) {
		if (Z_STRLEN_PP(zdata) < 128) {
			connection->login = estrndup(Z_STRVAL_PP(zdata), Z_STRLEN_PP(zdata));
		} else {
			zend_throw_exception(amqp_connection_exception_class_entry, "Parameter 'login' exceeds 128 character limit.", 0 TSRMLS_CC);
			return;
		}
	} else {
		connection->login = estrndup(INI_STR("amqp.login"), strlen(INI_STR("amqp.login")) > 128 ? 128 : strlen(INI_STR("amqp.login")));
	}

	/* Pull the password out of the $params array */
	zdata = NULL;
	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF(ini_arr), "password", sizeof("password"), (void*)&zdata)) {
		convert_to_string(*zdata);
	}
	/* Validate the given password */
	if (zdata && Z_STRLEN_PP(zdata) > 0) {
		if (Z_STRLEN_PP(zdata) < 128) {
			connection->password = estrndup(Z_STRVAL_PP(zdata), Z_STRLEN_PP(zdata));
		} else {
			zend_throw_exception(amqp_connection_exception_class_entry, "Parameter 'password' exceeds 128 character limit.", 0 TSRMLS_CC);
			return;
		}
	} else {
		connection->password = estrndup(INI_STR("amqp.password"), strlen(INI_STR("amqp.password")) > 128 ? 128 : strlen(INI_STR("amqp.password")));
	}

	/* Pull the host out of the $params array */
	zdata = NULL;
	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF(ini_arr), "host", sizeof("host"), (void *)&zdata)) {
		convert_to_string(*zdata);
	}
	/* Validate the given host */
	if (zdata && Z_STRLEN_PP(zdata) > 0) {
		if (Z_STRLEN_PP(zdata) < 128) {
			connection->host = estrndup(Z_STRVAL_PP(zdata), Z_STRLEN_PP(zdata));
		} else {
			zend_throw_exception(amqp_connection_exception_class_entry, "Parameter 'host' exceeds 128 character limit.", 0 TSRMLS_CC);
			return;
		}
	} else {
		connection->host = estrndup(INI_STR("amqp.host"), strlen(INI_STR("amqp.host")) > 128 ? 128 : strlen(INI_STR("amqp.host")));
	}

	/* Pull the vhost out of the $params array */
	zdata = NULL;
	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "vhost", sizeof("vhost"), (void*)&zdata)) {
		convert_to_string(*zdata);
	}
	/* Validate the given vhost */
	if (zdata && Z_STRLEN_PP(zdata) > 0) {
		if (Z_STRLEN_PP(zdata) < 128) {
			connection->vhost = estrndup(Z_STRVAL_PP(zdata), Z_STRLEN_PP(zdata));
		} else {
			zend_throw_exception(amqp_connection_exception_class_entry, "Parameter 'vhost' exceeds 128 character limit.", 0 TSRMLS_CC);
			return;
		}
	} else {
		connection->vhost = estrndup(INI_STR("amqp.vhost"), strlen(INI_STR("amqp.vhost")) > 128 ? 128 : strlen(INI_STR("amqp.vhost")));
	}

	connection->port = INI_INT("amqp.port");

	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "port", sizeof("port"), (void*)&zdata)) {
		convert_to_long(*zdata);
		connection->port = (size_t)Z_LVAL_PP(zdata);
	}

	connection->read_timeout = INI_FLT("amqp.read_timeout");

	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "read_timeout", sizeof("read_timeout"), (void*)&zdata)) {
		convert_to_double(*zdata);
		if (Z_DVAL_PP(zdata) < 0) {
			zend_throw_exception(amqp_connection_exception_class_entry, "Parameter 'read_timeout' must be greater than or equal to zero.", 0 TSRMLS_CC);
		} else {
			connection->read_timeout = Z_DVAL_PP(zdata);
		}

		if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "timeout", sizeof("timeout"), (void*)&zdata)) {
			/* 'read_timeout' takes precedence on 'timeout' but users have to know this */
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Parameter 'timeout' is deprecated, 'read_timeout' used instead");
		}

	} else if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "timeout", sizeof("timeout"), (void*)&zdata)) {

		php_error_docref(NULL TSRMLS_CC, E_DEPRECATED, "Parameter 'timeout' is deprecated; use 'read_timeout' instead");

		convert_to_double(*zdata);
		if (Z_DVAL_PP(zdata) < 0) {
			zend_throw_exception(amqp_connection_exception_class_entry, "Parameter 'timeout' must be greater than or equal to zero.", 0 TSRMLS_CC);
		} else {
			connection->read_timeout = Z_DVAL_PP(zdata);
		}
	} else {

		assert(DEFAULT_TIMEOUT == NULL);
		if (strcmp(DEFAULT_TIMEOUT, INI_STR("amqp.timeout")) != 0) {
			php_error_docref(NULL TSRMLS_CC, E_DEPRECATED, "INI setting 'amqp.timeout' is deprecated; use 'amqp.read_timeout' instead");

			assert(DEFAULT_READ_TIMEOUT != NULL);
			if (strcmp(DEFAULT_READ_TIMEOUT, INI_STR("amqp.read_timeout")) == 0) {
				connection->read_timeout = INI_FLT("amqp.timeout");
			} else {
				php_error_docref(NULL TSRMLS_CC, E_NOTICE, "INI setting 'amqp.read_timeout' will be used instead of 'amqp.timeout'");
				connection->read_timeout = INI_FLT("amqp.read_timeout");
			}
		} else {
			connection->read_timeout = INI_FLT("amqp.read_timeout");
		}
	}

	connection->write_timeout = INI_FLT("amqp.write_timeout");

	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "write_timeout", sizeof("write_timeout"), (void*)&zdata)) {
		convert_to_double(*zdata);
		if (Z_DVAL_PP(zdata) < 0) {
			zend_throw_exception(amqp_connection_exception_class_entry, "Parameter 'write_timeout' must be greater than or equal to zero.", 0 TSRMLS_CC);
		} else {
			connection->write_timeout = Z_DVAL_PP(zdata);
		}
	}

	connection->connect_timeout = INI_FLT("amqp.connect_timeout");

	if (ini_arr && SUCCESS == zend_hash_find(HASH_OF (ini_arr), "connect_timeout", sizeof("connect_timeout"), (void*)&zdata)) {
		convert_to_double(*zdata);
		if (Z_DVAL_PP(zdata) < 0) {
			zend_throw_exception(amqp_connection_exception_class_entry, "Parameter 'connect_timeout' must be greater than or equal to zero.", 0 TSRMLS_CC);
		} else {
			connection->connect_timeout = Z_DVAL_PP(zdata);
		}
	}
}
/* }}} */


/* {{{ proto amqp::isConnected()
check amqp connection */
PHP_METHOD(amqp_connection_class, isConnected)
{
	zval *id;
	amqp_connection_object *connection;

	/* Try to pull amqp object out of method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_connection_class_entry) == FAILURE) {
		return;
	}

	/* Get the connection object out of the store */
	connection = (amqp_connection_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* If the channel_connect is 1, we have a connection */
	if (connection->is_connected == '\1') {
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
	zval *id;
	amqp_connection_object *connection;

	/* Try to pull amqp object out of method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_connection_class_entry) == FAILURE) {
		return;
	}

	/* Get the connection object out of the store */
	connection = (amqp_connection_object *)zend_object_store_get_object(id TSRMLS_CC);

	if (connection->is_connected) {

		assert(connection->connection_resource != NULL);
		if (connection->is_persistent) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Attempt to start transient connection while persistent transient one already established. Continue.");
		}

		RETURN_TRUE;
	}

	/* Actually connect this resource to the broker */
	RETURN_BOOL(php_amqp_connect(connection, 0 TSRMLS_CC));
}
/* }}} */


/* {{{ proto amqp::connect()
create amqp connection */
PHP_METHOD(amqp_connection_class, pconnect)
{
	zval *id;
	amqp_connection_object *connection;

	/* Try to pull amqp object out of method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_connection_class_entry) == FAILURE) {
		return;
	}

	/* Get the connection object out of the store */
	connection = (amqp_connection_object *)zend_object_store_get_object(id TSRMLS_CC);

	if (connection->is_connected) {

		assert(connection->connection_resource != NULL);
		if (!connection->is_persistent) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Attempt to start persistent connection while transient one already established. Continue.");
		}

		RETURN_TRUE;
	}

	/* Actually connect this resource to the broker or use stored connection */
	RETURN_BOOL(php_amqp_connect(connection, 1 TSRMLS_CC));
}
/* }}} */


/* {{{ proto amqp:pdisconnect()
destroy amqp persistent connection */
PHP_METHOD(amqp_connection_class, pdisconnect)
{
	zval *id;
	amqp_connection_object *connection;

	/* Try to pull amqp object out of method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_connection_class_entry) == FAILURE) {
		return;
	}

	/* Get the connection object out of the store */
	connection = (amqp_connection_object *)zend_object_store_get_object(id TSRMLS_CC);

	if (!connection->is_connected) {
		RETURN_TRUE;
	}

	assert(connection->connection_resource != NULL);

	if (!connection->is_persistent) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Attempt to close persistent connection while transient one already established. Abort.");

		RETURN_FALSE;
	}

	php_amqp_disconnect_force(connection TSRMLS_CC);

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto amqp::disconnect()
destroy amqp connection */
PHP_METHOD(amqp_connection_class, disconnect)
{
	zval *id;
	amqp_connection_object *connection;

	/* Try to pull amqp object out of method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_connection_class_entry) == FAILURE) {
		return;
	}

	/* Get the connection object out of the store */
	connection = (amqp_connection_object *)zend_object_store_get_object(id TSRMLS_CC);

	if (!connection->is_connected) {
		RETURN_TRUE;
	}

	if (connection->is_persistent) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Attempt to close transient connection while persistent one already established. Abort.");

		RETURN_FALSE;
	}

	assert(connection->connection_resource != NULL);

	php_amqp_disconnect_safe(connection TSRMLS_CC);

	RETURN_TRUE;
}

/* }}} */

/* {{{ proto amqp::reconnect()
recreate amqp connection */
PHP_METHOD(amqp_connection_class, reconnect)
{
	zval *id;
	amqp_connection_object *connection;

	/* Try to pull amqp object out of method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_connection_class_entry) == FAILURE) {
		return;
	}

	/* Get the connection object out of the store */
	connection = (amqp_connection_object *)zend_object_store_get_object(id TSRMLS_CC);

	if (connection->is_connected == '\1') {

		assert(connection->connection_resource != NULL);

		if (connection->is_persistent) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Attempt to reconnect persistent connection while transient one already established. Abort.");

			RETURN_FALSE;
		}

		php_amqp_disconnect_safe(connection TSRMLS_CC);
	}

	RETURN_BOOL(php_amqp_connect(connection, 0 TSRMLS_CC));
}
/* }}} */

/* {{{ proto amqp::preconnect()
recreate amqp connection */
PHP_METHOD(amqp_connection_class, preconnect)
{
	zval *id;
	amqp_connection_object *connection;

	/* Try to pull amqp object out of method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_connection_class_entry) == FAILURE) {
		return;
	}

	/* Get the connection object out of the store */
	connection = (amqp_connection_object *)zend_object_store_get_object(id TSRMLS_CC);


	if (connection->is_connected == '\1') {

		assert(connection->connection_resource != NULL);

		if (!connection->is_persistent) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Attempt to reconnect transient connection while persistent one already established. Abort.");

			RETURN_FALSE;
		}

		php_amqp_disconnect_force(connection TSRMLS_CC);
	}

	RETURN_BOOL(php_amqp_connect(connection, 1 TSRMLS_CC));
}
/* }}} */


/* {{{ proto amqp::getLogin()
get the login */
PHP_METHOD(amqp_connection_class, getLogin)
{
	zval *id;
	amqp_connection_object *connection;

	/* Get the login from the method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_connection_class_entry) == FAILURE) {
		return;
	}

	/* Get the connection object out of the store */
	connection = (amqp_connection_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Copy the login to the amqp object */
	RETURN_STRING(connection->login, 1);
}
/* }}} */


/* {{{ proto amqp::setLogin(string login)
set the login */
PHP_METHOD(amqp_connection_class, setLogin)
{
	zval *id;
	amqp_connection_object *connection;
	char *login;
	int login_len;

	/* Get the login from the method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os", &id, amqp_connection_class_entry, &login, &login_len) == FAILURE) {
		return;
	}

	/* Validate login length */
	if (login_len > 128) {
		zend_throw_exception(amqp_connection_exception_class_entry, "Invalid 'login' given, exceeds 128 characters limit.", 0 TSRMLS_CC);
		return;
	}

	/* Get the connection object out of the store */
	connection = (amqp_connection_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Free previously existing login, in cases where setLogin() is called multiple times */
	if (connection->login) {
		efree(connection->login);
	}

	/* Copy the login to the amqp object */
	connection->login = estrndup(login, login_len);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto amqp::getPassword()
get the password */
PHP_METHOD(amqp_connection_class, getPassword)
{
	zval *id;
	amqp_connection_object *connection;

	/* Get the password from the method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_connection_class_entry) == FAILURE) {
		return;
	}

	/* Get the connection object out of the store */
	connection = (amqp_connection_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Copy the password to the amqp object */
	RETURN_STRING(connection->password, 1);
}
/* }}} */


/* {{{ proto amqp::setPassword(string password)
set the password */
PHP_METHOD(amqp_connection_class, setPassword)
{
	zval *id;
	amqp_connection_object *connection;
	char *password;
	int password_len;

	/* Get the password from the method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os", &id, amqp_connection_class_entry, &password, &password_len) == FAILURE) {
		return;
	}

	/* Validate password length */
	if (password_len > 128) {
		zend_throw_exception(amqp_connection_exception_class_entry, "Invalid 'password' given, exceeds 128 characters limit.", 0 TSRMLS_CC);
		return;
	}

	/* Get the connection object out of the store */
	connection = (amqp_connection_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Free previously existing password, in cases where setPassword() is called multiple times */
	if (connection->password) {
		efree(connection->password);
	}

	/* Copy the password to the amqp object */
	connection->password = estrndup(password, password_len);

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto amqp::getHost()
get the host */
PHP_METHOD(amqp_connection_class, getHost)
{
	zval *id;
	amqp_connection_object *connection;

	/* Get the host from the method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_connection_class_entry) == FAILURE) {
		return;
	}

	/* Get the connection object out of the store */
	connection = (amqp_connection_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Copy the host to the amqp object */
	RETURN_STRING(connection->host, 1);
}
/* }}} */


/* {{{ proto amqp::setHost(string host)
set the host */
PHP_METHOD(amqp_connection_class, setHost)
{
	zval *id;
	amqp_connection_object *connection;
	char *host;
	int host_len;

	/* Get the host from the method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os", &id, amqp_connection_class_entry, &host, &host_len) == FAILURE) {
		return;
	}

	/* Validate host length */
	if (host_len > 1024) {
		zend_throw_exception(amqp_connection_exception_class_entry, "Invalid 'host' given, exceeds 1024 character limit.", 0 TSRMLS_CC);
		return;
	}

	/* Get the connection object out of the store */
	connection = (amqp_connection_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Free previously existing host, in cases where setHost() is called multiple times */
	if (connection->host) {
		efree(connection->host);
	}

	/* Copy the host to the amqp object */
	connection->host = estrndup(host, host_len);

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto amqp::getPort()
get the port */
PHP_METHOD(amqp_connection_class, getPort)
{
	zval *id;
	amqp_connection_object *connection;

	/* Get the port from the method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_connection_class_entry) == FAILURE) {
		return;
	}

	/* Get the connection object out of the store */
	connection = (amqp_connection_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Copy the port to the amqp object */
	RETURN_LONG(connection->port);
}
/* }}} */


/* {{{ proto amqp::setPort(mixed port)
set the port */
PHP_METHOD(amqp_connection_class, setPort)
{
	zval *id;
	amqp_connection_object *connection;
	zval *zvalPort;
	int port;

	/* Get the port from the method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Oz", &id, amqp_connection_class_entry, &zvalPort) == FAILURE) {
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

	/* Get the connection object out of the store */
	connection = (amqp_connection_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Copy the port to the amqp object */
	connection->port = port;

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto amqp::getVhost()
get the vhost */
PHP_METHOD(amqp_connection_class, getVhost)
{
	zval *id;
	amqp_connection_object *connection;

	/* Get the vhost from the method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_connection_class_entry) == FAILURE) {
		return;
	}

	/* Get the connection object out of the store */
	connection = (amqp_connection_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Copy the vhost to the amqp object */
	RETURN_STRING(connection->vhost, 1);
}
/* }}} */


/* {{{ proto amqp::setVhost(string vhost)
set the vhost */
PHP_METHOD(amqp_connection_class, setVhost)
{
	zval *id;
	amqp_connection_object *connection;
	char *vhost;
	int vhost_len;

	/* Get the vhost from the method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os", &id, amqp_connection_class_entry, &vhost, &vhost_len) == FAILURE) {
		return;
	}

	/* Validate vhost length */
	if (vhost_len > 128) {
		zend_throw_exception(amqp_connection_exception_class_entry, "Parameter 'vhost' exceeds 128 characters limit.", 0 TSRMLS_CC);
		return;
	}

	/* Get the connection object out of the store */
	connection = (amqp_connection_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Free previously existing vhost, in cases where setVhost() is called multiple times */
	if (connection->vhost) {
		efree(connection->vhost);
	}

	/* Copy the vhost to the amqp object */
	connection->vhost = estrndup(vhost, vhost_len);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto amqp::getTimeout()
@deprecated
get the timeout */
PHP_METHOD(amqp_connection_class, getTimeout)
{
	zval *id;
	amqp_connection_object *connection;

	php_error_docref(NULL TSRMLS_CC, E_DEPRECATED, "AMQPConnection::getTimeout() method is deprecated; use AMQPConnection::getReadTimeout() instead");

	/* Get the timeout from the method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_connection_class_entry) == FAILURE) {
		return;
	}

	/* Get the connection object out of the store */
	connection = (amqp_connection_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Copy the timeout to the amqp object */
	RETURN_DOUBLE(connection->read_timeout);
}
/* }}} */

/* {{{ proto amqp::setTimeout(double timeout)
@deprecated
set the timeout */
PHP_METHOD(amqp_connection_class, setTimeout)
{
	zval *id;
	amqp_connection_object *connection;
	double read_timeout;

	php_error_docref(NULL TSRMLS_CC, E_DEPRECATED, "AMQPConnection::setTimeout($timeout) method is deprecated; use AMQPConnection::setReadTimeout($timeout) instead");

	/* Get the timeout from the method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Od", &id, amqp_connection_class_entry, &read_timeout) == FAILURE) {
		return;
	}

	/* Validate timeout */
	if (read_timeout < 0) {
		zend_throw_exception(amqp_connection_exception_class_entry, "Parameter 'timeout' must be greater than or equal to zero.", 0 TSRMLS_CC);
		return;
	}

	/* Get the connection object out of the store */
	connection = (amqp_connection_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Copy the timeout to the amqp object */
	connection->read_timeout = read_timeout;

	if (connection->is_connected == '\1') {
		if (php_amqp_set_resource_read_timeout(connection->connection_resource, connection->read_timeout TSRMLS_CC) == 0) {

			php_amqp_disconnect_force(connection TSRMLS_CC);
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
	zval *id;
	amqp_connection_object *connection;

	/* Get the timeout from the method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_connection_class_entry) == FAILURE) {
		return;
	}

	/* Get the connection object out of the store */
	connection = (amqp_connection_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Copy the timeout to the amqp object */
	RETURN_DOUBLE(connection->read_timeout);
}
/* }}} */

/* {{{ proto amqp::setReadTimeout(double timeout)
set read timeout */
PHP_METHOD(amqp_connection_class, setReadTimeout)
{
	zval *id;
	amqp_connection_object *connection;
	double read_timeout;

	/* Get the timeout from the method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Od", &id, amqp_connection_class_entry, &read_timeout) == FAILURE) {
		return;
	}

	/* Validate timeout */
	if (read_timeout < 0) {
		zend_throw_exception(amqp_connection_exception_class_entry, "Parameter 'read_timeout' must be greater than or equal to zero.", 0 TSRMLS_CC);
		return;
	}

	/* Get the connection object out of the store */
	connection = (amqp_connection_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Copy the timeout to the amqp object */
	connection->read_timeout = read_timeout;

	if (connection->is_connected == '\1') {
		if (php_amqp_set_resource_read_timeout(connection->connection_resource, connection->read_timeout TSRMLS_CC) == 0) {
			php_amqp_disconnect_force(connection TSRMLS_CC);

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
	zval *id;
	amqp_connection_object *connection;

	/* Get the timeout from the method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_connection_class_entry) == FAILURE) {
		return;
	}

	/* Get the connection object out of the store */
	connection = (amqp_connection_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Copy the timeout to the amqp object */
	RETURN_DOUBLE(connection->write_timeout);
}
/* }}} */

/* {{{ proto amqp::setWriteTimeout(double timeout)
set write timeout */
PHP_METHOD(amqp_connection_class, setWriteTimeout)
{
	zval *id;
	amqp_connection_object *connection;
	double write_timeout;

	/* Get the timeout from the method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Od", &id, amqp_connection_class_entry, &write_timeout) == FAILURE) {
		return;
	}

	/* Validate timeout */
	if (write_timeout < 0) {
		zend_throw_exception(amqp_connection_exception_class_entry, "Parameter 'write_timeout' must be greater than or equal to zero.", 0 TSRMLS_CC);
		return;
	}

	/* Get the connection object out of the store */
	connection = (amqp_connection_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Copy the timeout to the amqp object */
	connection->write_timeout = write_timeout;

	if (connection->is_connected == '\1') {
		if (php_amqp_set_resource_write_timeout(connection->connection_resource, connection->write_timeout TSRMLS_CC) == 0) {

			php_amqp_disconnect_force(connection TSRMLS_CC);
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
	zval *id;
	amqp_connection_object *connection;

	/* Get the timeout from the method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_connection_class_entry) == FAILURE) {
		return;
	}

	/* Get the connection object out of the store */
	connection = (amqp_connection_object *)zend_object_store_get_object(id TSRMLS_CC);

	if (!connection->is_connected) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Connection is not connected.");

		RETURN_LONG(0);
	}

	/* Copy the timeout to the amqp object */
	RETURN_LONG(connection->connection_resource->used_slots);
}
/* }}} */

/* {{{ proto amqp::getMaxChannels()
Get max supported channels number per connection*/
PHP_METHOD(amqp_connection_class, getMaxChannels)
{
	zval *id;
	amqp_connection_object *connection;

	/* Get the timeout from the method params */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_connection_class_entry) == FAILURE) {
		return;
	}

	/* Get the connection object out of the store */
	connection = (amqp_connection_object *)zend_object_store_get_object(id TSRMLS_CC);

	if (!connection->is_connected) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Connection is not connected.");

		RETURN_LONG(0);
	}

	/* Copy the timeout to the amqp object */
	RETURN_LONG(amqp_get_channel_max(connection->connection_resource->connection_state));
}
/* }}} */


/* {{{ proto amqp::isPersistent()
check whether amqp connection is persistent */
PHP_METHOD(amqp_connection_class, isPersistent)
{
	zval *id;
	amqp_connection_object *connection;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_connection_class_entry) == FAILURE) {
		return;
	}

	/* Get the connection object out of the store */
	connection = (amqp_connection_object *)zend_object_store_get_object(id TSRMLS_CC);

	RETURN_BOOL(connection->is_persistent);
}
/* }}} */


/*
*Local variables:
*tab-width: 4
*c-basic-offset: 4
*End:
*vim600: noet sw=4 ts=4 fdm=marker
*vim<6
*/
