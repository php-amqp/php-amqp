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
#else
# include <signal.h>
# include <stdint.h>
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
zend_object_handlers amqp_consumer_object_handlers;
HashTable *amqp_consumer_object_get_debug_info(zval *object, int *is_temp TSRMLS_DC) {
	zval *value;
	HashTable *debug_info;

	/* Get the object from which to read */
	amqp_consumer_object *consumer = (amqp_consumer_object *)zend_object_store_get_object(object TSRMLS_CC);

	/* Let zend clean up for us: */
	*is_temp = 1;

	/* Keep the # 2 matching the number of entries in this table*/
	ALLOC_HASHTABLE(debug_info);
	ZEND_INIT_SYMTABLE_EX(debug_info, 1 + 1, 0);
	
	Z_ADDREF_P(consumer->queue);
	zend_hash_add(debug_info, "queue", sizeof("queue"), &consumer->queue, sizeof(&consumer->queue), NULL);


	return debug_info;
}
#endif

void amqp_consumer_dtor(void *object TSRMLS_DC)
{
	amqp_consumer_object *consumer = (amqp_consumer_object*)object;
	
	zend_object_std_dtor(&consumer->zo TSRMLS_CC);
	
	efree(object);
}

zend_object_value amqp_consumer_ctor(zend_class_entry *ce TSRMLS_DC)
{
	zend_object_value new_value;
	amqp_consumer_object *consumer = (amqp_consumer_object*)emalloc(sizeof(amqp_consumer_object));
	memset(consumer, 0, sizeof(amqp_consumer_object));
	
	zend_object_std_init(&consumer->zo, ce TSRMLS_CC);
	AMQP_OBJECT_PROPERTIES_INIT(consumer->zo, ce);

	
	new_value.handle = zend_objects_store_put(
		consumer,
		(zend_objects_store_dtor_t)zend_objects_destroy_object,
		(zend_objects_free_object_storage_t)amqp_consumer_dtor,
		NULL TSRMLS_CC
	);
	
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3
	memcpy((void *)&amqp_consumer_object_handlers, (void *)zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	amqp_consumer_object_handlers.get_debug_info = amqp_consumer_object_get_debug_info;
	new_value.handlers = &amqp_consumer_object_handlers;
#else
	new_value.handlers = zend_get_std_object_handlers();
#endif

	return new_value;
}

/* {{{ proto AMQPConsumer::__construct(AMQPQueue queue, fnCallback)
 */
PHP_METHOD(amqp_consumer_class, __construct)
{
	zval *id;
	amqp_consumer_object *consumer;
	zval *queueObj;
	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache;
	
	/* Parse out the method parameters */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "OOf", &id, amqp_consumer_class_entry, &queueObj, amqp_queue_class_entry, &fci, &fci_cache) == FAILURE) {
		return;
	}

	consumer = (amqp_consumer_object *)zend_object_store_get_object(id TSRMLS_CC);
	consumer->queue = queueObj;
	consumer->callback.fci = fci;
	consumer->callback.fci_cache = fci_cache;
	
	/* Increment the ref count on the queue */
	Z_ADDREF_P(consumer->queue);
}
/* }}} */


/* {{{ proto int AMQPConsumer::getQueue();
select
*/
PHP_METHOD(amqp_consumer_class, getQueue)
{
	zval *id;
	amqp_consumer_object *consumer;
	
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_consumer_class_entry) == FAILURE) {
		return;
	}
		
	consumer = (amqp_consumer_object *)zend_object_store_get_object(id TSRMLS_CC);
	RETURN_ZVAL(consumer->queue, 1, 0);
}
/* }}} */


/* {{{ proto int AMQPConsumer::basicConsume([flags = <bitmask>, consumer_tag]);
start consuming on the queue
*/
PHP_METHOD(amqp_consumer_class, basicConsume)
{
	zval *id;
	amqp_consumer_object *consumer;
	amqp_queue_object *queue;
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache;
	int function_call_succeeded = 1;
	int read;
	amqp_table_t *arguments;

	char *consumer_tag;
	int consumer_tag_len = 0;
	amqp_bytes_t consumer_tag_bytes;
	long flags = INI_INT("amqp.auto_ack") ? AMQP_AUTOACK : AMQP_NOPARAM;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O|ls", &id, amqp_consumer_class_entry, &flags, &consumer_tag, &consumer_tag_len) == FAILURE) {
		return;
	}

	consumer = (amqp_consumer_object *)zend_object_store_get_object(id TSRMLS_CC);

	queue = (amqp_queue_object*) zend_object_store_get_object(consumer->queue TSRMLS_CC);
	
	channel = AMQP_GET_CHANNEL(queue);
	AMQP_VERIFY_CHANNEL(channel, "Could not get queue.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not get queue.");

	/* Setup the consume */
	arguments = convert_zval_to_arguments(queue->arguments);

	consumer_tag_bytes.bytes = (void *) consumer_tag;
	consumer_tag_bytes.len = consumer_tag_len;

	amqp_basic_consume(
		connection->connection_resource->connection_state,
		channel->channel_id,
		amqp_cstring_bytes(queue->name),
		consumer_tag_bytes,					/* Consumer tag */
		(AMQP_NOLOCAL & flags) ? 1 : 0, 	/* No local */
		(AMQP_AUTOACK & flags) ? 1 : 0,		/* no_ack, aka AUTOACK */
		queue->exclusive,
		*arguments
	);

	AMQP_EFREE_ARGUMENTS(arguments);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto array AMQPConsumer::consumeOne(callback);
consume one message and return the value from the callback function
return  boolean
*/
PHP_METHOD(amqp_consumer_class, consumeOne)
{
	zval *id;
	amqp_consumer_object *consumer;
	amqp_queue_object *queue;
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache;
	int function_call_succeeded = 1;
	int read;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_consumer_class_entry) == FAILURE) {
		return;
	}

	/* Pull the queue out */
	consumer = (amqp_consumer_object *)zend_object_store_get_object(id TSRMLS_CC);
	queue = (amqp_queue_object*) zend_object_store_get_object(consumer->queue TSRMLS_CC);

	channel = AMQP_GET_CHANNEL(queue);
	AMQP_VERIFY_CHANNEL(channel, "Could not get channel.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not get connection.");
	
	/* Initialize the message */
	zval *message;
	MAKE_STD_ZVAL(message);

	/* Read the message */
	read = read_message_from_channel(connection->connection_resource->connection_state, message TSRMLS_CC);

	/* Make the callback */
	if (read == AMQP_READ_SUCCESS) {
		zval *params;
		zval *retval_ptr = NULL;

		/* Initialize the return value pointer */
		consumer->callback.fci.retval_ptr_ptr = &retval_ptr;

		/* Build the parameter array */
		MAKE_STD_ZVAL(params);
		array_init(params);

		/* Dump it into the params array */
		add_index_zval(params, 0, message);
		Z_ADDREF_P(message);

		/* Add a pointer to the queue: */
		add_index_zval(params, 1, consumer->queue);
		Z_ADDREF_P(consumer->queue);

		/* Convert everything to be callable */
		zend_fcall_info_args(&consumer->callback.fci, params TSRMLS_CC);

		/* Call the function, and track the return value */
		if (zend_call_function(&consumer->callback.fci, &consumer->callback.fci_cache TSRMLS_CC) == SUCCESS && consumer->callback.fci.retval_ptr_ptr && *consumer->callback.fci.retval_ptr_ptr) {
			COPY_PZVAL_TO_ZVAL(*return_value, *consumer->callback.fci.retval_ptr_ptr);
		}

		/* Check if user land function wants to bail */
		if (EG(exception) || (Z_TYPE_P(return_value) == IS_BOOL && !Z_BVAL_P(return_value))) {
			function_call_succeeded = 0;
		}

		/* Clean up our mess */
		zend_fcall_info_args_clear(&consumer->callback.fci, 1);
		zval_ptr_dtor(&params);
		zval_ptr_dtor(&message);
	} else {
		zval_ptr_dtor(&message);
	}

	if(read != AMQP_READ_ERROR && function_call_succeeded == 1) {
	  RETURN_TRUE;
	} else {
	  RETURN_FALSE;
	}
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
