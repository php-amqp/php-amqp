/*
 * This file is part of the pdezwart/php-amqp PHP extension.
 *
 * Copyright (c) php-amqp contributors
 *
 * Licensed under the MIT license: http://opensource.org/licenses/MIT
 *
 * For the full copyright and license information, please view the
 * LICENSE file that was distributed with this source or visit
 * http://opensource.org/licenses/MIT
 */

#ifndef PHP_AMQP_CONNECTION_RESOURCE_H
#define PHP_AMQP_CONNECTION_RESOURCE_H

#define PHP_AMQP_RESOURCE_RESPONSE_BREAK                    1
#define PHP_AMQP_RESOURCE_RESPONSE_OK                       0
#define PHP_AMQP_RESOURCE_RESPONSE_ERROR                   -1
#define PHP_AMQP_RESOURCE_RESPONSE_ERROR_CHANNEL_CLOSED    -2
#define PHP_AMQP_RESOURCE_RESPONSE_ERROR_CONNECTION_CLOSED -3

extern int le_amqp_connection_resource;
extern int le_amqp_connection_resource_persistent;

#include "php_amqp.h"
#include "amqp.h"

void php_amqp_prepare_for_disconnect(amqp_connection_resource *resource TSRMLS_DC);

typedef struct _amqp_connection_params {
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
  char *cacert;
  char *cert;
  char *key;
  int verify;
} amqp_connection_params;

/* Figure out what's going on connection and handle protocol exceptions, if any */
int php_amqp_connection_resource_error(amqp_rpc_reply_t reply, char **message, amqp_connection_resource *resource, amqp_channel_t channel_id TSRMLS_DC);
int php_amqp_connection_resource_error_advanced(amqp_rpc_reply_t reply, char **message, amqp_connection_resource *resource, amqp_channel_t channel_id, amqp_channel_object *channel TSRMLS_DC);

/* Socket-related functions */
int php_amqp_set_resource_read_timeout(amqp_connection_resource *resource, double read_timeout TSRMLS_DC);
int php_amqp_set_resource_write_timeout(amqp_connection_resource *resource, double write_timeout TSRMLS_DC);

/* Channel-related functions */
amqp_channel_t php_amqp_connection_resource_get_available_channel_id(amqp_connection_resource *resource);
int php_amqp_connection_resource_unregister_channel(amqp_connection_resource *resource, amqp_channel_t channel_id);
int php_amqp_connection_resource_register_channel(amqp_connection_resource *resource, amqp_channel_resource *channel_resource, amqp_channel_t channel_id);

/* Creating and destroying resource */
amqp_connection_resource *connection_resource_constructor(amqp_connection_params *params, zend_bool persistent TSRMLS_DC);
ZEND_RSRC_DTOR_FUNC(amqp_connection_resource_dtor_persistent);
ZEND_RSRC_DTOR_FUNC(amqp_connection_resource_dtor);

#endif
/*
*Local variables:
*tab-width: 4
*c-basic-offset: 4
*End:
*vim600: noet sw=4 ts=4 fdm=marker
*vim<600: noet sw=4 ts=4
*/
