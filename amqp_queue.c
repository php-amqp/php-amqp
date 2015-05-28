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

/* $Id: amqp_queue.c 327551 2012-09-09 03:49:34Z pdezwart $ */

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
zend_object_handlers amqp_queue_object_handlers;
HashTable *amqp_queue_object_get_debug_info(zval *object, int *is_temp TSRMLS_DC) {
	zval *value;
	HashTable *debug_info;

	/* Get the envelope object from which to read */
	amqp_queue_object *queue = (amqp_queue_object *)zend_object_store_get_object(object TSRMLS_CC);

	/* Let zend clean up for us: */
	*is_temp = 1;

	/* Keep the # 7 matching the number of entries in this table*/
	ALLOC_HASHTABLE(debug_info);
	ZEND_INIT_SYMTABLE_EX(debug_info, 7 + 1, 0);

	/* Start adding values */
	MAKE_STD_ZVAL(value);
	ZVAL_STRINGL(value, queue->name, strlen(queue->name), 1);
	zend_hash_add(debug_info, "queue_name", sizeof("queue_name"), &value, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(value);
	if (queue->consumer_tag_len > 0) {
		ZVAL_STRINGL(value, queue->consumer_tag, strlen(queue->consumer_tag), 1);
	} else {
		ZVAL_NULL(value);
	}

	zend_hash_add(debug_info, "consumer_tag", sizeof("consumer_tag"), &value, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(value);
	ZVAL_BOOL(value, IS_PASSIVE(queue->flags));
	zend_hash_add(debug_info, "passive", sizeof("passive"), &value, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(value);
	ZVAL_BOOL(value, IS_DURABLE(queue->flags));
	zend_hash_add(debug_info, "durable", sizeof("durable"), &value, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(value);
	ZVAL_BOOL(value, IS_EXCLUSIVE(queue->flags));
	zend_hash_add(debug_info, "exclusive", sizeof("exclusive"), &value, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(value);
	ZVAL_BOOL(value, IS_AUTODELETE(queue->flags));
	zend_hash_add(debug_info, "auto_delete", sizeof("auto_delete"), &value, sizeof(zval *), NULL);

	Z_ADDREF_P(queue->arguments);
	zend_hash_add(debug_info, "arguments", sizeof("arguments"), &queue->arguments, sizeof(&queue->arguments), NULL);

	return debug_info;
}
#endif

/* Used in ctor, so must be declated first */
void amqp_queue_dtor(void *object TSRMLS_DC)
{
	amqp_queue_object *queue = (amqp_queue_object*)object;

	/* Destroy the connection object */
	if (queue->channel) {
		zval_ptr_dtor(&queue->channel);
	}

	/* Destroy the arguments storage */
	if (queue->arguments) {
		zval_ptr_dtor(&queue->arguments);
	}

	zend_object_std_dtor(&queue->zo TSRMLS_CC);

	/* Destroy this object */
	efree(object);
}

zend_object_value amqp_queue_ctor(zend_class_entry *ce TSRMLS_DC)
{
	zend_object_value new_value;
	amqp_queue_object* queue = (amqp_queue_object*)emalloc(sizeof(amqp_queue_object));

	memset(queue, 0, sizeof(amqp_queue_object));

	zend_object_std_init(&queue->zo, ce TSRMLS_CC);
	AMQP_OBJECT_PROPERTIES_INIT(queue->zo, ce);

	/* Initialize the arguments array: */
	MAKE_STD_ZVAL(queue->arguments);
	array_init(queue->arguments);

	new_value.handle = zend_objects_store_put(
		queue,
		(zend_objects_store_dtor_t)zend_objects_destroy_object,
		(zend_objects_free_object_storage_t)amqp_queue_dtor,
		NULL TSRMLS_CC
	);

#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3
	memcpy((void *)&amqp_queue_object_handlers, (void *)zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	amqp_queue_object_handlers.get_debug_info = amqp_queue_object_get_debug_info;
	new_value.handlers = &amqp_queue_object_handlers;
#else
	new_value.handlers = zend_get_std_object_handlers();
#endif

	return new_value;
}

void parse_amqp_table(amqp_table_t *table, zval *result)
{
	int i;
	zval *value;

	assert(Z_TYPE_P(result) == IS_ARRAY);

	for (i = 0; i < table->num_entries; i++) {
		MAKE_STD_ZVAL(value);

		amqp_table_entry_t *entry = &(table->entries[i]);
		switch (entry->value.kind) {
			case AMQP_FIELD_KIND_BOOLEAN:
				ZVAL_BOOL(value, entry->value.value.boolean);
				break;
			case AMQP_FIELD_KIND_I8:
				ZVAL_LONG(value, entry->value.value.i8);
				break;
			case AMQP_FIELD_KIND_U8:
				ZVAL_LONG(value, entry->value.value.u8);
				break;
			case AMQP_FIELD_KIND_I16:
				ZVAL_LONG(value, entry->value.value.i16);
				break;
			case AMQP_FIELD_KIND_U16:
				ZVAL_LONG(value, entry->value.value.u16);
				break;
			case AMQP_FIELD_KIND_I32:
				ZVAL_LONG(value, entry->value.value.i32);
				break;
			case AMQP_FIELD_KIND_U32:
				ZVAL_LONG(value, entry->value.value.u32);
				break;
			case AMQP_FIELD_KIND_I64:
				ZVAL_LONG(value, entry->value.value.i64);
				break;
			case AMQP_FIELD_KIND_U64:
				ZVAL_LONG(value, entry->value.value.i64);
				break;
			case AMQP_FIELD_KIND_F32:
				ZVAL_DOUBLE(value, entry->value.value.f32);
				break;
			case AMQP_FIELD_KIND_F64:
				ZVAL_DOUBLE(value, entry->value.value.f64);
				break;
			case AMQP_FIELD_KIND_UTF8:
			case AMQP_FIELD_KIND_BYTES:
				ZVAL_STRINGL(value, entry->value.value.bytes.bytes, entry->value.value.bytes.len, 1);
				break;
			case AMQP_FIELD_KIND_ARRAY:
				{
					int j;
					array_init(value);
					for (j = 0; j < entry->value.value.array.num_entries; ++j) {
						switch (entry->value.value.array.entries[j].kind) {
							case AMQP_FIELD_KIND_UTF8:
								add_next_index_stringl(
									value,
									entry->value.value.array.entries[j].value.bytes.bytes,
									entry->value.value.array.entries[j].value.bytes.len,
									1
								);
								break;
							case AMQP_FIELD_KIND_TABLE:
								{
									zval *subtable;
									MAKE_STD_ZVAL(subtable);
									array_init(subtable);
									parse_amqp_table(
										&(entry->value.value.array.entries[j].value.table),
										subtable
									);
									add_next_index_zval(value, subtable);
								}
								break;
						}
					}
				}
				break;
			case AMQP_FIELD_KIND_TABLE:
			    array_init(value);
				parse_amqp_table(&(entry->value.value.table), value);
				break;
			case AMQP_FIELD_KIND_TIMESTAMP:
				ZVAL_DOUBLE(value, entry->value.value.u64);
				break;
			case AMQP_FIELD_KIND_VOID:
			case AMQP_FIELD_KIND_DECIMAL:
			default:
				ZVAL_NULL(value);
				break;
		}

		if (Z_TYPE_P(value) != IS_NULL) {
			char *key = estrndup(entry->key.bytes, entry->key.len);
			add_assoc_zval(result, key, value);
			efree(key);
		} else {
			zval_dtor(value);
		}
	}
	return;
}

void convert_amqp_envelope_to_zval(amqp_envelope_t *amqp_envelope, zval *envelopeZval TSRMLS_DC)
{
	amqp_envelope_object *envelope;

	/* Build the envelope */
	object_init_ex(envelopeZval, amqp_envelope_class_entry);
	envelope = (amqp_envelope_object *)zend_object_store_get_object(envelopeZval TSRMLS_CC);

	AMQP_SET_STR_PROPERTY(envelope->routing_key,	amqp_envelope->routing_key.bytes, amqp_envelope->routing_key.len);
	AMQP_SET_STR_PROPERTY(envelope->exchange_name,	amqp_envelope->exchange.bytes, amqp_envelope->exchange.len);
	AMQP_SET_LONG_PROPERTY(envelope->delivery_tag,	amqp_envelope->delivery_tag);
	AMQP_SET_BOOL_PROPERTY(envelope->is_redelivery,	amqp_envelope->redelivered);

	amqp_basic_properties_t *p = &amqp_envelope->message.properties;
	amqp_message_t    *message = &amqp_envelope->message;

	if (p->_flags & AMQP_BASIC_CONTENT_TYPE_FLAG) {
		AMQP_SET_STR_PROPERTY(envelope->content_type, p->content_type.bytes, p->content_type.len);
	}

	if (p->_flags & AMQP_BASIC_CONTENT_ENCODING_FLAG) {
		AMQP_SET_STR_PROPERTY(envelope->content_encoding, p->content_encoding.bytes, p->content_encoding.len);
	}

	if (p->_flags & AMQP_BASIC_TYPE_FLAG) {
		AMQP_SET_STR_PROPERTY(envelope->type, p->type.bytes, p->type.len);
	}

	if (p->_flags & AMQP_BASIC_TIMESTAMP_FLAG) {
		AMQP_SET_LONG_PROPERTY(envelope->timestamp, p->timestamp);
	}

	if (p->_flags & AMQP_BASIC_DELIVERY_MODE_FLAG) {
		AMQP_SET_LONG_PROPERTY(envelope->delivery_mode, p->delivery_mode);
	}

	if (p->_flags & AMQP_BASIC_PRIORITY_FLAG) {
		AMQP_SET_LONG_PROPERTY(envelope->priority, p->priority);
	}

	if (p->_flags & AMQP_BASIC_EXPIRATION_FLAG) {
		AMQP_SET_STR_PROPERTY(envelope->expiration, p->expiration.bytes, p->expiration.len);
	}

	if (p->_flags & AMQP_BASIC_USER_ID_FLAG) {
		AMQP_SET_STR_PROPERTY(envelope->user_id, p->user_id.bytes, p->user_id.len);
	}

	if (p->_flags & AMQP_BASIC_APP_ID_FLAG) {
		AMQP_SET_STR_PROPERTY(envelope->app_id, p->app_id.bytes, p->app_id.len);
	}

	if (p->_flags & AMQP_BASIC_MESSAGE_ID_FLAG) {
		AMQP_SET_STR_PROPERTY(envelope->message_id, p->message_id.bytes, p->message_id.len);
	}

	if (p->_flags & AMQP_BASIC_REPLY_TO_FLAG) {
		AMQP_SET_STR_PROPERTY(envelope->reply_to, p->reply_to.bytes, p->reply_to.len);
	}

	if (p->_flags & AMQP_BASIC_CORRELATION_ID_FLAG) {
		AMQP_SET_STR_PROPERTY(envelope->correlation_id, p->correlation_id.bytes, p->correlation_id.len);
	}

	if (p->_flags & AMQP_BASIC_HEADERS_FLAG) {
		parse_amqp_table(&(p->headers), envelope->headers);
	}

	envelope->body     = estrndup(message->body.bytes, message->body.len);
	envelope->body_len = message->body.len;
}

/* {{{ proto AMQPQueue::__construct(AMQPChannel channel)
AMQPQueue constructor
*/
PHP_METHOD(amqp_queue_class, __construct)
{
	zval *id;
	zval *channelObj = NULL;
	amqp_queue_object *queue;
	amqp_channel_object *channel;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "OO", &id, amqp_queue_class_entry, &channelObj, amqp_channel_class_entry) == FAILURE) {
		zend_throw_exception(amqp_queue_exception_class_entry, "Parameter must be an instance of AMQPChannel.", 0 TSRMLS_CC);
		RETURN_NULL();
	}

	/* Store the connection object for later */
	queue = (amqp_queue_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Store the channel object */
	queue->channel = channelObj;

	/* Increment the ref count */
	Z_ADDREF_P(channelObj);

	/* Pull the channel out */
	channel = AMQP_GET_CHANNEL(queue);

	/* Check that the given connection has a channel, before trying to pull the connection off the stack */
	AMQP_VERIFY_CHANNEL(channel, "Could not construct queue.");

	/* By default, the auto_delete flag should be set */
	queue->flags = AMQP_AUTODELETE;
}
/* }}} */


/* {{{ proto AMQPQueue::getName()
Get the queue name */
PHP_METHOD(amqp_queue_class, getName)
{
	zval *id;
	amqp_queue_object *queue;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_queue_class_entry) == FAILURE) {
		return;
	}

	queue = (amqp_queue_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Check if there is a name to be had: */
	if (queue->name_len) {
		RETURN_STRING(queue->name, 1);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */


/* {{{ proto AMQPQueue::setName(string name)
Set the queue name */
PHP_METHOD(amqp_queue_class, setName)
{
	zval *id;
	amqp_queue_object *queue;
	char *name = NULL;
	int name_len = 0;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os", &id, amqp_queue_class_entry, &name, &name_len) == FAILURE) {
		return;
	}

	/* Pull the queue off the object store */
	queue = (amqp_queue_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Verify that the name is not null and not an empty string */
	if (name_len < 1 || name_len > 255) {
		zend_throw_exception(amqp_queue_exception_class_entry, "Invalid queue name given, must be between 1 and 255 characters long.", 0 TSRMLS_CC);
		return;
	}

	/* Set the queue name */
	AMQP_SET_NAME(queue, name);
}
/* }}} */



/* {{{ proto AMQPQueue::getFlags()
Get the queue parameters */
PHP_METHOD(amqp_queue_class, getFlags)
{
	zval *id;
	amqp_queue_object *queue;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_queue_class_entry) == FAILURE) {
		return;
	}

	queue = (amqp_queue_object *)zend_object_store_get_object(id TSRMLS_CC);

	RETURN_LONG(queue->flags);
}
/* }}} */


/* {{{ proto AMQPQueue::setFlags(long bitmask)
Set the queue parameters */
PHP_METHOD(amqp_queue_class, setFlags)
{
	zval *id;
	amqp_queue_object *queue;
	long flagBitmask;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ol", &id, amqp_queue_class_entry, &flagBitmask) == FAILURE) {
		return;
	}

	/* Pull the queue off the object store */
	queue = (amqp_queue_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Set the flags based on the bitmask we were given */
	queue->flags = flagBitmask ? flagBitmask & PHP_AMQP_QUEUE_FLAGS : flagBitmask;

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto AMQPQueue::getArgument(string key)
Get the queue argument referenced by key */
PHP_METHOD(amqp_queue_class, getArgument)
{
	zval *id;
	zval **tmp;
	amqp_queue_object *queue;
	char *key;
	int key_len;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os", &id, amqp_queue_class_entry, &key, &key_len) == FAILURE) {
		return;
	}

	queue = (amqp_queue_object *)zend_object_store_get_object(id TSRMLS_CC);

	if (zend_hash_find(Z_ARRVAL_P(queue->arguments), key, key_len + 1, (void **)&tmp) == FAILURE) {
		RETURN_FALSE;
	}

	*return_value = **tmp;
	zval_copy_ctor(return_value);
	INIT_PZVAL(return_value);

}
/* }}} */

/* {{{ proto AMQPQueue::getArguments
Get the queue arguments */
PHP_METHOD(amqp_queue_class, getArguments)
{
	zval *id;
	amqp_queue_object *queue;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_queue_class_entry) == FAILURE) {
		return;
	}

	queue = (amqp_queue_object *)zend_object_store_get_object(id TSRMLS_CC);

	zval_dtor(return_value);
	MAKE_COPY_ZVAL(&queue->arguments, return_value);
}
/* }}} */

/* {{{ proto AMQPQueue::setArguments(array args)
Overwrite all queue arguments with given args */
PHP_METHOD(amqp_queue_class, setArguments)
{
	zval *id, *arguments;
	amqp_queue_object *queue;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Oa", &id, amqp_queue_class_entry, &arguments) == FAILURE) {
		return;
	}

	/* Pull the queue off the object store */
	queue = (amqp_queue_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Destroy the arguments storage */
	if (queue->arguments) {
		zval_ptr_dtor(&queue->arguments);
	}

	MAKE_COPY_ZVAL(&arguments, queue->arguments);

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto AMQPQueue::setArgument(key, value)
Get the queue name */
PHP_METHOD(amqp_queue_class, setArgument)
{
	zval *id, *value;
	amqp_queue_object *queue;
	char *key;
	int key_len;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Osz", &id, amqp_queue_class_entry, &key, &key_len, &value) == FAILURE) {
		return;
	}

	/* Pull the queue off the object store */
	queue = (amqp_queue_object *)zend_object_store_get_object(id TSRMLS_CC);

	switch (Z_TYPE_P(value)) {
		case IS_NULL:
			zend_hash_del_key_or_index(Z_ARRVAL_P(queue->arguments), key, key_len + 1, 0, HASH_DEL_KEY);
			break;
		case IS_BOOL:
		case IS_LONG:
		case IS_DOUBLE:
		case IS_STRING:
			add_assoc_zval(queue->arguments, key, value);
			Z_ADDREF_P(value);
			break;
		default:
			zend_throw_exception(amqp_queue_exception_class_entry, "The value parameter must be of type NULL, int, double or string.", 0 TSRMLS_CC);
			return;
	}

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto int AMQPQueue::declareQueue();
declare queue
*/
PHP_METHOD(amqp_queue_class, declareQueue)
{
	zval *id;
	amqp_queue_object *queue;
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	char *name = "";
	amqp_table_t *arguments;
	long message_count;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_queue_class_entry) == FAILURE) {
		return;
	}

	queue = (amqp_queue_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Make sure we have a queue name, even if its empty: */
	if (queue->name_len < 1) {
		AMQP_SET_NAME(queue, name);
	}

	AMQP_ASSIGN_CHANNEL(channel, queue);
	AMQP_VERIFY_CHANNEL(channel, "Could not declare queue.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not declare queue.");

	arguments = convert_zval_to_amqp_table(queue->arguments TSRMLS_CC);

	amqp_queue_declare_ok_t *r = amqp_queue_declare(
		connection->connection_resource->connection_state,
		channel->channel_id,
		amqp_cstring_bytes(queue->name),
		IS_PASSIVE(queue->flags),
		IS_DURABLE(queue->flags),
		IS_EXCLUSIVE(queue->flags),
		IS_AUTODELETE(queue->flags),
		*arguments
	);

	php_amqp_free_amqp_table(arguments);

	if (!r) {
		amqp_rpc_reply_t res = amqp_get_rpc_reply(connection->connection_resource->connection_state);

		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_queue_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		PHP_AMQP_DESTROY_ERROR_MESSAGE();
		return;
	}

	message_count = r->message_count;

	/* Set the queue name, in case it is an autogenerated queue name */
	name = stringify_bytes(r->queue);
	AMQP_SET_NAME(queue, name);
	efree(name);

	php_amqp_maybe_release_buffers_on_channel(connection, channel);

	RETURN_LONG(message_count);
}
/* }}} */


/* {{{ proto int AMQPQueue::bind(string exchangeName, [string routingKey, array arguments]);
bind queue to exchange by routing key
*/
PHP_METHOD(amqp_queue_class, bind)
{
	zval *id, *zvalArguments = NULL;

	amqp_queue_object *queue;
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	char *exchange_name;
	int   exchange_name_len;

	char *keyname     = NULL;
	int   keyname_len = 0;

	amqp_table_t *arguments;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os|sa", &id, amqp_queue_class_entry, &exchange_name, &exchange_name_len, &keyname, &keyname_len, &zvalArguments) == FAILURE) {
		return;
	}

	queue = (amqp_queue_object *)zend_object_store_get_object(id TSRMLS_CC);

	channel = AMQP_GET_CHANNEL(queue);
	AMQP_VERIFY_CHANNEL(channel, "Could not bind queue.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not bind queue.");

	if (zvalArguments) {
		arguments = convert_zval_to_amqp_table(zvalArguments TSRMLS_CC);
	}

	amqp_queue_bind(
		connection->connection_resource->connection_state,
		channel->channel_id,
		amqp_cstring_bytes(queue->name),
		(exchange_name_len > 0 ? amqp_cstring_bytes(exchange_name) : amqp_empty_bytes),
		(keyname_len > 0 ? amqp_cstring_bytes(keyname) : amqp_empty_bytes),
		(zvalArguments ? *arguments : amqp_empty_table)
	);

	if (zvalArguments) {
		php_amqp_free_amqp_table(arguments);
	}

	amqp_rpc_reply_t res = amqp_get_rpc_reply(connection->connection_resource->connection_state);

	if (res.reply_type != AMQP_RESPONSE_NORMAL) {
		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_queue_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		PHP_AMQP_DESTROY_ERROR_MESSAGE();
		return;
	}

	php_amqp_maybe_release_buffers_on_channel(connection, channel);

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto int AMQPQueue::get([bit flags=AMQP_NOPARAM]);
read messages from queue
return array (messages)
*/
PHP_METHOD(amqp_queue_class, get)
{
	zval *id;
	amqp_queue_object *queue;
	amqp_channel_object *channel;
	amqp_connection_object *connection;
	zval *message;

	long flags = INI_INT("amqp.auto_ack") ? AMQP_AUTOACK : AMQP_NOPARAM;

	/* Parse out the method parameters */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O|l", &id, amqp_queue_class_entry, &flags) == FAILURE) {
		return;
	}

	queue = (amqp_queue_object *)zend_object_store_get_object(id TSRMLS_CC);

	channel = AMQP_GET_CHANNEL(queue);
	AMQP_VERIFY_CHANNEL(channel, "Could not get messages from queue.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not get messages from queue.");

	amqp_rpc_reply_t res = amqp_basic_get(
		connection->connection_resource->connection_state,
		channel->channel_id,
		amqp_cstring_bytes(queue->name),
		(AMQP_AUTOACK & flags) ? 1 : 0
	);

	if (res.reply_type != AMQP_RESPONSE_NORMAL) {
		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_queue_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		PHP_AMQP_DESTROY_ERROR_MESSAGE();
		return;
	}

	if (AMQP_BASIC_GET_EMPTY_METHOD == res.reply.id) {
		RETURN_FALSE;
	}

	assert(AMQP_BASIC_GET_OK_METHOD == res.reply.id);

	/* Fill the envelope from response */
	amqp_basic_get_ok_t *get_ok_method = res.reply.decoded;

	amqp_envelope_t envelope;

	envelope.channel      = channel->channel_id;
	envelope.consumer_tag = amqp_empty_bytes;
	envelope.delivery_tag = get_ok_method->delivery_tag;
	envelope.redelivered  = get_ok_method->redelivered;
	envelope.exchange     = amqp_bytes_malloc_dup(get_ok_method->exchange);
	envelope.routing_key  = amqp_bytes_malloc_dup(get_ok_method->routing_key);

	php_amqp_maybe_release_buffers_on_channel(connection, channel);

  	res = amqp_read_message(
		connection->connection_resource->connection_state,
		channel->channel_id,
		&envelope.message,
		0
	);

	if (AMQP_RESPONSE_NORMAL != res.reply_type) {
		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_queue_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		amqp_destroy_envelope(&envelope);
		PHP_AMQP_DESTROY_ERROR_MESSAGE();
		return;
	}

	MAKE_STD_ZVAL(message);
	convert_amqp_envelope_to_zval(&envelope, message TSRMLS_CC);

	php_amqp_maybe_release_buffers_on_channel(connection, channel);
	amqp_destroy_envelope(&envelope);

	COPY_PZVAL_TO_ZVAL(*return_value, message);
}
/* }}} */


/* {{{ proto array AMQPQueue::consume([callback, flags = <bitmask>, consumer_tag]);
consume the message
*/
PHP_METHOD(amqp_queue_class, consume)
{
	zval *id;
	amqp_queue_object *queue;
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	zend_fcall_info fci = empty_fcall_info;
	zend_fcall_info_cache fci_cache = empty_fcall_info_cache;

	amqp_table_t *arguments;

	char *consumer_tag;
	int consumer_tag_len = 0;

	long flags = INI_INT("amqp.auto_ack") ? AMQP_AUTOACK : AMQP_NOPARAM;

	int call_result;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O|f!ls", &id, amqp_queue_class_entry, &fci,
									 &fci_cache, &flags, &consumer_tag, &consumer_tag_len) == FAILURE) {
		return;
	}

	/* Pull the queue out */
	queue = (amqp_queue_object *) zend_object_store_get_object(id TSRMLS_CC);

	channel = AMQP_GET_CHANNEL(queue);
	AMQP_VERIFY_CHANNEL(channel, "Could not get channel.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not get connection.");

	if (!(AMQP_JUST_CONSUME & flags)) {
		/* Setup the consume */
		arguments = convert_zval_to_amqp_table(queue->arguments TSRMLS_CC);

		amqp_basic_consume_ok_t *r = amqp_basic_consume(
				connection->connection_resource->connection_state,
				channel->channel_id,
				amqp_cstring_bytes(queue->name),
				(consumer_tag_len > 0 ? amqp_cstring_bytes(consumer_tag) : amqp_empty_bytes), /* Consumer tag */
				(AMQP_NOLOCAL & flags) ? 1 : 0, /* No local */
				(AMQP_AUTOACK & flags) ? 1 : 0,    /* no_ack, aka AUTOACK */
				IS_EXCLUSIVE(queue->flags),
				*arguments
		);

		php_amqp_free_amqp_table(arguments);

		if (!r) {
			amqp_rpc_reply_t res = amqp_get_rpc_reply(connection->connection_resource->connection_state);

			PHP_AMQP_INIT_ERROR_MESSAGE();

			php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

			zend_throw_exception(amqp_queue_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
			php_amqp_maybe_release_buffers_on_channel(connection, channel);

			PHP_AMQP_DESTROY_ERROR_MESSAGE();
			return;
		}

		/* Set the consumer tag name, in case it is an autogenerated consumer tag name */
		AMQP_SET_STR_PROPERTY(queue->consumer_tag, r->consumer_tag.bytes, r->consumer_tag.len);
		queue->consumer_tag_len = r->consumer_tag.len;
	}

	if (!ZEND_FCI_INITIALIZED(fci)) {
		/* Callback not set, we have nothing to do - real consuming may happens later */
		return;
	}

	struct timeval tv = {0};
	struct timeval *tv_ptr = &tv;

	if (connection->read_timeout > 0) {
		tv.tv_sec  = (long int) connection->read_timeout;
		tv.tv_usec = (long int) ((connection->read_timeout - tv.tv_sec) * 1000000);
	} else {
		tv_ptr = NULL;
	}

	while(1) {
		/* Initialize the message */
		zval *message;

		amqp_envelope_t envelope;

		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		amqp_rpc_reply_t res = amqp_consume_message(connection->connection_resource->connection_state, &envelope, tv_ptr, 0);

		if (AMQP_RESPONSE_LIBRARY_EXCEPTION == res.reply_type && AMQP_STATUS_TIMEOUT == res.library_error) {
			PHP_AMQP_INIT_ERROR_MESSAGE();

			zend_throw_exception(amqp_queue_exception_class_entry, "Consumer timeout exceed", 0 TSRMLS_CC);

			amqp_destroy_envelope(&envelope);
			php_amqp_maybe_release_buffers_on_channel(connection, channel);

			PHP_AMQP_DESTROY_ERROR_MESSAGE();
			return;
		}

		if (AMQP_RESPONSE_NORMAL != res.reply_type) {
			PHP_AMQP_INIT_ERROR_MESSAGE();

			php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

			php_amqp_zend_throw_exception(res, amqp_queue_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);

			amqp_destroy_envelope(&envelope);
			php_amqp_maybe_release_buffers_on_channel(connection, channel);

			PHP_AMQP_DESTROY_ERROR_MESSAGE();
			return;
		}

		MAKE_STD_ZVAL(message);
		convert_amqp_envelope_to_zval(&envelope, message TSRMLS_CC);

		amqp_destroy_envelope(&envelope);

		/* Make the callback */
		zval *params;
		zval *retval_ptr = NULL;

		/* Build the parameter array */
		MAKE_STD_ZVAL(params);
		array_init(params);

		/* Dump it into the params array */
		add_index_zval(params, 0, message);
		Z_ADDREF_P(message);

		/* Add a pointer to the queue: */
		add_index_zval(params, 1, id);
		Z_ADDREF_P(id);

		/* Convert everything to be callable */
		zend_fcall_info_args(&fci, params TSRMLS_CC);
		/* Initialize the return value pointer */
		fci.retval_ptr_ptr = &retval_ptr;

		/* Call the function, and track the return value */
		if (zend_call_function(&fci, &fci_cache TSRMLS_CC) == SUCCESS && fci.retval_ptr_ptr && *fci.retval_ptr_ptr) {
			COPY_PZVAL_TO_ZVAL(*return_value, *fci.retval_ptr_ptr);
  		}

		/* Clean up our mess */
		zend_fcall_info_args_clear(&fci, 1);
		zval_ptr_dtor(&params);
		zval_ptr_dtor(&message);

		/* Check if user land function wants to bail */
		if (EG(exception) || (Z_TYPE_P(return_value) == IS_BOOL && !Z_BVAL_P(return_value))) {
			break;
		}
	}

	php_amqp_maybe_release_buffers_on_channel(connection, channel);
	return;
}
/* }}} */


/* {{{ proto int AMQPQueue::ack(long deliveryTag, [bit flags=AMQP_NOPARAM]);
	acknowledge the message
*/
PHP_METHOD(amqp_queue_class, ack)
{
	zval *id;
	amqp_queue_object *queue;
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	long deliveryTag = 0;
	long flags = AMQP_NOPARAM;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ol|l", &id, amqp_queue_class_entry, &deliveryTag, &flags ) == FAILURE) {
		return;
	}

	queue = (amqp_queue_object *)zend_object_store_get_object(id TSRMLS_CC);

	channel = AMQP_GET_CHANNEL(queue);
	AMQP_VERIFY_CHANNEL(channel, "Could not ack message.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not ack message.");

	/* NOTE: basic.ack is asynchronous and thus will not indicate failure if something goes wrong on the broker */
	int status = amqp_basic_ack(
		connection->connection_resource->connection_state,
		channel->channel_id,
		deliveryTag,
		(AMQP_MULTIPLE & flags) ? 1 : 0
	);

	if (status != AMQP_STATUS_OK) {
		/* Emulate library error */
		amqp_rpc_reply_t res;
		res.reply_type 	  = AMQP_RESPONSE_LIBRARY_EXCEPTION;
		res.library_error = status;

		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_queue_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		PHP_AMQP_DESTROY_ERROR_MESSAGE();
		return;
	}

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto int AMQPQueue::nack(long deliveryTag, [bit flags=AMQP_NOPARAM]);
	acknowledge the message
*/
PHP_METHOD(amqp_queue_class, nack)
{
	zval *id;
	amqp_queue_object *queue;
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	long deliveryTag = 0;
	long flags = AMQP_NOPARAM;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ol|l", &id, amqp_queue_class_entry, &deliveryTag, &flags ) == FAILURE) {
		return;
	}

	queue = (amqp_queue_object *)zend_object_store_get_object(id TSRMLS_CC);

	channel = AMQP_GET_CHANNEL(queue);
	AMQP_VERIFY_CHANNEL(channel, "Could not nack message.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not nack message.");

	/* NOTE: basic.nack is asynchronous and thus will not indicate failure if something goes wrong on the broker */
	int status = amqp_basic_nack(
		connection->connection_resource->connection_state,
		channel->channel_id,
		deliveryTag,
		(AMQP_MULTIPLE & flags) ? 1 : 0,
		(AMQP_REQUEUE & flags) ? 1 : 0
	);

	if (status != AMQP_STATUS_OK) {
		/* Emulate library error */
		amqp_rpc_reply_t res;
		res.reply_type 	  = AMQP_RESPONSE_LIBRARY_EXCEPTION;
		res.library_error = status;

		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_queue_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		PHP_AMQP_DESTROY_ERROR_MESSAGE();
		return;
	}

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto int AMQPQueue::reject(long deliveryTag, [bit flags=AMQP_NOPARAM]);
	acknowledge the message
*/
PHP_METHOD(amqp_queue_class, reject)
{
	zval *id;
	amqp_queue_object *queue;
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	long deliveryTag = 0;
	long flags = AMQP_NOPARAM;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ol|l", &id, amqp_queue_class_entry, &deliveryTag, &flags ) == FAILURE) {
		return;
	}

	queue = (amqp_queue_object *)zend_object_store_get_object(id TSRMLS_CC);

	channel = AMQP_GET_CHANNEL(queue);
	AMQP_VERIFY_CHANNEL(channel, "Could not reject message.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not reject message.");

	/* NOTE: basic.reject is asynchronous and thus will not indicate failure if something goes wrong on the broker */
	int status = amqp_basic_reject(
		connection->connection_resource->connection_state,
		channel->channel_id,
		deliveryTag,
		(AMQP_REQUEUE & flags) ? 1 : 0
	);

	if (status != AMQP_STATUS_OK) {
		/* Emulate library error */
		amqp_rpc_reply_t res;
		res.reply_type 	  = AMQP_RESPONSE_LIBRARY_EXCEPTION;
		res.library_error = status;

		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_queue_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		PHP_AMQP_DESTROY_ERROR_MESSAGE();
		return;
	}

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto int AMQPQueue::purge();
purge queue
*/
PHP_METHOD(amqp_queue_class, purge)
{
	zval *id;
	amqp_queue_object *queue;
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_queue_class_entry) == FAILURE) {
		return;
	}

	queue = (amqp_queue_object *)zend_object_store_get_object(id TSRMLS_CC);

	channel = AMQP_GET_CHANNEL(queue);
	AMQP_VERIFY_CHANNEL(channel, "Could not purge queue.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not purge queue.");

	amqp_queue_purge_ok_t *r = amqp_queue_purge(
		connection->connection_resource->connection_state,
		channel->channel_id,
		amqp_cstring_bytes(queue->name)
	);

	if (!r) {
		amqp_rpc_reply_t res = amqp_get_rpc_reply(connection->connection_resource->connection_state);

		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_queue_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		PHP_AMQP_DESTROY_ERROR_MESSAGE();
		return;
	}

	/* long message_count = r->message_count; */

	php_amqp_maybe_release_buffers_on_channel(connection, channel);

	/* RETURN_LONG(message_count) */;
	RETURN_TRUE;
}
/* }}} */


/* {{{ proto int AMQPQueue::cancel([string consumer_tag]);
cancel queue to consumer
*/
PHP_METHOD(amqp_queue_class, cancel)
{
	zval *id;
	amqp_queue_object *queue;
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	char *consumer_tag     = NULL;
	int   consumer_tag_len = 0;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O|s", &id, amqp_queue_class_entry, &consumer_tag, &consumer_tag_len) == FAILURE) {
		return;
	}

	queue = (amqp_queue_object *)zend_object_store_get_object(id TSRMLS_CC);

	channel = AMQP_GET_CHANNEL(queue);
	AMQP_VERIFY_CHANNEL(channel, "Could not cancel queue.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not cancel queue.");

    if (!consumer_tag_len && !queue->consumer_tag_len) {
        return;
    }

	amqp_basic_cancel_ok_t *r = amqp_basic_cancel(
		connection->connection_resource->connection_state,
		channel->channel_id,
		consumer_tag_len > 0 ? amqp_cstring_bytes(consumer_tag) : amqp_cstring_bytes(queue->consumer_tag)
	);

	if (!r) {
		amqp_rpc_reply_t res = amqp_get_rpc_reply(connection->connection_resource->connection_state);

		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_queue_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		PHP_AMQP_DESTROY_ERROR_MESSAGE();
		return;
	}

	if (!consumer_tag_len || strcmp(consumer_tag, queue->consumer_tag) != 0) {
		memset(queue->consumer_tag, 0, sizeof(queue->consumer_tag));
		queue->consumer_tag_len = 0;
	}

	php_amqp_maybe_release_buffers_on_channel(connection, channel);

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto int AMQPQueue::unbind(string exchangeName, [string routingKey, array arguments]);
unbind queue from exchange
*/
PHP_METHOD(amqp_queue_class, unbind)
{
	zval *id, *zvalArguments = NULL;
	amqp_queue_object *queue;
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	char *exchange_name;
	int   exchange_name_len;
	char *keyname     = NULL;
	int   keyname_len = 0;

	amqp_table_t *arguments;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os|sa", &id, amqp_queue_class_entry, &exchange_name, &exchange_name_len, &keyname, &keyname_len, &zvalArguments) == FAILURE) {
		return;
	}

	queue = (amqp_queue_object *)zend_object_store_get_object(id TSRMLS_CC);

	channel = AMQP_GET_CHANNEL(queue);
	AMQP_VERIFY_CHANNEL(channel, "Could not unbind queue.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not unbind queue.");

	if (zvalArguments) {
		arguments = convert_zval_to_amqp_table(zvalArguments TSRMLS_CC);
	}

	amqp_queue_unbind(
		connection->connection_resource->connection_state,
		channel->channel_id,
		amqp_cstring_bytes(queue->name),
		(exchange_name_len > 0 ? amqp_cstring_bytes(exchange_name) : amqp_empty_bytes),
		(keyname_len > 0 ? amqp_cstring_bytes(keyname) : amqp_empty_bytes),
		(zvalArguments ? *arguments : amqp_empty_table)
	);

	if (zvalArguments) {
		php_amqp_free_amqp_table(arguments);
	}

	amqp_rpc_reply_t res = amqp_get_rpc_reply(connection->connection_resource->connection_state);

	if (res.reply_type != AMQP_RESPONSE_NORMAL) {
		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_queue_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		PHP_AMQP_DESTROY_ERROR_MESSAGE();
		return;
	}

	php_amqp_maybe_release_buffers_on_channel(connection, channel);

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto int AMQPQueue::delete([long flags = AMQP_NOPARAM]]);
delete queue and return the number of messages deleted in it
*/
PHP_METHOD(amqp_queue_class, delete)
{
	zval *id;
	amqp_queue_object *queue;
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	long flags = AMQP_NOPARAM;

	long message_count;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O|l", &id, amqp_queue_class_entry, &flags) == FAILURE) {
		return;
	}

	queue = (amqp_queue_object *)zend_object_store_get_object(id TSRMLS_CC);

	channel = AMQP_GET_CHANNEL(queue);
	AMQP_VERIFY_CHANNEL(channel, "Could not delete queue.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not delete queue.");

	amqp_queue_delete_ok_t * r = amqp_queue_delete(
		connection->connection_resource->connection_state,
		channel->channel_id,
		amqp_cstring_bytes(queue->name),
		(AMQP_IFUNUSED & flags) ? 1 : 0,
		(AMQP_IFEMPTY & flags) ? 1 : 0
	);

	if (!r) {
		amqp_rpc_reply_t res = amqp_get_rpc_reply(connection->connection_resource->connection_state);

		PHP_AMQP_INIT_ERROR_MESSAGE();

		php_amqp_error(res, PHP_AMQP_ERROR_MESSAGE_PTR, connection, channel TSRMLS_CC);

		php_amqp_zend_throw_exception(res, amqp_queue_exception_class_entry, PHP_AMQP_ERROR_MESSAGE, 0 TSRMLS_CC);
		php_amqp_maybe_release_buffers_on_channel(connection, channel);

		PHP_AMQP_DESTROY_ERROR_MESSAGE();
		return;
	}

	message_count = r->message_count;

	php_amqp_maybe_release_buffers_on_channel(connection, channel);

	RETURN_LONG(message_count);
}
/* }}} */

/* {{{ proto AMQPChannel::getChannel()
Get the AMQPChannel object in use */
PHP_METHOD(amqp_queue_class, getChannel)
{
	zval *id;
	amqp_queue_object *queue;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O|l", &id, amqp_queue_class_entry) == FAILURE) {
		return;
	}

	queue = (amqp_queue_object *)zend_object_store_get_object(id TSRMLS_CC);

	RETURN_ZVAL(queue->channel, 1, 0);
}
/* }}} */

/* {{{ proto AMQPChannel::getConnection()
Get the AMQPConnection object in use */
PHP_METHOD(amqp_queue_class, getConnection)
{
	zval *id;
	amqp_queue_object *queue;
	amqp_channel_object *channel;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O|l", &id, amqp_queue_class_entry) == FAILURE) {
		return;
	}

	queue = (amqp_queue_object *)zend_object_store_get_object(id TSRMLS_CC);
	channel = AMQP_GET_CHANNEL(queue);

	RETURN_ZVAL(channel->connection, 1, 0);
}
/* }}} */

/* {{{ proto string AMQPChannel::getConsumerTag()
Get latest consumer tag*/
PHP_METHOD(amqp_queue_class, getConsumerTag)
{
	amqp_queue_object *queue;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	queue = (amqp_queue_object *)zend_object_store_get_object(getThis() TSRMLS_CC);

	if (queue->consumer_tag_len > 0) {
		RETURN_STRINGL(queue->consumer_tag, strlen(queue->consumer_tag), 1);
	}

	RETURN_NULL();
}
/* }}} */


/*
*Local variables:
*tab-width: 4
*tabstop: 4
*c-basic-offset: 4
*End:
*vim600: noet sw=4 ts=4 fdm=marker
*vim<600: noet sw=4 ts=4
*/
