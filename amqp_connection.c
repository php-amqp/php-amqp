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
    #include <rabbitmq-c/tcp_socket.h>
#else
    #include <amqp.h>
    #include <amqp_framing.h>
    #include <amqp_tcp_socket.h>
#endif

#ifdef PHP_WIN32
    #include "win32/unistd.h"
#else
    #include <unistd.h>
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

#define PHP_AMQP_EXTRACT_CONNECTION_STR(name)                                                                          \
    zdata = NULL;                                                                                                      \
    if (ini_arr && (zdata = zend_hash_str_find(HASH_OF(ini_arr), ZEND_STRL(name))) != NULL) {                          \
        SEPARATE_ZVAL(zdata);                                                                                          \
        convert_to_string(zdata);                                                                                      \
    }                                                                                                                  \
    if (zdata && Z_STRLEN_P(zdata) > 0) {                                                                              \
        zend_update_property_string(                                                                                   \
            this_ce,                                                                                                   \
            PHP_AMQP_COMPAT_OBJ_P(getThis()),                                                                          \
            ZEND_STRL(name),                                                                                           \
            Z_STRVAL_P(zdata) TSRMLS_CC                                                                                \
        );                                                                                                             \
    } else {                                                                                                           \
        zend_update_property_string(                                                                                   \
            this_ce,                                                                                                   \
            PHP_AMQP_COMPAT_OBJ_P(getThis()),                                                                          \
            ZEND_STRL(name),                                                                                           \
            INI_STR("amqp." name) TSRMLS_CC                                                                            \
        );                                                                                                             \
    }

#define PHP_AMQP_EXTRACT_CONNECTION_BOOL(name)                                                                         \
    zdata = NULL;                                                                                                      \
    if (ini_arr && (zdata = zend_hash_str_find(HASH_OF(ini_arr), (name), sizeof(name))) != NULL) {                     \
        SEPARATE_ZVAL(zdata);                                                                                          \
        convert_to_long(zdata);                                                                                        \
    }                                                                                                                  \
    if (zdata) {                                                                                                       \
        zend_update_property_bool(                                                                                     \
            this_ce,                                                                                                   \
            PHP_AMQP_COMPAT_OBJ_P(getThis()),                                                                          \
            ZEND_STRL(name),                                                                                           \
            Z_LVAL_P(zdata) TSRMLS_CC                                                                                  \
        );                                                                                                             \
    } else {                                                                                                           \
        zend_update_property_bool(                                                                                     \
            this_ce,                                                                                                   \
            PHP_AMQP_COMPAT_OBJ_P(getThis()),                                                                          \
            ZEND_STRL(name),                                                                                           \
            INI_INT("amqp." name) TSRMLS_CC                                                                            \
        );                                                                                                             \
    }

static int php_amqp_connection_resource_deleter(zval *el, amqp_connection_resource *connection_resource TSRMLS_DC)
{
    if (Z_RES_P(el)->ptr == connection_resource) {
        return ZEND_HASH_APPLY_REMOVE | ZEND_HASH_APPLY_STOP;
    }

    return ZEND_HASH_APPLY_KEEP;
}

static size_t php_amqp_get_connection_hash(amqp_connection_params *params, char **hash)
{
    return spprintf(
        hash,
        0,
        "amqp_conn_res_h:%s_p:%d_v:%s_l:%s_p:%s_f:%d_c:%d_h:%d_cacert:%s_cert:%s_key:%s_sasl_method:%d_connection_name:"
        "%s",
        params->host,
        params->port,
        params->vhost,
        params->login,
        params->password,
        params->frame_max,
        params->channel_max,
        params->heartbeat,
        params->cacert,
        params->cert,
        params->key,
        params->sasl_method,
        params->connection_name
    );
}

static void php_amqp_cleanup_connection_resource(amqp_connection_resource *connection_resource TSRMLS_DC)
{
    if (!connection_resource) {
        return;
    }

    zend_resource *resource = connection_resource->resource;

    connection_resource->parent->connection_resource = NULL;
    connection_resource->parent = NULL;

    if (connection_resource->is_dirty) {
        if (connection_resource->is_persistent) {
            zend_hash_apply_with_argument(
                &EG(persistent_list),
                (apply_func_arg_t) php_amqp_connection_resource_deleter,
                (void *) connection_resource TSRMLS_CC
            );
        }

        zend_list_delete(resource);
    } else {
        if (connection_resource->is_persistent) {
            connection_resource->resource = NULL;
        }

        if (connection_resource->resource != NULL) {
            zend_list_delete(resource);
        }
    }
}

static void php_amqp_disconnect(amqp_connection_resource *resource TSRMLS_DC)
{
    php_amqp_prepare_for_disconnect(resource TSRMLS_CC);
    php_amqp_cleanup_connection_resource(resource TSRMLS_CC);
}


void php_amqp_disconnect_force(amqp_connection_resource *resource TSRMLS_DC)
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
    zval rv;

    char *key = NULL;
    size_t key_len = 0;

    if (connection->connection_resource) {
        /* Clean up old memory allocations which are now invalid (new connection) */
        php_amqp_cleanup_connection_resource(connection->connection_resource TSRMLS_CC);
    }

    assert(connection->connection_resource == NULL);

    amqp_connection_params connection_params;

    connection_params.host = PHP_AMQP_READ_THIS_PROP_STR("host");
    connection_params.port = (int) PHP_AMQP_READ_THIS_PROP_LONG("port");
    connection_params.vhost = PHP_AMQP_READ_THIS_PROP_STR("vhost");
    connection_params.login = PHP_AMQP_READ_THIS_PROP_STR("login");
    connection_params.password = PHP_AMQP_READ_THIS_PROP_STR("password");
    connection_params.frame_max = (int) PHP_AMQP_READ_THIS_PROP_LONG("frameMax");
    connection_params.channel_max = (int) PHP_AMQP_READ_THIS_PROP_LONG("channelMax");
    connection_params.heartbeat = (int) PHP_AMQP_READ_THIS_PROP_LONG("heartbeat");
    connection_params.read_timeout = PHP_AMQP_READ_THIS_PROP_DOUBLE("readTimeout");
    connection_params.write_timeout = PHP_AMQP_READ_THIS_PROP_DOUBLE("writeTimeout");
    connection_params.connect_timeout = PHP_AMQP_READ_THIS_PROP_DOUBLE("connectTimeout");
    connection_params.rpc_timeout = PHP_AMQP_READ_THIS_PROP_DOUBLE("rpcTimeout");
    connection_params.cacert = PHP_AMQP_READ_THIS_PROP_STRLEN("cacert") ? PHP_AMQP_READ_THIS_PROP_STR("cacert") : NULL;
    connection_params.cert = PHP_AMQP_READ_THIS_PROP_STRLEN("cert") ? PHP_AMQP_READ_THIS_PROP_STR("cert") : NULL;
    connection_params.key = PHP_AMQP_READ_THIS_PROP_STRLEN("key") ? PHP_AMQP_READ_THIS_PROP_STR("key") : NULL;
    connection_params.verify = (int) PHP_AMQP_READ_THIS_PROP_BOOL("verify");
    connection_params.sasl_method = (int) PHP_AMQP_READ_THIS_PROP_LONG("saslMethod");
    connection_params.connection_name =
        PHP_AMQP_READ_THIS_PROP_STRLEN("connectionName") ? PHP_AMQP_READ_THIS_PROP_STR("connectionName") : NULL;

    if (persistent) {
        zend_resource *le = NULL;

        /* Look for an established resource */
        key_len = php_amqp_get_connection_hash(&connection_params, &key);

        if ((le = zend_hash_str_find_ptr(&EG(persistent_list), key, key_len)) != NULL) {
            efree(key);

            if (le->type != le_amqp_connection_resource_persistent) {
                /* hash conflict, given name associate with non-amqp persistent connection resource */
                zend_throw_exception(
                    amqp_connection_exception_class_entry,
                    "Connection hash conflict detected. Persistent connection found that does not belong to AMQP.",
                    0 TSRMLS_CC
                );
                return 0;
            }

            /* An entry for this connection resource already exists */
            /* Stash the connection resource in the connection */
            connection->connection_resource = le->ptr;

            if (connection->connection_resource->resource != NULL) {
                /*  resource in use! */
                connection->connection_resource = NULL;

                zend_throw_exception(
                    amqp_connection_exception_class_entry,
                    "There are already established persistent connection to the same resource.",
                    0 TSRMLS_CC
                );
                return 0;
            }

            connection->connection_resource->resource = zend_register_resource(
                connection->connection_resource,
                persistent ? le_amqp_connection_resource_persistent : le_amqp_connection_resource
            );
            connection->connection_resource->parent = connection;

            /* Set desired timeouts */
            if (php_amqp_set_resource_read_timeout(
                    connection->connection_resource,
                    PHP_AMQP_READ_THIS_PROP_DOUBLE("readTimeout") TSRMLS_CC
                ) == 0 ||
                php_amqp_set_resource_write_timeout(
                    connection->connection_resource,
                    PHP_AMQP_READ_THIS_PROP_DOUBLE("writeTimeout") TSRMLS_CC
                ) == 0 ||
                php_amqp_set_resource_rpc_timeout(
                    connection->connection_resource,
                    PHP_AMQP_READ_THIS_PROP_DOUBLE("rpcTimeout") TSRMLS_CC
                ) == 0) {

                php_amqp_disconnect_force(connection->connection_resource TSRMLS_CC);
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

    connection->connection_resource->resource = zend_register_resource(
        connection->connection_resource,
        persistent ? le_amqp_connection_resource_persistent : le_amqp_connection_resource
    );
    connection->connection_resource->parent = connection;

    /* Set connection status to connected */
    connection->connection_resource->is_connected = '\1';

    if (persistent) {
        connection->connection_resource->is_persistent = persistent;

        key_len = php_amqp_get_connection_hash(&connection_params, &key);

        zend_resource new_le;

        /* Store a reference in the persistence list */
        new_le.ptr = connection->connection_resource;
        new_le.type = le_amqp_connection_resource_persistent;

        if (!zend_hash_str_update_mem(&EG(persistent_list), key, key_len, &new_le, sizeof(zend_resource))) {
            efree(key);
            php_amqp_disconnect_force(connection->connection_resource TSRMLS_CC);

            zend_throw_exception(
                amqp_connection_exception_class_entry,
                "Could not store persistent connection in pool.",
                0 TSRMLS_CC
            );

            return 0;
        }
        efree(key);
    }

    return 1;
}

void amqp_connection_free(zend_object *object TSRMLS_DC)
{
    amqp_connection_object *connection = PHP_AMQP_FETCH_CONNECTION(object);

    if (connection->connection_resource) {
        php_amqp_disconnect(connection->connection_resource TSRMLS_CC);
    }

    zend_object_std_dtor(&connection->zo TSRMLS_CC);
}

zend_object *amqp_connection_ctor(zend_class_entry *ce TSRMLS_DC)
{
    amqp_connection_object *connection =
        (amqp_connection_object *) ecalloc(1, sizeof(amqp_connection_object) + zend_object_properties_size(ce));

    zend_object_std_init(&connection->zo, ce TSRMLS_CC);
    AMQP_OBJECT_PROPERTIES_INIT(connection->zo, ce);

    connection->zo.handlers = &amqp_connection_object_handlers;

    return &connection->zo;
}


/* {{{ proto AMQPConnection::__construct([array optional])
 * The array can contain 'host', 'port', 'login', 'password', 'vhost', 'read_timeout', 'write_timeout', 'connect_timeout', 'rpc_timeout' and 'timeout' (deprecated) indexes
 */
static PHP_METHOD(amqp_connection_class, __construct)
{
    zval *ini_arr = NULL;

    zval *zdata = NULL;

    /* Parse out the method parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|a/", &ini_arr) == FAILURE) {
        return;
    }

    /* Pull the login out of the $params array */
    zdata = NULL;
    if (ini_arr && (zdata = zend_hash_str_find(HASH_OF(ini_arr), ZEND_STRL("login"))) != NULL) {
        SEPARATE_ZVAL(zdata);
        convert_to_string(zdata);
    }
    /* Validate the given login */
    if (zdata && Z_STRLEN_P(zdata) > 0) {
        if (!php_amqp_is_valid_credential(Z_STR_P(zdata))) {
            zend_throw_exception_ex(
                amqp_connection_exception_class_entry,
                0 TSRMLS_CC,
                "Parameter 'login' exceeds %d character limit.",
                PHP_AMQP_MAX_CREDENTIALS_LENGTH
            );
            return;
        }

        zend_update_property(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("login"), zdata TSRMLS_CC);
    } else {
        zend_update_property_string(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(getThis()),
            ZEND_STRL("login"),
            INI_STR("amqp.login") TSRMLS_CC
        );
    }

    /* Pull the password out of the $params array */
    zdata = NULL;
    if (ini_arr && (zdata = zend_hash_str_find(HASH_OF(ini_arr), ZEND_STRL("password"))) != NULL) {
        SEPARATE_ZVAL(zdata);
        convert_to_string(zdata);
    }
    /* Validate the given password */
    if (zdata && Z_STRLEN_P(zdata) > 0) {
        if (!php_amqp_is_valid_credential(Z_STR_P(zdata))) {
            zend_throw_exception_ex(
                amqp_connection_exception_class_entry,
                0 TSRMLS_CC,
                "Parameter 'password' exceeds %d character limit.",
                PHP_AMQP_MAX_CREDENTIALS_LENGTH
            );
            return;
        }

        zend_update_property_stringl(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(getThis()),
            ZEND_STRL("password"),
            Z_STRVAL_P(zdata),
            Z_STRLEN_P(zdata) TSRMLS_CC
        );
    } else {
        zend_update_property_string(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(getThis()),
            ZEND_STRL("password"),
            INI_STR("amqp.password") TSRMLS_CC
        );
    }

    /* Pull the host out of the $params array */
    zdata = NULL;
    if (ini_arr && (zdata = zend_hash_str_find(HASH_OF(ini_arr), ZEND_STRL("host"))) != NULL) {
        SEPARATE_ZVAL(zdata);
        convert_to_string(zdata);
    }
    /* Validate the given host */
    if (zdata && Z_STRLEN_P(zdata) > 0) {
        if (!php_amqp_is_valid_identifier(Z_STR_P(zdata))) {
            zend_throw_exception_ex(
                amqp_connection_exception_class_entry,
                0 TSRMLS_CC,
                "Parameter 'host' exceeds %d character limit.",
                PHP_AMQP_MAX_IDENTIFIER_LENGTH
            );
            return;
        }

        zend_update_property_stringl(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(getThis()),
            ZEND_STRL("host"),
            Z_STRVAL_P(zdata),
            Z_STRLEN_P(zdata) TSRMLS_CC
        );
    } else {
        zend_update_property_string(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(getThis()),
            ZEND_STRL("host"),
            INI_STR("amqp.host") TSRMLS_CC
        );
    }

    /* Pull the vhost out of the $params array */
    zdata = NULL;
    if (ini_arr && (zdata = zend_hash_str_find(HASH_OF(ini_arr), ZEND_STRL("vhost"))) != NULL) {
        SEPARATE_ZVAL(zdata);
        convert_to_string(zdata);
    }
    /* Validate the given vhost */
    if (zdata && Z_STRLEN_P(zdata) > 0) {
        if (!php_amqp_is_valid_identifier(Z_STR_P(zdata))) {
            zend_throw_exception_ex(
                amqp_connection_exception_class_entry,
                0 TSRMLS_CC,
                "Parameter 'vhost' exceeds %d character limit.",
                PHP_AMQP_MAX_IDENTIFIER_LENGTH
            );
            return;
        }

        zend_update_property_stringl(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(getThis()),
            ZEND_STRL("vhost"),
            Z_STRVAL_P(zdata),
            Z_STRLEN_P(zdata) TSRMLS_CC
        );
    } else {
        zend_update_property_string(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(getThis()),
            ZEND_STRL("vhost"),
            INI_STR("amqp.vhost") TSRMLS_CC
        );
    }

    zend_update_property_long(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("port"),
        INI_INT("amqp.port") TSRMLS_CC
    );

    if (ini_arr && (zdata = zend_hash_str_find(HASH_OF(ini_arr), ZEND_STRL("port"))) != NULL) {
        SEPARATE_ZVAL(zdata);
        convert_to_long(zdata);

        if (!php_amqp_is_valid_port(Z_LVAL_P(zdata))) {
            zend_throw_exception_ex(
                amqp_connection_exception_class_entry,
                0 TSRMLS_CC,
                "Parameter 'port' must be a valid port number  between %d and %d.",
                PHP_AMQP_MIN_PORT,
                PHP_AMQP_MAX_PORT
            );
            return;
        }

        zend_update_property_long(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(getThis()),
            ZEND_STRL("port"),
            Z_LVAL_P(zdata) TSRMLS_CC
        );
    }

    zend_update_property_double(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("readTimeout"),
        INI_FLT("amqp.read_timeout") TSRMLS_CC
    );

    if (ini_arr && (zdata = zend_hash_str_find(HASH_OF(ini_arr), ZEND_STRL("read_timeout"))) != NULL) {
        SEPARATE_ZVAL(zdata);
        convert_to_double(zdata);

        if (!php_amqp_is_valid_timeout(Z_DVAL_P(zdata))) {
            zend_throw_exception(
                amqp_connection_exception_class_entry,
                "Parameter 'read_timeout' must be greater than or equal to zero.",
                0 TSRMLS_CC
            );
            return;
        }

        zend_update_property_double(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(getThis()),
            ZEND_STRL("readTimeout"),
            Z_DVAL_P(zdata) TSRMLS_CC
        );

        if (ini_arr && (zdata = zend_hash_str_find(HASH_OF(ini_arr), ZEND_STRL("timeout"))) != NULL) {
            /* 'read_timeout' takes precedence on 'timeout' but users have to know this */
            php_error_docref(
                NULL TSRMLS_CC,
                E_NOTICE,
                "Parameter 'timeout' is deprecated, 'read_timeout' used instead"
            );
        }

    } else if (ini_arr && (zdata = zend_hash_str_find(HASH_OF(ini_arr), ZEND_STRL("timeout"))) != NULL) {

        php_error_docref(NULL TSRMLS_CC, E_DEPRECATED, "Parameter 'timeout' is deprecated; use 'read_timeout' instead");

        SEPARATE_ZVAL(zdata);
        convert_to_double(zdata);

        if (!php_amqp_is_valid_timeout(Z_DVAL_P(zdata))) {
            zend_throw_exception(
                amqp_connection_exception_class_entry,
                "Parameter 'timeout' must be greater than or equal to zero.",
                0 TSRMLS_CC
            );
            return;
        }

        zend_update_property_double(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(getThis()),
            ZEND_STRL("readTimeout"),
            Z_DVAL_P(zdata) TSRMLS_CC
        );
    } else {

        assert(DEFAULT_TIMEOUT != NULL);
        if (strcmp(DEFAULT_TIMEOUT, INI_STR("amqp.timeout")) != 0) {
            php_error_docref(
                NULL TSRMLS_CC,
                E_DEPRECATED,
                "INI setting 'amqp.timeout' is deprecated; use 'amqp.read_timeout' instead"
            );

            if (strcmp(DEFAULT_READ_TIMEOUT, INI_STR("amqp.read_timeout")) == 0) {
                zend_update_property_double(
                    this_ce,
                    PHP_AMQP_COMPAT_OBJ_P(getThis()),
                    ZEND_STRL("readTimeout"),
                    INI_FLT("amqp.timeout") TSRMLS_CC
                );
            } else {
                php_error_docref(
                    NULL TSRMLS_CC,
                    E_NOTICE,
                    "INI setting 'amqp.read_timeout' will be used instead of 'amqp.timeout'"
                );
                zend_update_property_double(
                    this_ce,
                    PHP_AMQP_COMPAT_OBJ_P(getThis()),
                    ZEND_STRL("readTimeout"),
                    INI_FLT("amqp.read_timeout") TSRMLS_CC
                );
            }
        } else {
            zend_update_property_double(
                this_ce,
                PHP_AMQP_COMPAT_OBJ_P(getThis()),
                ZEND_STRL("readTimeout"),
                INI_FLT("amqp.read_timeout") TSRMLS_CC
            );
        }
    }

    zend_update_property_double(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("writeTimeout"),
        INI_FLT("amqp.write_timeout") TSRMLS_CC
    );

    if (ini_arr && (zdata = zend_hash_str_find(HASH_OF(ini_arr), ZEND_STRL("write_timeout"))) != NULL) {
        SEPARATE_ZVAL(zdata);
        convert_to_double(zdata);

        if (!php_amqp_is_valid_timeout(Z_DVAL_P(zdata))) {
            zend_throw_exception(
                amqp_connection_exception_class_entry,
                "Parameter 'write_timeout' must be greater than or equal to zero.",
                0 TSRMLS_CC
            );
            return;
        }

        zend_update_property_double(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(getThis()),
            ZEND_STRL("writeTimeout"),
            Z_DVAL_P(zdata) TSRMLS_CC
        );
    }

    zend_update_property_double(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("rpcTimeout"),
        INI_FLT("amqp.rpc_timeout") TSRMLS_CC
    );

    if (ini_arr && (zdata = zend_hash_str_find(HASH_OF(ini_arr), ZEND_STRL("rpc_timeout"))) != NULL) {
        SEPARATE_ZVAL(zdata);
        convert_to_double(zdata);

        if (!php_amqp_is_valid_timeout(Z_DVAL_P(zdata))) {
            zend_throw_exception(
                amqp_connection_exception_class_entry,
                "Parameter 'rpc_timeout' must be greater than or equal to zero.",
                0 TSRMLS_CC
            );
            return;
        }

        zend_update_property_double(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(getThis()),
            ZEND_STRL("rpcTimeout"),
            Z_DVAL_P(zdata) TSRMLS_CC
        );
    }

    zend_update_property_double(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("connectTimeout"),
        INI_FLT("amqp.connect_timeout") TSRMLS_CC
    );

    if (ini_arr && (zdata = zend_hash_str_find(HASH_OF(ini_arr), ZEND_STRL("connect_timeout"))) != NULL) {
        SEPARATE_ZVAL(zdata);
        convert_to_double(zdata);

        if (!php_amqp_is_valid_timeout(Z_DVAL_P(zdata))) {
            zend_throw_exception(
                amqp_connection_exception_class_entry,
                "Parameter 'connect_timeout' must be greater than or equal to zero.",
                0 TSRMLS_CC
            );
            return;
        }

        zend_update_property_double(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(getThis()),
            ZEND_STRL("connectTimeout"),
            Z_DVAL_P(zdata) TSRMLS_CC
        );
    }

    zend_update_property_long(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("channelMax"),
        INI_INT("amqp.channel_max") TSRMLS_CC
    );

    if (ini_arr && (zdata = zend_hash_str_find(HASH_OF(ini_arr), ZEND_STRL("channel_max"))) != NULL) {
        SEPARATE_ZVAL(zdata);
        convert_to_long(zdata);

        if (!php_amqp_is_valid_channel_max(Z_LVAL_P(zdata))) {
            zend_throw_exception(
                amqp_connection_exception_class_entry,
                "Parameter 'channel_max' is out of range.",
                0 TSRMLS_CC
            );
            return;
        }

        if (Z_LVAL_P(zdata) == 0) {
            zend_update_property_long(
                this_ce,
                PHP_AMQP_COMPAT_OBJ_P(getThis()),
                ZEND_STRL("channelMax"),
                PHP_AMQP_DEFAULT_CHANNEL_MAX TSRMLS_CC
            );
        } else {
            zend_update_property_long(
                this_ce,
                PHP_AMQP_COMPAT_OBJ_P(getThis()),
                ZEND_STRL("channelMax"),
                Z_LVAL_P(zdata) TSRMLS_CC
            );
        }
    }

    zend_update_property_long(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("frameMax"),
        INI_INT("amqp.frame_max") TSRMLS_CC
    );

    if (ini_arr && (zdata = zend_hash_str_find(HASH_OF(ini_arr), ZEND_STRL("frame_max"))) != NULL) {
        SEPARATE_ZVAL(zdata);
        convert_to_long(zdata);
        if (!php_amqp_is_valid_frame_size_max(Z_LVAL_P(zdata))) {
            zend_throw_exception(
                amqp_connection_exception_class_entry,
                "Parameter 'frame_max' is out of range.",
                0 TSRMLS_CC
            );
            return;
        }

        if (Z_LVAL_P(zdata) == 0) {
            zend_update_property_long(
                this_ce,
                PHP_AMQP_COMPAT_OBJ_P(getThis()),
                ZEND_STRL("frameMax"),
                PHP_AMQP_DEFAULT_FRAME_MAX TSRMLS_CC
            );
        } else {
            zend_update_property_long(
                this_ce,
                PHP_AMQP_COMPAT_OBJ_P(getThis()),
                ZEND_STRL("frameMax"),
                Z_LVAL_P(zdata) TSRMLS_CC
            );
        }
    }

    zend_update_property_long(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("heartbeat"),
        INI_INT("amqp.heartbeat") TSRMLS_CC
    );

    if (ini_arr && (zdata = zend_hash_str_find(HASH_OF(ini_arr), ZEND_STRL("heartbeat"))) != NULL) {
        SEPARATE_ZVAL(zdata);
        convert_to_long(zdata);
        if (!php_amqp_is_valid_heartbeat(Z_LVAL_P(zdata))) {
            zend_throw_exception(
                amqp_connection_exception_class_entry,
                "Parameter 'heartbeat' is out of range.",
                0 TSRMLS_CC
            );
            return;
        }

        zend_update_property_long(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(getThis()),
            ZEND_STRL("heartbeat"),
            Z_LVAL_P(zdata) TSRMLS_CC
        );
    }

    zend_update_property_long(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("saslMethod"),
        INI_INT("amqp.sasl_method") TSRMLS_CC
    );

    if (ini_arr && (zdata = zend_hash_str_find(HASH_OF(ini_arr), ZEND_STRL("sasl_method"))) != NULL) {
        SEPARATE_ZVAL(zdata);
        convert_to_long(zdata);
        zend_update_property_long(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(getThis()),
            ZEND_STRL("saslMethod"),
            Z_LVAL_P(zdata) TSRMLS_CC
        );
    }


    PHP_AMQP_EXTRACT_CONNECTION_STR("cacert");
    PHP_AMQP_EXTRACT_CONNECTION_STR("key");
    PHP_AMQP_EXTRACT_CONNECTION_STR("cert");

    PHP_AMQP_EXTRACT_CONNECTION_BOOL("verify");

    /* Pull the connection_name out of the $params array */
    zdata = NULL;
    if (ini_arr && (zdata = zend_hash_str_find(HASH_OF(ini_arr), ZEND_STRL("connection_name"))) != NULL) {
        SEPARATE_ZVAL(zdata);
        convert_to_string(zdata);
    }
    if (zdata && Z_STRLEN_P(zdata) > 0) {
        zend_update_property_string(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(getThis()),
            ZEND_STRL("connectionName"),
            Z_STRVAL_P(zdata) TSRMLS_CC
        );
    }
}
/* }}} */


/* {{{ proto amqp::isConnected()
check amqp connection */
static PHP_METHOD(amqp_connection_class, isConnected)
{
    amqp_connection_object *connection;

    PHP_AMQP_NOPARAMS();

    /* Get the connection object out of the store */
    connection = PHP_AMQP_GET_CONNECTION(getThis());

    RETURN_BOOL(connection->connection_resource != NULL && connection->connection_resource->is_connected);
}
/* }}} */


/* {{{ proto amqp::connect()
create amqp connection */
static PHP_METHOD(amqp_connection_class, connect)
{
    amqp_connection_object *connection;

    PHP_AMQP_NOPARAMS();

    /* Get the connection object out of the store */
    connection = PHP_AMQP_GET_CONNECTION(getThis());

    if (connection->connection_resource && connection->connection_resource->is_connected) {
        if (connection->connection_resource->is_persistent) {
            php_error_docref(
                NULL TSRMLS_CC,
                E_WARNING,
                "Attempt to start transient connection while persistent transient one already established. Continue."
            );
        }

        RETURN_TRUE;
    }

    /* Actually connect this resource to the broker */
    php_amqp_connect(connection, 0, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */


/* {{{ proto amqp::connect()
create amqp connection */
static PHP_METHOD(amqp_connection_class, pconnect)
{
    amqp_connection_object *connection;

    PHP_AMQP_NOPARAMS();

    /* Get the connection object out of the store */
    connection = PHP_AMQP_GET_CONNECTION(getThis());

    if (connection->connection_resource && connection->connection_resource->is_connected) {

        assert(connection->connection_resource != NULL);
        if (!connection->connection_resource->is_persistent) {
            php_error_docref(
                NULL TSRMLS_CC,
                E_WARNING,
                "Attempt to start persistent connection while transient one already established. Continue."
            );
        }

        RETURN_TRUE;
    }

    /* Actually connect this resource to the broker or use stored connection */
    php_amqp_connect(connection, 1, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

#define PERSISTENT_TRANSIENT_EXCEPTION_MESSAGE                                                                         \
    "Attempted to %s a %s connection while a %s connection is established. Call '%s' instead"

/* {{{ proto amqp:pdisconnect()
destroy amqp persistent connection */
static PHP_METHOD(amqp_connection_class, pdisconnect)
{
    amqp_connection_object *connection;

    PHP_AMQP_NOPARAMS();

    /* Get the connection object out of the store */
    connection = PHP_AMQP_GET_CONNECTION(getThis());

    if (!connection->connection_resource || !connection->connection_resource->is_connected) {
        return;
    }

    assert(connection->connection_resource != NULL);

    if (!connection->connection_resource->is_persistent) {
        zend_throw_exception_ex(
            amqp_connection_exception_class_entry,
            0 TSRMLS_CC,
            PERSISTENT_TRANSIENT_EXCEPTION_MESSAGE,
            "close",
            "persistent",
            "transient",
            "disconnect"
        );
        return;
    }

    php_amqp_disconnect_force(connection->connection_resource TSRMLS_CC);
}
/* }}} */


/* {{{ proto amqp::disconnect()
destroy amqp connection */
static PHP_METHOD(amqp_connection_class, disconnect)
{
    amqp_connection_object *connection;

    PHP_AMQP_NOPARAMS();

    /* Get the connection object out of the store */
    connection = PHP_AMQP_GET_CONNECTION(getThis());

    if (!connection->connection_resource || !connection->connection_resource->is_connected) {
        return;
    }

    if (connection->connection_resource->is_persistent) {
        zend_throw_exception_ex(
            amqp_connection_exception_class_entry,
            0 TSRMLS_CC,
            PERSISTENT_TRANSIENT_EXCEPTION_MESSAGE,
            "close",
            "transient",
            "persistent",
            "pdisconnect"
        );
        return;
    }

    assert(connection->connection_resource != NULL);

    php_amqp_disconnect(connection->connection_resource TSRMLS_CC);
}

/* }}} */

/* {{{ proto amqp::reconnect()
recreate amqp connection */
static PHP_METHOD(amqp_connection_class, reconnect)
{
    amqp_connection_object *connection;

    PHP_AMQP_NOPARAMS();

    /* Get the connection object out of the store */
    connection = PHP_AMQP_GET_CONNECTION(getThis());

    if (connection->connection_resource && connection->connection_resource->is_connected) {

        assert(connection->connection_resource != NULL);

        if (connection->connection_resource->is_persistent) {
            zend_throw_exception_ex(
                amqp_connection_exception_class_entry,
                0 TSRMLS_CC,
                PERSISTENT_TRANSIENT_EXCEPTION_MESSAGE,
                "reconnect",
                "transient",
                "persistent",
                "preconnect"
            );
            return;
        }

        php_amqp_disconnect(connection->connection_resource TSRMLS_CC);
    }

    php_amqp_connect(connection, 0, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto amqp::preconnect()
recreate amqp connection */
static PHP_METHOD(amqp_connection_class, preconnect)
{
    amqp_connection_object *connection;

    PHP_AMQP_NOPARAMS();

    /* Get the connection object out of the store */
    connection = PHP_AMQP_GET_CONNECTION(getThis());


    if (connection->connection_resource && connection->connection_resource->is_connected) {

        assert(connection->connection_resource != NULL);

        if (!connection->connection_resource->is_persistent) {
            zend_throw_exception_ex(
                amqp_connection_exception_class_entry,
                0 TSRMLS_CC,
                PERSISTENT_TRANSIENT_EXCEPTION_MESSAGE,
                "reconnect",
                "persistent",
                "transient",
                "reconnect"
            );
            return;
        }

        php_amqp_disconnect_force(connection->connection_resource TSRMLS_CC);
    }

    php_amqp_connect(connection, 1, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */


/* {{{ proto amqp::getLogin()
get the login */
static PHP_METHOD(amqp_connection_class, getLogin)
{
    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("login");
}
/* }}} */


/* {{{ proto amqp::setLogin(string login)
set the login */
static PHP_METHOD(amqp_connection_class, setLogin)
{
    char *login = NULL;
    size_t login_len = 0;

    /* Get the login from the method params */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &login, &login_len) == FAILURE) {
        return;
    }

    /* Validate login length */
    if (login_len > PHP_AMQP_MAX_CREDENTIALS_LENGTH) {
        zend_throw_exception_ex(
            amqp_connection_exception_class_entry,
            0 TSRMLS_CC,
            "Parameter 'login' exceeds %d character limit.",
            PHP_AMQP_MAX_CREDENTIALS_LENGTH
        );
        return;
    }

    zend_update_property_stringl(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("login"),
        login,
        login_len TSRMLS_CC
    );
}
/* }}} */

/* {{{ proto amqp::getPassword()
get the password */
static PHP_METHOD(amqp_connection_class, getPassword)
{
    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("password");
}
/* }}} */


/* {{{ proto amqp::setPassword(string password)
set the password */
static PHP_METHOD(amqp_connection_class, setPassword)
{
    char *password = NULL;
    size_t password_len = 0;

    /* Get the password from the method params */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &password, &password_len) == FAILURE) {
        return;
    }

    /* Validate password length */
    if (password_len > PHP_AMQP_MAX_CREDENTIALS_LENGTH) {
        zend_throw_exception_ex(
            amqp_connection_exception_class_entry,
            0 TSRMLS_CC,
            "Parameter 'password' exceeds %d character limit.",
            PHP_AMQP_MAX_CREDENTIALS_LENGTH
        );
        return;
    }

    zend_update_property_stringl(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("password"),
        password,
        password_len TSRMLS_CC
    );
}
/* }}} */


/* {{{ proto amqp::getHost()
get the host */
static PHP_METHOD(amqp_connection_class, getHost)
{
    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("host");
}
/* }}} */


/* {{{ proto amqp::setHost(string host)
set the host */
static PHP_METHOD(amqp_connection_class, setHost)
{
    char *host = NULL;
    size_t host_len = 0;

    /* Get the host from the method params */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &host, &host_len) == FAILURE) {
        return;
    }

    /* Validate host length */
    if (host_len > PHP_AMQP_MAX_IDENTIFIER_LENGTH) {
        zend_throw_exception_ex(
            amqp_connection_exception_class_entry,
            0 TSRMLS_CC,
            "Parameter 'host' exceeds %d character limit.",
            PHP_AMQP_MAX_IDENTIFIER_LENGTH
        );
        return;
    }

    zend_update_property_stringl(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("host"),
        host,
        host_len TSRMLS_CC
    );
}
/* }}} */


/* {{{ proto amqp::getPort()
get the port */
static PHP_METHOD(amqp_connection_class, getPort)
{
    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("port");
}
/* }}} */


/* {{{ proto amqp::setPort(mixed port)
set the port */
static PHP_METHOD(amqp_connection_class, setPort)
{
    zval *zvalPort;
    int port;

    /* Get the port from the method params */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z/", &zvalPort) == FAILURE) {
        return;
    }

    /* Parse out the port*/
    switch (Z_TYPE_P(zvalPort)) {
        case IS_DOUBLE:
            port = (int) Z_DVAL_P(zvalPort);
            break;
        case IS_LONG:
            port = (int) Z_LVAL_P(zvalPort);
            break;
        case IS_STRING:
            convert_to_long(zvalPort);
            port = (int) Z_LVAL_P(zvalPort);
            break;
        default:
            port = 0;
    }

    /* Check the port value */
    if (port <= 0 || port > 65535) {
        zend_throw_exception(
            amqp_connection_exception_class_entry,
            "Invalid port given. Value must be between 1 and 65535.",
            0 TSRMLS_CC
        );
        return;
    }

    zend_update_property_long(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("port"), port TSRMLS_CC);
}
/* }}} */

/* {{{ proto amqp::getVhost()
get the vhost */
static PHP_METHOD(amqp_connection_class, getVhost)
{
    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("vhost");
}
/* }}} */


/* {{{ proto amqp::setVhost(string vhost)
set the vhost */
static PHP_METHOD(amqp_connection_class, setVhost)
{
    char *vhost = NULL;
    size_t vhost_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &vhost, &vhost_len) == FAILURE) {
        return;
    }

    /* Validate vhost length */
    if (vhost_len > PHP_AMQP_MAX_IDENTIFIER_LENGTH) {
        zend_throw_exception_ex(
            amqp_connection_exception_class_entry,
            0 TSRMLS_CC,
            "Parameter 'vhost' exceeds %d characters limit.",
            PHP_AMQP_MAX_IDENTIFIER_LENGTH
        );
        return;
    }

    zend_update_property_stringl(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("vhost"),
        vhost,
        vhost_len TSRMLS_CC
    );
}
/* }}} */

/* {{{ proto amqp::getTimeout()
@deprecated
get the timeout */
static PHP_METHOD(amqp_connection_class, getTimeout)
{
    php_error_docref(
        NULL TSRMLS_CC,
        E_DEPRECATED,
        "AMQPConnection::getTimeout() method is deprecated; use AMQPConnection::getReadTimeout() instead"
    );

    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("readTimeout");
}
/* }}} */

/* {{{ proto amqp::setTimeout(double timeout)
@deprecated
set the timeout */
static PHP_METHOD(amqp_connection_class, setTimeout)
{
    amqp_connection_object *connection;
    double read_timeout;

    php_error_docref(
        NULL TSRMLS_CC,
        E_DEPRECATED,
        "AMQPConnection::setTimeout($timeout) method is deprecated; use AMQPConnection::setReadTimeout($timeout) "
        "instead"
    );

    /* Get the timeout from the method params */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d", &read_timeout) == FAILURE) {
        return;
    }

    /* Validate timeout */
    if (!php_amqp_is_valid_timeout(read_timeout)) {
        zend_throw_exception(
            amqp_connection_exception_class_entry,
            "Parameter 'timeout' must be greater than or equal to zero.",
            0 TSRMLS_CC
        );
        return;
    }

    /* Get the connection object out of the store */
    connection = PHP_AMQP_GET_CONNECTION(getThis());

    zend_update_property_double(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("readTimeout"),
        read_timeout TSRMLS_CC
    );

    if (connection->connection_resource && connection->connection_resource->is_connected) {
        if (php_amqp_set_resource_read_timeout(connection->connection_resource, read_timeout TSRMLS_CC) == 0) {

            php_amqp_disconnect_force(connection->connection_resource TSRMLS_CC);

            zend_throw_exception(amqp_connection_exception_class_entry, "Could not set read timeout", 0 TSRMLS_CC);
        }
    }
}
/* }}} */

/* {{{ proto amqp::getReadTimeout()
get the read timeout */
static PHP_METHOD(amqp_connection_class, getReadTimeout)
{
    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("readTimeout");
}
/* }}} */

/* {{{ proto amqp::setReadTimeout(double timeout)
set read timeout */
static PHP_METHOD(amqp_connection_class, setReadTimeout)
{
    amqp_connection_object *connection;
    double read_timeout;

    /* Get the timeout from the method params */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d", &read_timeout) == FAILURE) {
        return;
    }

    /* Validate timeout */
    if (!php_amqp_is_valid_timeout(read_timeout)) {
        zend_throw_exception(
            amqp_connection_exception_class_entry,
            "Parameter 'readTimeout' must be greater than or equal to zero.",
            0 TSRMLS_CC
        );
        return;
    }

    /* Get the connection object out of the store */
    connection = PHP_AMQP_GET_CONNECTION(getThis());

    zend_update_property_double(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("readTimeout"),
        read_timeout TSRMLS_CC
    );

    if (connection->connection_resource && connection->connection_resource->is_connected) {
        if (php_amqp_set_resource_read_timeout(connection->connection_resource, read_timeout TSRMLS_CC) == 0) {

            php_amqp_disconnect_force(connection->connection_resource TSRMLS_CC);

            zend_throw_exception(amqp_connection_exception_class_entry, "Could not set read timeout", 0 TSRMLS_CC);
        }
    }
}
/* }}} */

/* {{{ proto amqp::getWriteTimeout()
get write timeout */
static PHP_METHOD(amqp_connection_class, getWriteTimeout)
{
    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("writeTimeout");
}
/* }}} */

/* {{{ proto amqp::setWriteTimeout(double timeout)
set write timeout */
static PHP_METHOD(amqp_connection_class, setWriteTimeout)
{
    amqp_connection_object *connection;
    double write_timeout;

    /* Get the timeout from the method params */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d", &write_timeout) == FAILURE) {
        return;
    }

    /* Validate timeout */
    if (!php_amqp_is_valid_timeout(write_timeout)) {
        zend_throw_exception(
            amqp_connection_exception_class_entry,
            "Parameter 'writeTimeout' must be greater than or equal to zero.",
            0 TSRMLS_CC
        );
        return;
    }

    /* Get the connection object out of the store */
    connection = PHP_AMQP_GET_CONNECTION(getThis());

    zend_update_property_double(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("writeTimeout"),
        write_timeout TSRMLS_CC
    );

    if (connection->connection_resource && connection->connection_resource->is_connected) {
        if (php_amqp_set_resource_write_timeout(connection->connection_resource, write_timeout TSRMLS_CC) == 0) {

            php_amqp_disconnect_force(connection->connection_resource TSRMLS_CC);

            zend_throw_exception(amqp_connection_exception_class_entry, "Could not set write timeout", 0 TSRMLS_CC);
        }
    }
}
/* }}} */

/* {{{ proto amqp::getRpcTimeout()
get rpc timeout */
static PHP_METHOD(amqp_connection_class, getConnectTimeout)
{
    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("connectTimeout");
}
/* }}} */

/* {{{ proto amqp::getRpcTimeout()
get rpc timeout */
static PHP_METHOD(amqp_connection_class, getRpcTimeout)
{
    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("rpcTimeout");
}
/* }}} */

/* {{{ proto amqp::setRpcTimeout(double timeout)
set rpc timeout */
static PHP_METHOD(amqp_connection_class, setRpcTimeout)
{
    amqp_connection_object *connection;
    double rpc_timeout;

    /* Get the timeout from the method params */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d", &rpc_timeout) == FAILURE) {
        return;
    }

    /* Validate timeout */
    if (!php_amqp_is_valid_timeout(rpc_timeout)) {
        zend_throw_exception(
            amqp_connection_exception_class_entry,
            "Parameter 'rpcTimeout' must be greater than or equal to zero.",
            0 TSRMLS_CC
        );
        return;
    }

    /* Get the connection object out of the store */
    connection = PHP_AMQP_GET_CONNECTION(getThis());

    zend_update_property_double(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("rpcTimeout"),
        rpc_timeout TSRMLS_CC
    );

    if (connection->connection_resource && connection->connection_resource->is_connected) {
        if (php_amqp_set_resource_rpc_timeout(connection->connection_resource, rpc_timeout TSRMLS_CC) == 0) {

            php_amqp_disconnect_force(connection->connection_resource TSRMLS_CC);

            zend_throw_exception(amqp_connection_exception_class_entry, "Could not set connect timeout", 0 TSRMLS_CC);
        }
    }
}
/* }}} */

/* {{{ proto amqp::getUsedChannels()
Get max used channels number */
static PHP_METHOD(amqp_connection_class, getUsedChannels)
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
    zval rv;
    amqp_connection_object *connection;

    PHP_AMQP_NOPARAMS();

    /* Get the connection object out of the store */
    connection = PHP_AMQP_GET_CONNECTION(getThis());

    if (connection->connection_resource && connection->connection_resource->is_connected) {
        RETURN_LONG(connection->connection_resource->max_slots);
    }

    PHP_AMQP_RETURN_THIS_PROP("channelMax");
}
/* }}} */

/* {{{ proto amqp::getMaxFrameSize()
Get max supported frame size per connection in bytes */
static PHP_METHOD(amqp_connection_class, getMaxFrameSize)
{
    zval rv;
    amqp_connection_object *connection;

    PHP_AMQP_NOPARAMS();

    /* Get the connection object out of the store */
    connection = PHP_AMQP_GET_CONNECTION(getThis());

    if (connection->connection_resource && connection->connection_resource->is_connected) {
        RETURN_LONG(amqp_get_frame_max(connection->connection_resource->connection_state));
    }

    PHP_AMQP_RETURN_THIS_PROP("frameMax");
}
/* }}} */

/* {{{ proto amqp::getHeartbeatInterval()
Get number of seconds between heartbeats of the connection in seconds */
static PHP_METHOD(amqp_connection_class, getHeartbeatInterval)
{
    zval rv;
    amqp_connection_object *connection;

    PHP_AMQP_NOPARAMS();

    /* Get the connection object out of the store */
    connection = PHP_AMQP_GET_CONNECTION(getThis());

    if (connection->connection_resource != NULL && connection->connection_resource->is_connected != '\0') {
        RETURN_LONG(amqp_get_heartbeat(connection->connection_resource->connection_state));
    }

    PHP_AMQP_RETURN_THIS_PROP("heartbeat");
}
/* }}} */

/* {{{ proto amqp::isPersistent()
check whether amqp connection is persistent */
static PHP_METHOD(amqp_connection_class, isPersistent)
{
    amqp_connection_object *connection;

    PHP_AMQP_NOPARAMS();

    connection = PHP_AMQP_GET_CONNECTION(getThis());

    RETURN_BOOL(connection->connection_resource && connection->connection_resource->is_persistent);
}
/* }}} */


/* {{{ proto amqp::getCACert() */
static PHP_METHOD(amqp_connection_class, getCACert)
{
    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("cacert");
}
/* }}} */

/* {{{ proto amqp::setCACert(string cacert) */
static PHP_METHOD(amqp_connection_class, setCACert)
{
    char *str = NULL;
    size_t str_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &str, &str_len) == FAILURE) {
        return;
    }

    zend_update_property_stringl(
        this_ce,
        PHP_AMQP_COMPAT_OBJ_P(getThis()),
        ZEND_STRL("cacert"),
        str,
        str_len TSRMLS_CC
    );
}
/* }}} */

/* {{{ proto amqp::getCert() */
static PHP_METHOD(amqp_connection_class, getCert)
{
    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("cert");
}
/* }}} */

/* {{{ proto amqp::setCert(string cert) */
static PHP_METHOD(amqp_connection_class, setCert)
{
    char *str = NULL;
    size_t str_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &str, &str_len) == FAILURE) {
        return;
    }

    zend_update_property_stringl(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("cert"), str, str_len TSRMLS_CC);
}
/* }}} */

/* {{{ proto amqp::getKey() */
static PHP_METHOD(amqp_connection_class, getKey)
{
    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("key");
}
/* }}} */

/* {{{ proto amqp::setKey(string key) */
static PHP_METHOD(amqp_connection_class, setKey)
{
    char *str = NULL;
    size_t str_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &str, &str_len) == FAILURE) {
        return;
    }

    zend_update_property_stringl(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("key"), str, str_len TSRMLS_CC);
}
/* }}} */


/* {{{ proto amqp::getVerify() */
static PHP_METHOD(amqp_connection_class, getVerify)
{
    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("verify");
}
/* }}} */

/* {{{ proto amqp::setVerify(bool verify) */
static PHP_METHOD(amqp_connection_class, setVerify)
{
    zend_bool verify = 1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &verify) == FAILURE) {
        return;
    }

    zend_update_property_bool(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("verify"), verify TSRMLS_CC);
}
/* }}} */

/* {{{ proto amqp::getSaslMethod()
get sasl method */
static PHP_METHOD(amqp_connection_class, getSaslMethod)
{
    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("saslMethod");
}
/* }}} */

/* {{{ proto amqp::setSaslMethod(mixed method)
set sasl method */
static PHP_METHOD(amqp_connection_class, setSaslMethod)
{
    long method;

    /* Get the port from the method params */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &method) == FAILURE) {
        return;
    }

    /* Check the method value */
    if (method != AMQP_SASL_METHOD_PLAIN && method != AMQP_SASL_METHOD_EXTERNAL) {
        zend_throw_exception(
            amqp_connection_exception_class_entry,
            "Invalid SASL method given. Method must be AMQP_SASL_METHOD_PLAIN or AMQP_SASL_METHOD_EXTERNAL.",
            0 TSRMLS_CC
        );
        return;
    }

    zend_update_property_long(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("saslMethod"), method TSRMLS_CC);
}
/* }}} */

/* {{{ proto amqp::getConnectionName() */
static PHP_METHOD(amqp_connection_class, getConnectionName)
{
    zval rv;
    PHP_AMQP_NOPARAMS();
    PHP_AMQP_RETURN_THIS_PROP("connectionName");
}
/* }}} */

/* {{{ proto amqp::setConnectionName(string connectionName) */
static PHP_METHOD(amqp_connection_class, setConnectionName)
{
    char *str = NULL;
    size_t str_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s!", &str, &str_len) == FAILURE) {
        return;
    }
    if (str == NULL) {
        zend_update_property_null(this_ce, PHP_AMQP_COMPAT_OBJ_P(getThis()), ZEND_STRL("connectionName") TSRMLS_CC);
    } else {
        zend_update_property_stringl(
            this_ce,
            PHP_AMQP_COMPAT_OBJ_P(getThis()),
            ZEND_STRL("connectionName"),
            str,
            str_len TSRMLS_CC
        );
    }
}
/* }}} */

/* amqp_connection_class ARG_INFO definition */
ZEND_BEGIN_ARG_INFO_EX(arginfo_amqp_connection_class__construct, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, credentials, IS_ARRAY, 0, "[]")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_isConnected, ZEND_SEND_BY_VAL, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_connect, ZEND_SEND_BY_VAL, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_pconnect, ZEND_SEND_BY_VAL, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_pdisconnect, ZEND_SEND_BY_VAL, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_disconnect, ZEND_SEND_BY_VAL, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_reconnect, ZEND_SEND_BY_VAL, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_preconnect, ZEND_SEND_BY_VAL, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_getLogin, ZEND_SEND_BY_VAL, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_setLogin, ZEND_SEND_BY_VAL, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, login, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_getPassword, ZEND_SEND_BY_VAL, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_setPassword, ZEND_SEND_BY_VAL, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, password, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_getHost, ZEND_SEND_BY_VAL, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_setHost, ZEND_SEND_BY_VAL, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, host, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_getPort, ZEND_SEND_BY_VAL, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_setPort, ZEND_SEND_BY_VAL, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_getVhost, ZEND_SEND_BY_VAL, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_setVhost, ZEND_SEND_BY_VAL, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, vhost, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_getTimeout, ZEND_SEND_BY_VAL, 0, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_setTimeout, ZEND_SEND_BY_VAL, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, timeout, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_getReadTimeout, ZEND_SEND_BY_VAL, 0, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_setReadTimeout, ZEND_SEND_BY_VAL, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, timeout, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(
    arginfo_amqp_connection_class_getWriteTimeout,
    ZEND_SEND_BY_VAL,
    0,
    IS_DOUBLE,
    0
)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_setWriteTimeout, ZEND_SEND_BY_VAL, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, timeout, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(
    arginfo_amqp_connection_class_getConnectTimeout,
    ZEND_SEND_BY_VAL,
    0,
    IS_DOUBLE,
    0
)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_getRpcTimeout, ZEND_SEND_BY_VAL, 0, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_setRpcTimeout, ZEND_SEND_BY_VAL, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, timeout, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_getUsedChannels, ZEND_SEND_BY_VAL, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_getMaxChannels, ZEND_SEND_BY_VAL, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_getMaxFrameSize, ZEND_SEND_BY_VAL, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(
    arginfo_amqp_connection_class_getHeartbeatInterval,
    ZEND_SEND_BY_VAL,
    0,
    IS_LONG,
    0
)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_isPersistent, ZEND_SEND_BY_VAL, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_getCACert, ZEND_SEND_BY_VAL, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_setCACert, ZEND_SEND_BY_VAL, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, cacert, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_getCert, ZEND_SEND_BY_VAL, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_setCert, ZEND_SEND_BY_VAL, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, cert, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_getKey, ZEND_SEND_BY_VAL, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_setKey, ZEND_SEND_BY_VAL, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_getVerify, ZEND_SEND_BY_VAL, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_setVerify, ZEND_SEND_BY_VAL, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, verify, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_getSaslMethod, ZEND_SEND_BY_VAL, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_amqp_connection_class_setSaslMethod, ZEND_SEND_BY_VAL, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, saslMethod, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(
    arginfo_amqp_connection_class_getConnectionName,
    ZEND_SEND_BY_VAL,
    0,
    IS_STRING,
    1
)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(
    arginfo_amqp_connection_class_setConnectionName,
    ZEND_SEND_BY_VAL,
    1,
    IS_VOID,
    0
)
    ZEND_ARG_TYPE_INFO(0, connectionName, IS_STRING, 1)
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

    PHP_ME(amqp_connection_class, getConnectTimeout, 	arginfo_amqp_connection_class_getConnectTimeout,	ZEND_ACC_PUBLIC)
  /** setConnectTimeout intentionally left out */

    PHP_ME(amqp_connection_class, getRpcTimeout, 	arginfo_amqp_connection_class_getRpcTimeout,	ZEND_ACC_PUBLIC)
    PHP_ME(amqp_connection_class, setRpcTimeout, 	arginfo_amqp_connection_class_setRpcTimeout,	ZEND_ACC_PUBLIC)

    PHP_ME(amqp_connection_class, getUsedChannels, arginfo_amqp_connection_class_getUsedChannels,	ZEND_ACC_PUBLIC)
    PHP_ME(amqp_connection_class, getMaxChannels,  arginfo_amqp_connection_class_getMaxChannels,	ZEND_ACC_PUBLIC)
    PHP_ME(amqp_connection_class, isPersistent, 	arginfo_amqp_connection_class_isPersistent,		ZEND_ACC_PUBLIC)
    PHP_ME(amqp_connection_class, getHeartbeatInterval,  arginfo_amqp_connection_class_getHeartbeatInterval,	ZEND_ACC_PUBLIC)
    PHP_ME(amqp_connection_class, getMaxFrameSize,  arginfo_amqp_connection_class_getMaxFrameSize,	ZEND_ACC_PUBLIC)

    PHP_ME(amqp_connection_class, getCACert, 	arginfo_amqp_connection_class_getCACert,		ZEND_ACC_PUBLIC)
    PHP_ME(amqp_connection_class, setCACert, 	arginfo_amqp_connection_class_setCACert,		ZEND_ACC_PUBLIC)

    PHP_ME(amqp_connection_class, getCert, 	arginfo_amqp_connection_class_getCert,		ZEND_ACC_PUBLIC)
    PHP_ME(amqp_connection_class, setCert, 	arginfo_amqp_connection_class_setCert,		ZEND_ACC_PUBLIC)

    PHP_ME(amqp_connection_class, getKey, 	arginfo_amqp_connection_class_getKey,		ZEND_ACC_PUBLIC)
    PHP_ME(amqp_connection_class, setKey, 	arginfo_amqp_connection_class_setKey,		ZEND_ACC_PUBLIC)

    PHP_ME(amqp_connection_class, getVerify, 	arginfo_amqp_connection_class_getVerify,		ZEND_ACC_PUBLIC)
    PHP_ME(amqp_connection_class, setVerify, 	arginfo_amqp_connection_class_setVerify,		ZEND_ACC_PUBLIC)

    PHP_ME(amqp_connection_class, getSaslMethod, 	arginfo_amqp_connection_class_getSaslMethod,		ZEND_ACC_PUBLIC)
    PHP_ME(amqp_connection_class, setSaslMethod, 	arginfo_amqp_connection_class_setSaslMethod,		ZEND_ACC_PUBLIC)

    PHP_ME(amqp_connection_class, getConnectionName, 	arginfo_amqp_connection_class_getConnectionName,		ZEND_ACC_PUBLIC)
    PHP_ME(amqp_connection_class, setConnectionName, 	arginfo_amqp_connection_class_setConnectionName,		ZEND_ACC_PUBLIC)

    {NULL, NULL, NULL}
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

    zend_declare_property_null(this_ce, ZEND_STRL("readTimeout"), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(this_ce, ZEND_STRL("writeTimeout"), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(this_ce, ZEND_STRL("connectTimeout"), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(this_ce, ZEND_STRL("rpcTimeout"), ZEND_ACC_PRIVATE TSRMLS_CC);

    zend_declare_property_null(this_ce, ZEND_STRL("frameMax"), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(this_ce, ZEND_STRL("channelMax"), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(this_ce, ZEND_STRL("heartbeat"), ZEND_ACC_PRIVATE TSRMLS_CC);

    zend_declare_property_null(this_ce, ZEND_STRL("cacert"), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(this_ce, ZEND_STRL("key"), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(this_ce, ZEND_STRL("cert"), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(this_ce, ZEND_STRL("verify"), ZEND_ACC_PRIVATE TSRMLS_CC);

    zend_declare_property_null(this_ce, ZEND_STRL("saslMethod"), ZEND_ACC_PRIVATE TSRMLS_CC);

    zend_declare_property_null(this_ce, ZEND_STRL("connectionName"), ZEND_ACC_PRIVATE TSRMLS_CC);

    memcpy(&amqp_connection_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

    amqp_connection_object_handlers.offset = XtOffsetOf(amqp_connection_object, zo);
    amqp_connection_object_handlers.free_obj = amqp_connection_free;

    return SUCCESS;
}
