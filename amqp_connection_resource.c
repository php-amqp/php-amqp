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
#include "ext/standard/datetime.h"
#include "zend_exceptions.h"

#ifdef PHP_WIN32
# include "win32/php_stdint.h"
# include "win32/signal.h"
#else
# include <signal.h>
# include <stdint.h>
#endif

#include <amqp.h>
#include <amqp_tcp_socket.h>
#include <amqp_framing.h>
#include <amqp_ssl_socket.h>

#ifdef PHP_WIN32
# include "win32/unistd.h"
#else
# include <unistd.h>
#endif

//#include "amqp_basic_properties.h"
#include "amqp_methods_handling.h"
#include "amqp_connection_resource.h"
#include "amqp_channel.h"
#include "php_amqp.h"

#ifndef E_DEPRECATED
#define E_DEPRECATED E_WARNING
#endif

int le_amqp_connection_resource;
int le_amqp_connection_resource_persistent;

static void connection_resource_destructor(amqp_connection_resource *resource, int persistent TSRMLS_DC);
static void php_amqp_close_connection_from_server(amqp_rpc_reply_t reply, char **message, amqp_connection_resource *resource TSRMLS_DC);
static void php_amqp_close_channel_from_server(amqp_rpc_reply_t reply, char **message, amqp_connection_resource *resource, amqp_channel_t channel_id TSRMLS_DC);


/* Figure out what's going on connection and handle protocol exceptions, if any */
int php_amqp_connection_resource_error(amqp_rpc_reply_t reply, char **message, amqp_connection_resource *resource, amqp_channel_t channel_id TSRMLS_DC)
{
	assert (resource != NULL);

	switch (reply.reply_type) {
		case AMQP_RESPONSE_NORMAL:
			return PHP_AMQP_RESOURCE_RESPONSE_OK;

		case AMQP_RESPONSE_NONE:
			spprintf(message, 0, "Missing RPC reply type.");
			return PHP_AMQP_RESOURCE_RESPONSE_ERROR;

		case AMQP_RESPONSE_LIBRARY_EXCEPTION:

			spprintf(message, 0, "Library error: %s", amqp_error_string2(reply.library_error));
			return PHP_AMQP_RESOURCE_RESPONSE_ERROR;

		case AMQP_RESPONSE_SERVER_EXCEPTION:
			switch (reply.reply.id) {
				case AMQP_CONNECTION_CLOSE_METHOD: {
					php_amqp_close_connection_from_server(reply, message, resource TSRMLS_CC);
					return PHP_AMQP_RESOURCE_RESPONSE_ERROR_CONNECTION_CLOSED;
				}
				case AMQP_CHANNEL_CLOSE_METHOD: {
					php_amqp_close_channel_from_server(reply, message, resource, channel_id TSRMLS_CC);
					return PHP_AMQP_RESOURCE_RESPONSE_ERROR_CHANNEL_CLOSED;
				}
			}
		/* Default for the above switch should be handled by the below default. */
		default:
			spprintf(message, 0, "Unknown server error, method id 0x%08X",	reply.reply.id);
			return PHP_AMQP_RESOURCE_RESPONSE_ERROR;
	}

	/* Should not never get here*/
}

static void php_amqp_close_connection_from_server(amqp_rpc_reply_t reply, char **message, amqp_connection_resource *resource TSRMLS_DC) {
	amqp_connection_close_t *m = (amqp_connection_close_t *)reply.reply.decoded;
	int result;

	if (!reply.reply.id) {
		PHP_AMQP_G(error_code) = -1;
		spprintf(message, 0, "Server connection error: %ld, message: %s",
				 (long)PHP_AMQP_G(error_code),
				 "unexpected response"
		);
	} else {
		PHP_AMQP_G(error_code) = m->reply_code;
		spprintf(message, 0, "Server connection error: %d, message: %.*s",
				 m->reply_code,
				 (int) m->reply_text.len,
				 (char *) m->reply_text.bytes
		);
	}
		
	/*
	 *    - If r.reply.id == AMQP_CONNECTION_CLOSE_METHOD a connection exception
	 *      occurred, cast r.reply.decoded to amqp_connection_close_t* to see
	 *      details of the exception. The client amqp_send_method() a
	 *      amqp_connection_close_ok_t and disconnect from the broker.
	 */

	amqp_connection_close_ok_t *decoded = (amqp_connection_close_ok_t *) NULL;

	result = amqp_send_method(
		resource->connection_state,
		0, /* NOTE: 0-channel is reserved for things like this */
		AMQP_CONNECTION_CLOSE_OK_METHOD,
		&decoded
	);

	if (result != AMQP_STATUS_OK) {
		zend_throw_exception(amqp_channel_exception_class_entry, "An error occurred while closing the connection.", 0 TSRMLS_CC);
	}

	/* Prevent finishing AMQP connection in connection resource destructor */
	resource->is_connected = '\0';
}

static void php_amqp_close_channel_from_server(amqp_rpc_reply_t reply, char **message, amqp_connection_resource *resource, amqp_channel_t channel_id TSRMLS_DC) {
	assert(channel_id > 0 && channel_id <= resource->max_slots);

	amqp_channel_close_t *m = (amqp_channel_close_t *) reply.reply.decoded;

	if (!reply.reply.id) {
		PHP_AMQP_G(error_code) = -1;
		spprintf(message, 0, "Server channel error: %ld, message: %s",
				 (long)PHP_AMQP_G(error_code),
				 "unexpected response"
		);
	} else {
		PHP_AMQP_G(error_code) = m->reply_code;
		spprintf(message, 0, "Server channel error: %d, message: %.*s",
			m->reply_code,
			(int) m->reply_text.len,
			(char *)m->reply_text.bytes
		);
	}

	/*
	 *    - If r.reply.id == AMQP_CHANNEL_CLOSE_METHOD a channel exception
	 *      occurred, cast r.reply.decoded to amqp_channel_close_t* to see details
	 *      of the exception. The client should amqp_send_method() a
	 *      amqp_channel_close_ok_t. The channel must be re-opened before it
	 *      can be used again. Any resources associated with the channel
	 *      (auto-delete exchanges, auto-delete queues, consumers) are invalid
	 *      and must be recreated before attempting to use them again.
	 */

	if (resource) {
		int result;
		amqp_channel_close_ok_t *decoded = (amqp_channel_close_ok_t *) NULL;

		result = amqp_send_method(
				resource->connection_state,
				channel_id,
				AMQP_CHANNEL_CLOSE_OK_METHOD,
				&decoded
		);
		if (result != AMQP_STATUS_OK) {
			zend_throw_exception(amqp_channel_exception_class_entry, "An error occurred while closing channel.", 0 TSRMLS_CC);
		}
	}
}


int php_amqp_connection_resource_error_advanced(amqp_rpc_reply_t reply, char **message, amqp_connection_resource *resource, amqp_channel_t channel_id, amqp_channel_object *channel TSRMLS_DC)
{
	assert(resource != NULL);

	amqp_frame_t frame;

	assert(AMQP_RESPONSE_LIBRARY_EXCEPTION == reply.reply_type);
	assert(AMQP_STATUS_UNEXPECTED_STATE == reply.library_error);

	if (channel_id < 0 || AMQP_STATUS_OK != amqp_simple_wait_frame(resource->connection_state, &frame)) {
		if (*message != NULL) {
			efree(*message);
		}

		spprintf(message, 0, "Library error: %s", amqp_error_string2(reply.library_error));
		return PHP_AMQP_RESOURCE_RESPONSE_ERROR;
	}

	if (channel_id != frame.channel) {
		spprintf(message, 0, "Library error: channel mismatch");
		return PHP_AMQP_RESOURCE_RESPONSE_ERROR;
	}

	if (AMQP_FRAME_METHOD == frame.frame_type) {
		switch (frame.payload.method.id) {
			case AMQP_CONNECTION_CLOSE_METHOD: {
				php_amqp_close_connection_from_server(reply, message, resource TSRMLS_CC);
				return PHP_AMQP_RESOURCE_RESPONSE_ERROR_CONNECTION_CLOSED;
			}
			case AMQP_CHANNEL_CLOSE_METHOD: {
				php_amqp_close_channel_from_server(reply, message, resource, channel_id TSRMLS_CC);
				return PHP_AMQP_RESOURCE_RESPONSE_ERROR_CHANNEL_CLOSED;
			}

			case AMQP_BASIC_ACK_METHOD:
				/* if we've turned publisher confirms on, and we've published a message
				 * here is a message being confirmed
				 */

				return php_amqp_handle_basic_ack(message, resource, channel_id, channel, &frame.payload.method TSRMLS_CC);
			case AMQP_BASIC_NACK_METHOD:
				/* if we've turned publisher confirms on, and we've published a message
				 * here is a message being confirmed
				 */

				return php_amqp_handle_basic_nack(message, resource, channel_id, channel, &frame.payload.method TSRMLS_CC);
			case AMQP_BASIC_RETURN_METHOD:
				/* if a published message couldn't be routed and the mandatory flag was set
				 * this is what would be returned. The message then needs to be read.
				 */

				return php_amqp_handle_basic_return(message, resource, channel_id, channel, &frame.payload.method TSRMLS_CC);
			default:
				if (*message != NULL) {
					efree(*message);
				}

				spprintf(message, 0, "Library error: An unexpected method was received 0x%08X\n", frame.payload.method.id);
				return PHP_AMQP_RESOURCE_RESPONSE_ERROR;
		}
	}

	if (*message != NULL) {
		efree(*message);
	}

	spprintf(message, 0, "Library error: %s", amqp_error_string2(reply.library_error));
	return PHP_AMQP_RESOURCE_RESPONSE_ERROR;
}

/* Socket-related functions */
int php_amqp_set_resource_read_timeout(amqp_connection_resource *resource, double timeout TSRMLS_DC)
{
	assert(timeout >= 0.0);

#ifdef PHP_WIN32
	DWORD read_timeout;
	/*
	In Windows, setsockopt with SO_RCVTIMEO sets actual timeout
	to a value that's 500ms greater than specified value.
	Also, it's not possible to set timeout to any value below 500ms.
	Zero timeout works like it should, however.
	*/
	if (timeout == 0.) {
		read_timeout = 0;
	} else {
		read_timeout = (int) (max(timeout * 1.e+3 - .5e+3, 1.));
	}
#else
	struct timeval read_timeout;
	read_timeout.tv_sec = (int) floor(timeout);
	read_timeout.tv_usec = (int) ((timeout - floor(timeout)) * 1.e+6);
#endif

	if (0 != setsockopt(amqp_get_sockfd(resource->connection_state), SOL_SOCKET, SO_RCVTIMEO, (char *)&read_timeout, sizeof(read_timeout))) {
		zend_throw_exception(amqp_connection_exception_class_entry, "Socket error: cannot setsockopt SO_RCVTIMEO", 0 TSRMLS_CC);
		return 0;
	}

	return 1;
}

int php_amqp_set_resource_rpc_timeout(amqp_connection_resource *resource, double timeout TSRMLS_DC)
{
	assert(timeout >= 0.0);

#if AMQP_VERSION_MAJOR * 100 + AMQP_VERSION_MINOR * 10 + AMQP_VERSION_PATCH >= 90
	struct timeval rpc_timeout;

	if (timeout == 0.) return 1;

	rpc_timeout.tv_sec = (int) floor(timeout);
	rpc_timeout.tv_usec = (int) ((timeout - floor(timeout)) * 1.e+6);

	if (AMQP_STATUS_OK != amqp_set_rpc_timeout(resource->connection_state, &rpc_timeout)) {
		zend_throw_exception(amqp_connection_exception_class_entry, "Library error: cannot set rpc_timeout", 0 TSRMLS_CC);
		return 0;
	}
#endif

	return 1;
}

int php_amqp_set_resource_write_timeout(amqp_connection_resource *resource, double timeout TSRMLS_DC)
{
	assert(timeout >= 0.0);

#ifdef PHP_WIN32
	DWORD write_timeout;

	if (timeout == 0.) {
		write_timeout = 0;
	} else {
		write_timeout = (int) (max(timeout * 1.e+3 - .5e+3, 1.));
	}
#else
	struct timeval write_timeout;
	write_timeout.tv_sec = (int) floor(timeout);
	write_timeout.tv_usec = (int) ((timeout - floor(timeout)) * 1.e+6);
#endif

	if (0 != setsockopt(amqp_get_sockfd(resource->connection_state), SOL_SOCKET, SO_SNDTIMEO, (char *)&write_timeout, sizeof(write_timeout))) {
		zend_throw_exception(amqp_connection_exception_class_entry, "Socket error: cannot setsockopt SO_SNDTIMEO", 0 TSRMLS_CC);
		return 0;
	}

	return 1;
}


/* Channel-related functions */

amqp_channel_t php_amqp_connection_resource_get_available_channel_id(amqp_connection_resource *resource)
{
	assert(resource != NULL);
	assert(resource->slots != NULL);

	/* Check if there are any open slots */
	if (resource->used_slots >= resource->max_slots) {
		return 0;
	}

	amqp_channel_t slot;

	for (slot = 0; slot < resource->max_slots; slot++) {
		if (resource->slots[slot] == 0) {
			return (amqp_channel_t) (slot + 1);
		}
	}

	return 0;
}

int php_amqp_connection_resource_register_channel(amqp_connection_resource *resource, amqp_channel_resource *channel_resource, amqp_channel_t channel_id)
{
	assert(resource != NULL);
	assert(resource->slots != NULL);
	assert(channel_id > 0 && channel_id <= resource->max_slots);

	if (resource->slots[channel_id - 1] != 0) {
		return FAILURE;
	}

	resource->slots[channel_id - 1] = channel_resource;
	channel_resource->connection_resource = resource;
	resource->used_slots++;

	return SUCCESS;
}

int php_amqp_connection_resource_unregister_channel(amqp_connection_resource *resource, amqp_channel_t channel_id)
{
	assert(resource != NULL);
	assert(resource->slots != NULL);
	assert(channel_id > 0 && channel_id <= resource->max_slots);

	if (resource->slots[channel_id - 1] != 0) {
		resource->slots[channel_id - 1]->connection_resource = NULL;

		resource->slots[channel_id - 1] = 0;
		resource->used_slots--;
	}

	return SUCCESS;
}


/* Creating and destroying resource */

amqp_connection_resource *connection_resource_constructor(amqp_connection_params *params, zend_bool persistent TSRMLS_DC)
{
	struct timeval tv = {0};
	struct timeval *tv_ptr = &tv;

	char *std_datetime;
	amqp_table_entry_t client_properties_entries[5];
	amqp_table_t       client_properties_table;

	amqp_table_entry_t custom_properties_entries[2];
	amqp_table_t       custom_properties_table;

	amqp_connection_resource *resource;

	/* Allocate space for the connection resource */
	resource = (amqp_connection_resource *)pecalloc(1, sizeof(amqp_connection_resource), persistent);

	/* Create the connection */
	resource->connection_state = amqp_new_connection();

	/* Create socket object */
	if (params->cacert) {
		resource->socket = amqp_ssl_socket_new(resource->connection_state);

		if (!resource->socket) {
			zend_throw_exception(amqp_connection_exception_class_entry, "Socket error: could not create SSL socket.", 0 TSRMLS_CC);

			return NULL;
		}
	} else {
		resource->socket = amqp_tcp_socket_new(resource->connection_state);

		if (!resource->socket) {
			zend_throw_exception(amqp_connection_exception_class_entry, "Socket error: could not create socket.", 0 TSRMLS_CC);

			return NULL;
		}
	}

	if (params->cacert && amqp_ssl_socket_set_cacert(resource->socket, params->cacert)) {
		zend_throw_exception(amqp_connection_exception_class_entry, "Socket error: could not set CA certificate.", 0 TSRMLS_CC);

		return NULL;
	}

	if (params->cacert) {
#if AMQP_VERSION_MAJOR * 100 + AMQP_VERSION_MINOR * 10 + AMQP_VERSION_PATCH >= 80
		amqp_ssl_socket_set_verify_peer(resource->socket, params->verify);
		amqp_ssl_socket_set_verify_hostname(resource->socket, params->verify);
#else
		amqp_ssl_socket_set_verify(resource->socket, params->verify);
#endif
	}

	if (params->cert && params->key && amqp_ssl_socket_set_key(resource->socket, params->cert, params->key)) {
		zend_throw_exception(amqp_connection_exception_class_entry, "Socket error: could not setting client cert.", 0 TSRMLS_CC);

		return NULL;
	}

	if (params->connect_timeout > 0) {
		tv.tv_sec = (long int) params->connect_timeout;
		tv.tv_usec = (long int) ((params->connect_timeout - tv.tv_sec) * 1000000);
	} else {
		tv_ptr = NULL;
	}

	/* Try to connect and verify that no error occurred */
	if (amqp_socket_open_noblock(resource->socket, params->host, params->port, tv_ptr)) {

		zend_throw_exception(amqp_connection_exception_class_entry, "Socket error: could not connect to host.", 0 TSRMLS_CC);

		connection_resource_destructor(resource, persistent TSRMLS_CC);

		return NULL;
	}

	if (!php_amqp_set_resource_read_timeout(resource, params->read_timeout TSRMLS_CC)) {
		connection_resource_destructor(resource, persistent TSRMLS_CC);
		return NULL;
	}

	if (!php_amqp_set_resource_write_timeout(resource, params->write_timeout TSRMLS_CC)) {
		connection_resource_destructor(resource, persistent TSRMLS_CC);
		return NULL;
	}

	if (!php_amqp_set_resource_rpc_timeout(resource, params->rpc_timeout TSRMLS_CC)) {
		connection_resource_destructor(resource, persistent TSRMLS_CC);
		return NULL;
	}

	std_datetime = php_std_date(time(NULL) TSRMLS_CC);

	client_properties_entries[0].key               = amqp_cstring_bytes("type");
	client_properties_entries[0].value.kind        = AMQP_FIELD_KIND_UTF8;
	client_properties_entries[0].value.value.bytes = amqp_cstring_bytes("php-amqp extension");

	client_properties_entries[1].key               = amqp_cstring_bytes("version");
	client_properties_entries[1].value.kind        = AMQP_FIELD_KIND_UTF8;
	client_properties_entries[1].value.value.bytes = amqp_cstring_bytes(PHP_AMQP_VERSION);

	client_properties_entries[2].key               = amqp_cstring_bytes("revision");
	client_properties_entries[2].value.kind        = AMQP_FIELD_KIND_UTF8;
	client_properties_entries[2].value.value.bytes = amqp_cstring_bytes(PHP_AMQP_REVISION);

	client_properties_entries[3].key               = amqp_cstring_bytes("connection type");
	client_properties_entries[3].value.kind        = AMQP_FIELD_KIND_UTF8;
	client_properties_entries[3].value.value.bytes = amqp_cstring_bytes(persistent ? "persistent" : "transient");

	client_properties_entries[4].key               = amqp_cstring_bytes("connection started");
	client_properties_entries[4].value.kind        = AMQP_FIELD_KIND_UTF8;
	client_properties_entries[4].value.value.bytes = amqp_cstring_bytes(std_datetime);

	client_properties_table.entries = client_properties_entries;
	client_properties_table.num_entries = sizeof(client_properties_entries) / sizeof(amqp_table_entry_t);

	custom_properties_entries[0].key               = amqp_cstring_bytes("client");
	custom_properties_entries[0].value.kind        = AMQP_FIELD_KIND_TABLE;
	custom_properties_entries[0].value.value.table = client_properties_table;


	if (params->connection_name) {
		custom_properties_entries[1].key               = amqp_cstring_bytes("connection_name");
		custom_properties_entries[1].value.kind        = AMQP_FIELD_KIND_UTF8;
		custom_properties_entries[1].value.value.bytes = amqp_cstring_bytes(params->connection_name);
	}

	custom_properties_table.entries     = custom_properties_entries;
	custom_properties_table.num_entries = params->connection_name ? 2 : 1;

	/* We can assume that connection established here but it is not true, real handshake goes during login */

	assert(params->frame_max > 0);

	amqp_rpc_reply_t res = amqp_login_with_properties(
		resource->connection_state,
		params->vhost,
		params->channel_max,
		params->frame_max,
		params->heartbeat,
		&custom_properties_table,
		params->sasl_method,
		params->login,
		params->password
	);

	efree(std_datetime);

	if (AMQP_RESPONSE_NORMAL != res.reply_type) {
		char *message = NULL, *long_message = NULL;

		php_amqp_connection_resource_error(res, &message, resource, 0 TSRMLS_CC);

		spprintf(&long_message, 0, "%s - Potential login failure.", message);
		zend_throw_exception(amqp_connection_exception_class_entry, long_message, PHP_AMQP_G(error_code) TSRMLS_CC);

		efree(message);
		efree(long_message);

		/* https://www.rabbitmq.com/resources/specs/amqp0-9-1.pdf
		 *
		 * 2.2.4 The Connection Class:
		 * ... a peer that detects an error MUST close the socket without sending any further data.
		 *
		 * 4.10.2 Denial of Service Attacks:
		 * ... The general response to any exceptional condition in the connection negotiation is to pause that connection
		 * (presumably a thread) for a period of several seconds and then to close the network connection. This
		 * includes syntax errors, over-sized data, and failed attempts to authenticate.
		 */
		connection_resource_destructor(resource, persistent TSRMLS_CC);
		return NULL;
	}

	/* Allocate space for the channel slots in the ring buffer */
	resource->max_slots = (amqp_channel_t) amqp_get_channel_max(resource->connection_state);
	assert(resource->max_slots > 0);

	resource->slots = (amqp_channel_resource **)pecalloc(resource->max_slots + 1, sizeof(amqp_channel_object*), persistent);

	resource->is_connected = '\1';

	return resource;
}

ZEND_RSRC_DTOR_FUNC(amqp_connection_resource_dtor_persistent)
{
	amqp_connection_resource *resource = (amqp_connection_resource *)PHP5to7_ZEND_RESOURCE_DTOR_ARG->ptr;

	connection_resource_destructor(resource, 1 TSRMLS_CC);
}

ZEND_RSRC_DTOR_FUNC(amqp_connection_resource_dtor)
{
	amqp_connection_resource *resource = (amqp_connection_resource *)PHP5to7_ZEND_RESOURCE_DTOR_ARG->ptr;

	connection_resource_destructor(resource, 0 TSRMLS_CC);
}

static void connection_resource_destructor(amqp_connection_resource *resource, int persistent TSRMLS_DC)
{
	assert(resource != NULL);

#ifndef PHP_WIN32
	void * old_handler;

	/*
	If we are trying to close the connection and the connection already closed, it will throw
	SIGPIPE, which is fine, so ignore all SIGPIPES
	*/

	/* Start ignoring SIGPIPE */
	old_handler = signal(SIGPIPE, SIG_IGN);
#endif

	if (resource->parent) {
		resource->parent->connection_resource = NULL;
	}

	if (resource->slots) {
		php_amqp_prepare_for_disconnect(resource TSRMLS_CC);

		pefree(resource->slots, persistent);
		resource->slots = NULL;
	}

	/* connection may be closed in case of previous failure */
	if (resource->is_connected) {
		amqp_connection_close(resource->connection_state, AMQP_REPLY_SUCCESS);
	}

	amqp_destroy_connection(resource->connection_state);

#ifndef PHP_WIN32
	/* End ignoring of SIGPIPEs */
	signal(SIGPIPE, old_handler);
#endif

	pefree(resource, persistent);
}

void php_amqp_prepare_for_disconnect(amqp_connection_resource *resource TSRMLS_DC)
{
	if (resource == NULL) {
		return;
	}

	if(resource->slots != NULL) {
		/* NOTE: when we have persistent connection we do not move channels between php requests
		 *       due to current php-amqp extension limitation in AMQPChannel where __construct == channel.open AMQP method call
		 *       and __destruct = channel.close AMQP method call
		 */

		/* Clean up old memory allocations which are now invalid (new connection) */
		amqp_channel_t slot;

		for (slot = 0; slot < resource->max_slots; slot++) {
			if (resource->slots[slot] != 0) {
				php_amqp_close_channel(resource->slots[slot], 0 TSRMLS_CC);
			}
		}
	}

	/* If it's persistent connection do not destroy connection resource (this keep connection alive) */
	if (resource->is_persistent) {
		/* Cleanup buffers to reduce memory usage in idle mode */
		amqp_maybe_release_buffers(resource->connection_state);
	}

	return;
}

