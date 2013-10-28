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
  | Author: Duncan McIntyre dmcintyre@gopivotal.com Copyright (c) 2013                                                |
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
# include <Winsock2.h>
# include <ws2tcpip.h>
#else
# include <signal.h>
# include <stdint.h>

# include <sys/types.h>      /* On older BSD this must come before net includes */
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <sys/socket.h>
# include <netdb.h>
# include <sys/uio.h>
# include <fcntl.h>


#endif

#include <amqp.h>
#include <amqp_framing.h>

#ifdef PHP_WIN32
# include "win32/unistd.h"
#else
# include <unistd.h>
#endif

#include "php_amqp.h"


#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3
zend_object_handlers amqp_selector_object_handlers;
HashTable *amqp_selector_object_get_debug_info(zval *object, int *is_temp TSRMLS_DC) {
	zval *value;
	HashTable *debug_info;

	/* Get the object from which to read */
	amqp_selector_object *sel = (amqp_selector_object *)zend_object_store_get_object(object TSRMLS_CC);

	/* Let zend clean up for us: */
	*is_temp = 1;

	/* Keep the # 7 matching the number of entries in this table*/
	ALLOC_HASHTABLE(debug_info);
	ZEND_INIT_SYMTABLE_EX(debug_info, 0 + 1, 0);
	
	return debug_info;
}
#endif

void amqp_selector_dtor(void *object TSRMLS_DC)
{
	efree(object);
}

zend_object_value amqp_selector_ctor(zend_class_entry *ce TSRMLS_DC)
{
	zend_object_value new_value;
	amqp_selector_object *selector = (amqp_selector_object*)emalloc(sizeof(amqp_selector_object));
	memset(selector, 0, sizeof(amqp_selector_object));
	
	
	zend_object_std_init(&selector->zo, ce TSRMLS_CC);
	AMQP_OBJECT_PROPERTIES_INIT(selector->zo, ce);

	
	new_value.handle = zend_objects_store_put(
		selector,
		(zend_objects_store_dtor_t)zend_objects_destroy_object,
		(zend_objects_free_object_storage_t)amqp_selector_dtor,
		NULL TSRMLS_CC
	);
	
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3
	memcpy((void *)&amqp_selector_object_handlers, (void *)zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	new_value.handlers = &amqp_selector_object_handlers;
#else
	new_value.handlers = zend_get_std_object_handlers();
#endif

	return new_value;
}



/**
 * Verify that the given zval is a consumer and attempt to return the connection object
 * If NULL is returned, then an exception will be on the stack, so callers should return immediately
 */
amqp_connection_object *getConsumerConnection(zval *z_consumer)
{
	amqp_queue_object *queue;
	amqp_connection_object *connection;
	amqp_channel_object *channel;
	
	zval *z_queue;
	
	if(Z_TYPE_P(z_consumer) != IS_OBJECT) {
		zend_throw_exception(amqp_queue_exception_class_entry, "AMQPConsumer expected", 0 TSRMLS_CC);
		return NULL;
	}
	
	zend_call_method(&z_consumer, NULL, NULL, "getqueue", sizeof("getqueue")-1, &z_queue, 0, NULL, NULL TSRMLS_CC);
	
	if(!z_queue) {
		zend_throw_exception(amqp_queue_exception_class_entry, "Consumer not bound", 0 TSRMLS_CC);
		return;
	}
		
	if (Z_TYPE_P(z_queue) != IS_OBJECT) {
		zend_throw_exception(amqp_queue_exception_class_entry, "Consumer not bound to a queue", 0 TSRMLS_CC);
		return;
	}

	queue = (amqp_queue_object *)zend_object_store_get_object(z_queue TSRMLS_CC);
	channel = AMQP_GET_CHANNEL(queue);
	AMQP_VERIFY_CHANNEL(channel, "Could not get channel.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not get connection.");
	
	return connection;
}

/* {{{ proto AMQPSelector::__construct()
 */
PHP_METHOD(amqp_selector_class, __construct)
{
	zval *id;

	amqp_selector_object *selector;

	/* Parse out the method parameters */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_selector_class_entry) == FAILURE) {
		return;
	}

	selector = (amqp_selector_object *)zend_object_store_get_object(id TSRMLS_CC);

}
/* }}} */


/* {{{ proto int AMQPSelector::select(array consumers, [long timeout = 0]]);
select
*/
PHP_METHOD(amqp_selector_class, select)
{
	zval *id, *zvalArguments;
	long  timeout = 0;

	HashTable *argumentHash;
	HashPosition pos;
	zval **data;
	amqp_table_t *arguments;
	
	struct timeval tv;
	
	fd_set read_fd;
	fd_set except_fd;
	
	int max_fd = -1;
	
	int res;


	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Oa|l", &id, amqp_selector_class_entry, &zvalArguments, &timeout) == FAILURE) {
		return;
	}
	

	tv.tv_sec = timeout;
	tv.tv_usec = 0;
		
	FD_ZERO(&read_fd);
	FD_ZERO(&except_fd);
	
	 /* Iterate over the array pulling out AMQPConsumer objects so we can get at their queues */
	argumentHash = Z_ARRVAL_P(zvalArguments);
	
	for (zend_hash_internal_pointer_reset_ex(argumentHash, &pos);
		zend_hash_get_current_data_ex(argumentHash, (void**) &data, &pos) == SUCCESS;
		zend_hash_move_forward_ex(argumentHash, &pos)) {
	
		zval *z_consumer = *data;
		amqp_connection_object *connection;
		
		connection = getConsumerConnection(z_consumer);
		if(!connection) {
			return;
		}

		if(amqp_data_in_buffer(connection->connection_resource->connection_state)) {
			RETURN_ZVAL(z_consumer, 1, 0);
		}
					
		int fd = connection->connection_resource->fd;
		
		if(fd > max_fd) {
			max_fd = fd;
		}
		
		FD_SET(fd, &read_fd);
		FD_SET(fd, &except_fd);
	}
	res =  select(max_fd+1, &read_fd, NULL, &except_fd, &tv);
	if(res <= 0) {
		RETURN_FALSE;
	}
	
	// Find the first consumer which is in the set
	// Do something more clever in the future
	for (zend_hash_internal_pointer_reset_ex(argumentHash, &pos);
		zend_hash_get_current_data_ex(argumentHash, (void**) &data, &pos) == SUCCESS;
		zend_hash_move_forward_ex(argumentHash, &pos)) {
		
		zval *z_consumer = *data;
		amqp_connection_object *connection;
		
		connection = getConsumerConnection(z_consumer);
		if(!connection) {
			return;
		}
		
		int fd = connection->connection_resource->fd;
		if(FD_ISSET(fd, &read_fd)) {
			RETURN_ZVAL(z_consumer, 1, 0);
		}
	}
	
	RETURN_FALSE;

	
}
/* }}} */
