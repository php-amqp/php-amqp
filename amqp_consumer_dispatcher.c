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
zend_object_handlers amqp_consumer_dispatcher_object_handlers;
HashTable *amqp_consumer_dispatcher_object_get_debug_info(zval *object, int *is_temp TSRMLS_DC) {
	zval *value;
	HashTable *debug_info;

	/* Get the object from which to read */
	amqp_consumer_dispatcher_object *dispatcher = (amqp_consumer_dispatcher_object *)zend_object_store_get_object(object TSRMLS_CC);

	/* Let zend clean up for us: */
	*is_temp = 1;

	/* Keep the # 1 matching the number of entries in this table*/
	ALLOC_HASHTABLE(debug_info);
	ZEND_INIT_SYMTABLE_EX(debug_info, 1 + 1, 0);

	Z_ADDREF_P(dispatcher->consumers);
	zend_hash_add(debug_info, "consumers", sizeof("consumers"), &dispatcher->consumers, sizeof(&dispatcher->consumers), NULL);
	
	return debug_info;
}
#endif

void amqp_consumer_dispatcher_dtor(void *object TSRMLS_DC)
{
	amqp_consumer_dispatcher_object *consumer_dispatcher = (amqp_consumer_dispatcher_object*)object;
	
	if(consumer_dispatcher->consumers) {
		zval_ptr_dtor(&consumer_dispatcher->consumers);
	}
	
	zend_object_std_dtor(&consumer_dispatcher->zo TSRMLS_CC);
	
	efree(object);
}

zend_object_value amqp_consumer_dispatcher_ctor(zend_class_entry *ce TSRMLS_DC)
{
	zend_object_value new_value;
	amqp_consumer_dispatcher_object *consumer_dispatcher = (amqp_consumer_dispatcher_object*)emalloc(sizeof(amqp_consumer_dispatcher_object));
	
	memset(consumer_dispatcher, 0, sizeof(amqp_consumer_dispatcher_object));
	
	zend_object_std_init(&consumer_dispatcher->zo, ce TSRMLS_CC);
	AMQP_OBJECT_PROPERTIES_INIT(consumer_dispatcher->zo, ce);
	
	MAKE_STD_ZVAL(consumer_dispatcher->consumers);
	array_init(consumer_dispatcher->consumers);

	new_value.handle = zend_objects_store_put(
		consumer_dispatcher,
		(zend_objects_store_dtor_t)zend_objects_destroy_object,
		(zend_objects_free_object_storage_t)amqp_consumer_dispatcher_dtor,
		NULL TSRMLS_CC
	);
	
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3
	memcpy((void *)&amqp_consumer_dispatcher_object_handlers, (void *)zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	amqp_consumer_dispatcher_object_handlers.get_debug_info = amqp_consumer_dispatcher_object_get_debug_info;
	new_value.handlers = &amqp_consumer_dispatcher_object_handlers;
#else
	new_value.handlers = zend_get_std_object_handlers();
#endif

	return new_value;
}



/*
 * Verify that the given zval is a consumer and attempt to return the connection object
 * If NULL is returned, then an exception will be on the stack, so callers should return immediately
 */
amqp_connection_object *getConsumerConnection(zval *z_consumer)
{
	amqp_queue_object *queue;
	amqp_connection_object *connection;
	amqp_channel_object *channel;
	
	zval *z_queue = NULL;
	
	if(!z_consumer) {
		zend_throw_exception(amqp_queue_exception_class_entry, "Consumer expected", 0 TSRMLS_CC);
		return NULL;
	}
		
	if(Z_TYPE_P(z_consumer) != IS_OBJECT) {
		zend_throw_exception(amqp_queue_exception_class_entry, "AMQPConsumer expected", 0 TSRMLS_CC);
		return NULL;
	}
	
	zend_call_method(&z_consumer, NULL, NULL, "getQueue", sizeof("getQueue")-1, &z_queue, 0, NULL, NULL TSRMLS_CC);
	
	if(!z_queue) {
		zend_throw_exception(amqp_queue_exception_class_entry, "Consumer not bound", 0 TSRMLS_CC);
		return NULL;
	}
		
	if (Z_TYPE_P(z_queue) != IS_OBJECT) {
		php_printf("Type of queue is %d", Z_TYPE_P(z_queue));
		zend_throw_exception(amqp_queue_exception_class_entry, "Consumer not bound to a queue", 0 TSRMLS_CC);
		return NULL;
	}

	queue = (amqp_queue_object *)zend_object_store_get_object(z_queue TSRMLS_CC);
	channel = AMQP_GET_CHANNEL(queue);
	AMQP_VERIFY_CHANNEL(channel, "Could not get channel.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not get connection.");
	
	return connection;
}

/*
 * Rotate the array of consumers
 */
void rotate_consumers(amqp_consumer_dispatcher_object *consumer_dispatcher)
{
	zval *retval1, *retval2, *arrayRef;

	MAKE_STD_ZVAL(arrayRef);
	ZVAL_ZVAL(arrayRef, consumer_dispatcher->consumers, 0, 0);
		
	/* Rotate the array */
	
	zend_call_method(NULL, NULL, NULL, "array_shift", sizeof("array_shift")-1, &retval1, 1, arrayRef, NULL);
	zend_call_method(NULL, NULL, NULL, "array_push", sizeof("array_push")-1, &retval2, 2, arrayRef, retval1);
	
	zval_ptr_dtor(&retval1);
	zval_ptr_dtor(&retval2);
	zval_delref_p(arrayRef);
	zval_ptr_dtor(&arrayRef);
}


/* {{{ proto AMQPConsumerDispatcher::__construct(array consumers)
 */
PHP_METHOD(amqp_consumer_dispatcher_class, __construct)
{
	zval *id, *zvalConsumers;
	amqp_consumer_dispatcher_object *consumer_dispatcher;

	/* Parse out the method parameters */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Oa", &id, amqp_consumer_dispatcher_class_entry, &zvalConsumers) == FAILURE) {
		return;
	}

	consumer_dispatcher = (amqp_consumer_dispatcher_object *)zend_object_store_get_object(id TSRMLS_CC);
	if(consumer_dispatcher->consumers) {
		zval_ptr_dtor(&consumer_dispatcher->consumers);
	}
	
	consumer_dispatcher->consumers = zvalConsumers;
	
	/* Increment the ref count on the consumers array */
	Z_ADDREF_P(consumer_dispatcher->consumers);

}
/* }}} */


/* {{{ proto AMQPConsumerDispatcher::rotateConsumers()
 */
PHP_METHOD(amqp_consumer_dispatcher_class, rotateConsumers)
{
	zval *id;
	amqp_consumer_dispatcher_object *consumer_dispatcher;
	
	/* Parse out the method parameters */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_consumer_dispatcher_class_entry) == FAILURE) {
		return;
	}

	consumer_dispatcher = (amqp_consumer_dispatcher_object *)zend_object_store_get_object(id TSRMLS_CC);
	rotate_consumers(consumer_dispatcher);
}
/* }}} */


/* {{{ proto int AMQPConsumerDispatcher::select([long timeout = 0]]);
select
@return NULL if no consumer was selected
*/
PHP_METHOD(amqp_consumer_dispatcher_class, select)
{
	zval *id;
	amqp_consumer_dispatcher_object *consumer_dispatcher;
	long  timeout = 0;

	HashTable *cht;
	HashPosition pos;
	
	zval **data;
	
	struct timeval tv;
	
	fd_set read_fd;
	fd_set except_fd;
	
	int max_fd = -1;
	
	int res;
	
	int nConsumers;
	zval *z_consumer;
	amqp_connection_object *connection;
		


	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O|l", &id, amqp_consumer_dispatcher_class_entry, &timeout) == FAILURE) {
		return;
	}
	
	consumer_dispatcher = (amqp_consumer_dispatcher_object *)zend_object_store_get_object(id TSRMLS_CC);
	
	tv.tv_sec = timeout;
	tv.tv_usec = 0;
		
	FD_ZERO(&read_fd);
	FD_ZERO(&except_fd);
	
	rotate_consumers(consumer_dispatcher);
	
	 /* Iterate over the array pulling out AMQPConsumer objects so we can get at their queues */
	cht = Z_ARRVAL_P(consumer_dispatcher->consumers);
	
	nConsumers = zend_hash_num_elements(cht);
	
	for (zend_hash_internal_pointer_reset_ex(cht, &pos);
		zend_hash_get_current_data_ex(cht, (void**) &data, &pos) == SUCCESS;
		zend_hash_move_forward_ex(cht, &pos)) {
		
		z_consumer = *data;
		connection = getConsumerConnection(z_consumer);
	
		if(!connection) {
			return; /* Exception has been thrown*/
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
		return;
	}
	
	/* Find the first consumer which is in the set */
	/* Do something more clever in the future */	
	for (zend_hash_internal_pointer_reset_ex(cht, &pos);
		zend_hash_get_current_data_ex(cht, (void**) &data, &pos) == SUCCESS;
		zend_hash_move_forward_ex(cht, &pos)) {
		
		z_consumer = *data;
		
		connection = getConsumerConnection(z_consumer);
		
		if(!connection) {
			return; /* Exception has been thrown*/
		}
		
		int fd = connection->connection_resource->fd;
		if(FD_ISSET(fd, &read_fd)) {
			RETURN_ZVAL(z_consumer, 1, 0);
		}
	}
	
	return;
}
/* }}} */

/* {{{ proto AMQPConsumerDispatcher::hasConsumers()
Return true if any consumers are still registered */
PHP_METHOD(amqp_consumer_dispatcher_class, hasConsumers)
{
	zval *id;
	amqp_consumer_dispatcher_object *consumer_dispatcher;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_consumer_dispatcher_class_entry) == FAILURE) {
		return;
	}

	consumer_dispatcher = (amqp_consumer_dispatcher_object *)zend_object_store_get_object(id TSRMLS_CC);

	if (zend_hash_num_elements(Z_ARRVAL_P(consumer_dispatcher->consumers))) {
		RETURN_TRUE;
	}
	
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto AMQPConsumerDispatcher::getConsumers
Return the array of consumers */
PHP_METHOD(amqp_consumer_dispatcher_class, getConsumers)
{
	zval *id;
	amqp_consumer_dispatcher_object *consumer_dispatcher;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_consumer_dispatcher_class_entry) == FAILURE) {
		return;
	}
	
	consumer_dispatcher = (amqp_consumer_dispatcher_object *)zend_object_store_get_object(id TSRMLS_CC);

	*return_value = *consumer_dispatcher->consumers;
	zval_copy_ctor(return_value);

	/* Increment the ref count */
	Z_ADDREF_P(consumer_dispatcher->consumers);
}
/* }}} */

/* {{{ proto AMQPConsumerDispatcher::removeConsumer(AMQPConsumer consumer);
create Exchange   */
PHP_METHOD(amqp_consumer_dispatcher_class, removeConsumer)
{
	zval *id;
	zval *consumerObj;
	amqp_consumer_object *consumer;
	amqp_consumer_dispatcher_object *consumer_dispatcher;

	HashTable *cht;
	HashPosition pos;
	zval **data;
	
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "OO", &id, amqp_consumer_dispatcher_class_entry, &consumerObj, amqp_consumer_class_entry) == FAILURE) {
		RETURN_NULL();
	}

	if (!instanceof_function(Z_OBJCE_P(consumerObj), amqp_consumer_class_entry TSRMLS_CC)) {
		zend_throw_exception(amqp_exchange_exception_class_entry, "The first parameter must be an instance of AMQPConsumer.", 0 TSRMLS_CC);
		return;
	}

	consumer_dispatcher = (amqp_consumer_dispatcher_object *)zend_object_store_get_object(id TSRMLS_CC);
	consumer = (amqp_consumer_object *)zend_object_store_get_object(consumerObj TSRMLS_CC);

	 /* Iterate over the array pulling out AMQPConsumer objects so we can get at their queues */
	cht = Z_ARRVAL_P(consumer_dispatcher->consumers);
	
	for (zend_hash_internal_pointer_reset_ex(cht, &pos);
		zend_hash_get_current_data_ex(cht, (void**) &data, &pos) == SUCCESS;
		zend_hash_move_forward_ex(cht, &pos)) {
	
		zval *z_consumer = *data;
		amqp_consumer_object* check = (amqp_consumer_object*)zend_object_store_get_object(z_consumer TSRMLS_CC);
	
		if(check == consumer) {
			zend_hash_index_del(Z_ARRVAL_P(consumer_dispatcher->consumers), pos->h);
			zend_hash_rehash(Z_ARRVAL_P(consumer_dispatcher->consumers));
		}
	}
}
/* }}} */

