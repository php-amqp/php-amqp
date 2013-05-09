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
	ZVAL_STRINGL(value, queue->consumer_tag, strlen(queue->consumer_tag), 1);
	zend_hash_add(debug_info, "consumer_tag", sizeof("consumer_tag"), &value, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(value);
	ZVAL_BOOL(value, queue->passive);
	zend_hash_add(debug_info, "passive", sizeof("passive"), &value, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(value);
	ZVAL_BOOL(value, queue->durable);
	zend_hash_add(debug_info, "durable", sizeof("durable"), &value, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(value);
	ZVAL_BOOL(value, queue->exclusive);
	zend_hash_add(debug_info, "exclusive", sizeof("exclusive"), &value, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(value);
	ZVAL_BOOL(value, queue->auto_delete);
	zend_hash_add(debug_info, "auto_delete", sizeof("auto_delete"), &value, sizeof(zval *), NULL);

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
	array_init(result);

	for (i = 0; i < table->num_entries; i++) {
		zval *value;
		amqp_table_entry_t *entry = &(table->entries[i]);
		MAKE_STD_ZVAL(value);
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

/*
Read a message that is pending on a channel. A call to basic.get or basic.consume must preceed this call.
*/
int read_message_from_channel(amqp_connection_state_t connection, zval *envelopeZval TSRMLS_DC) {
	size_t body_received = 0;
	size_t body_target = 0;
	char *message_body_buffer = NULL;
	amqp_frame_t frame;
	int result = 0;
	amqp_envelope_object *envelope;
	amqp_basic_properties_t * p;

	/* Build the envelope */
	object_init_ex(envelopeZval, amqp_envelope_class_entry);
	envelope = (amqp_envelope_object *)zend_object_store_get_object(envelopeZval TSRMLS_CC);

	/* The "standard" for this is to continuously loop over frames in the pipeline until an entire message is read */
	while (1) {
		/* Release pending buffers for ingestion */
		amqp_maybe_release_buffers(connection);
		result = amqp_simple_wait_frame(connection, &frame);

		/* Check that the basic read from frame did not fail */
		if (result < 0) {
			zend_throw_exception(amqp_connection_exception_class_entry, amqp_error_string(-result), -result TSRMLS_CC);
			return AMQP_READ_ERROR;
		}

		if (frame.payload.method.id == AMQP_CHANNEL_CLOSE_OK_METHOD) {
			amqp_channel_close_t *err = (amqp_channel_close_t *)frame.payload.method.decoded;
			char str[256];
			char **pstr = (char **)&str;
			if (err) {
				spprintf(pstr, 0, "Server error: %d", (int)err->reply_code);
			} else {
				spprintf(pstr, 0, "Unknown server error occurred.");
			}

			zend_throw_exception(amqp_queue_exception_class_entry, *pstr, 0 TSRMLS_CC);
			return AMQP_READ_ERROR;
		}

		/* Verify that we have a method frame first  */
		if (frame.frame_type != AMQP_FRAME_METHOD) {
			continue;
		}

		if (frame.payload.method.id == AMQP_BASIC_GET_OK_METHOD) {
			/* get message metadata */
			amqp_basic_get_ok_t *delivery = (amqp_basic_get_ok_t *) frame.payload.method.decoded;

			AMQP_SET_STR_PROPERTY(envelope->routing_key,	delivery->routing_key.bytes, delivery->routing_key.len);
			AMQP_SET_STR_PROPERTY(envelope->exchange_name,	delivery->exchange.bytes, delivery->exchange.len);
			AMQP_SET_LONG_PROPERTY(envelope->delivery_tag,	delivery->delivery_tag);
			AMQP_SET_BOOL_PROPERTY(envelope->is_redelivery,	delivery->redelivered);
		} else if (frame.payload.method.id == AMQP_BASIC_DELIVER_METHOD) {
			/* get message metadata */
			amqp_basic_deliver_t *delivery = (amqp_basic_deliver_t *) frame.payload.method.decoded;

			AMQP_SET_STR_PROPERTY(envelope->routing_key,	delivery->routing_key.bytes, delivery->routing_key.len);
			AMQP_SET_STR_PROPERTY(envelope->exchange_name,	delivery->exchange.bytes, delivery->exchange.len);
			AMQP_SET_LONG_PROPERTY(envelope->delivery_tag,	delivery->delivery_tag);
			AMQP_SET_BOOL_PROPERTY(envelope->is_redelivery,	delivery->redelivered);
		} else if (frame.payload.method.id == AMQP_BASIC_GET_EMPTY_METHOD) {
			/* We did a get and there were no messages */
			return AMQP_READ_NO_MESSAGES;
        } else if (frame.payload.method.id == AMQP_CONNECTION_CLOSE_METHOD) {
            amqp_channel_close_t *err = NULL;
            amqp_send_method(
                connection,
                frame.channel,
                AMQP_CONNECTION_CLOSE_OK_METHOD,
                &err
            );
            return AMQP_READ_ERROR;
        }


		/* Read in the next frame */
		result = amqp_simple_wait_frame(connection, &frame);
		if (result < 0) {
			zend_throw_exception(amqp_connection_exception_class_entry, amqp_error_string(-result), -result TSRMLS_CC);
			return AMQP_READ_ERROR;
		}

		/* Make sure it is something we expect*/
		if (frame.frame_type != AMQP_FRAME_HEADER) {
			zend_throw_exception(amqp_queue_exception_class_entry, "Invalid frame type, expecting header.", 0 TSRMLS_CC);
			return AMQP_READ_ERROR;
		}

		body_target = frame.payload.properties.body_size;

		/* Only allocate space for the body if there is a body */
		if (body_target > 0) {
			message_body_buffer = (char *) emalloc(body_target);
			memset(message_body_buffer, 0, body_target);
		}

		p = (amqp_basic_properties_t *) frame.payload.properties.decoded;

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
			zval_dtor(envelope->headers);
			parse_amqp_table(&(p->headers), envelope->headers);
		}

		/* Check if we are going to even get a body */
		if (frame.payload.properties.body_size == 0) {
			/* No point in reading the next frame. Bail! */
			break;
		}

		/* Lets get the body */
		while (body_received < body_target) {
			/* Read in the next frame */
			result = amqp_simple_wait_frame(connection, &frame);
			if (result < 0) {
				zend_throw_exception(amqp_connection_exception_class_entry, amqp_error_string(-result), -result TSRMLS_CC);
				return AMQP_READ_ERROR;
			}

			if (frame.frame_type != AMQP_FRAME_BODY) {
				zend_throw_exception(amqp_queue_exception_class_entry, "Invalid frame type, expecting body.", 0 TSRMLS_CC);
				return AMQP_READ_ERROR;
			}

			memcpy(message_body_buffer + body_received, frame.payload.body_fragment.bytes, frame.payload.body_fragment.len);
			body_received += frame.payload.body_fragment.len;
		}

		/* We are done with this message, we can get out of the loop now */
		break;
	}

	/* Put the final touches into the zval */
	envelope->body = estrndup(message_body_buffer, body_target);
    envelope->body_len = body_target;

	/* Clean up message buffer */
	if (message_body_buffer) {
		efree(message_body_buffer);
	}

	return AMQP_READ_SUCCESS;
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

	/* We have a valid connection: */
	queue->is_connected = '\1';

	/* By default, the auto_delete flag should be set */
	queue->auto_delete = 1;
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
	long flagBitmask = 0;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_queue_class_entry) == FAILURE) {
		return;
	}

	queue = (amqp_queue_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Set the bitmask based on what is set in the queue */
	flagBitmask |= (queue->passive ? AMQP_PASSIVE : 0);
	flagBitmask |= (queue->durable ? AMQP_DURABLE : 0);
	flagBitmask |= (queue->exclusive ? AMQP_EXCLUSIVE : 0);
	flagBitmask |= (queue->auto_delete ? AMQP_AUTODELETE : 0);

	RETURN_LONG(flagBitmask);
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
	queue->passive = IS_PASSIVE(flagBitmask);
	queue->durable = IS_DURABLE(flagBitmask);
	queue->exclusive = IS_EXCLUSIVE(flagBitmask);
	queue->auto_delete = IS_AUTODELETE(flagBitmask);

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

	*return_value = *queue->arguments;
	zval_copy_ctor(return_value);

	/* Increment the ref count */
	Z_ADDREF_P(queue->arguments);
}
/* }}} */

/* {{{ proto AMQPQueue::setArguments(array args)
Overwrite all queue arguments with given args */
PHP_METHOD(amqp_queue_class, setArguments)
{
	zval *id, *zvalArguments;
	amqp_queue_object *queue;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Oa", &id, amqp_queue_class_entry, &zvalArguments) == FAILURE) {
		return;
	}

	/* Pull the queue off the object store */
	queue = (amqp_queue_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Destroy the arguments storage */
	if (queue->arguments) {
		zval_ptr_dtor(&queue->arguments);
	}

	queue->arguments = zvalArguments;

	/* Increment the ref count */
	Z_ADDREF_P(queue->arguments);

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

	amqp_rpc_reply_t res;

	amqp_queue_declare_ok_t *r;

	char *name = "";
	amqp_table_t *arguments;
	long message_count;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_queue_class_entry) == FAILURE) {
		zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Error parsing parameters." ,0 TSRMLS_CC);
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

	arguments = convert_zval_to_arguments(queue->arguments);

	r = amqp_queue_declare(
		connection->connection_resource->connection_state,
		channel->channel_id,
		amqp_cstring_bytes(queue->name),
		queue->passive,
		queue->durable,
		queue->exclusive,
		queue->auto_delete,
		*arguments
	);

	res = AMQP_RPC_REPLY_T_CAST amqp_get_rpc_reply(connection->connection_resource->connection_state);

	AMQP_EFREE_ARGUMENTS(arguments);

	/* handle any errors that occured outside of signals */
	if (res.reply_type != AMQP_RESPONSE_NORMAL) {
		char str[256];
		char ** pstr = (char **) &str;
		amqp_error(res, pstr);
		channel->is_connected = '\0';
		zend_throw_exception(amqp_queue_exception_class_entry, *pstr, 0 TSRMLS_CC);
		amqp_maybe_release_buffers(connection->connection_resource->connection_state);
		return;
	}

	message_count = r->message_count;

	/* Set the queue name, in case it is an autogenerated queue name */
	name = stringify_bytes(r->queue);
	AMQP_SET_NAME(queue, name);
	efree(name);

	amqp_maybe_release_buffers(connection->connection_resource->connection_state);
	RETURN_LONG(message_count);
}
/* }}} */


/* {{{ proto int AMQPQueue::bind(string exchangeName, [string routingKey]);
bind queue to exchange by routing key
*/
PHP_METHOD(amqp_queue_class, bind)
{
	zval *id;
	amqp_queue_object *queue;
	amqp_channel_object *channel;
	amqp_connection_object *connection;
	char *exchange_name;
	int exchange_name_len;
	char *keyname = NULL;
	int keyname_len = 0;

	amqp_rpc_reply_t res;
	amqp_queue_bind_t s;
	amqp_method_number_t bind_ok = AMQP_QUEUE_BIND_OK_METHOD;


	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os|s", &id, amqp_queue_class_entry, &exchange_name, &exchange_name_len, &keyname, &keyname_len) == FAILURE) {
		return;
	}

	queue = (amqp_queue_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Check that the given connection has a channel, before trying to pull the connection off the stack */
	if (queue->is_connected != '\1') {
		zend_throw_exception(amqp_queue_exception_class_entry, "Could not bind queue. No connection available.", 0 TSRMLS_CC);
		return;
	}

	channel = AMQP_GET_CHANNEL(queue);
	AMQP_VERIFY_CHANNEL(channel, "Could not bind queue.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not bind queue.");

	s.ticket 				= 0;
	s.queue.len				= queue->name_len;
	s.queue.bytes			= queue->name;
	s.exchange.len			= exchange_name_len;
	s.exchange.bytes		= exchange_name;
	s.routing_key.len		= keyname_len;
	s.routing_key.bytes		= keyname;
	s.nowait				= 0;
	s.arguments.num_entries = 0;
	s.arguments.entries	 	= NULL;

	res = AMQP_RPC_REPLY_T_CAST amqp_simple_rpc(
		connection->connection_resource->connection_state,
		channel->channel_id,
		AMQP_QUEUE_BIND_METHOD,
		&bind_ok,
		&s
	);

	if (res.reply_type != AMQP_RESPONSE_NORMAL) {
		char str[256];
		char **pstr = (char **)&str;
		amqp_error(res, pstr);

		channel->is_connected = 0;
		zend_throw_exception(amqp_queue_exception_class_entry, *pstr, 0 TSRMLS_CC);
		amqp_maybe_release_buffers(connection->connection_resource->connection_state);
		return;
	}
	amqp_maybe_release_buffers(connection->connection_resource->connection_state);

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
	amqp_basic_get_t s;
	zval *message;
	int read;

	long flags = INI_INT("amqp.auto_ack") ? AMQP_AUTOACK : AMQP_NOPARAM;

	/* Parse out the method parameters */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O|l", &id, amqp_queue_class_entry, &flags) == FAILURE) {
		return;
	}

	queue = (amqp_queue_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Check that the given connection has a channel, before trying to pull the connection off the stack */
	if (queue->is_connected != '\1') {
		zend_throw_exception(amqp_queue_exception_class_entry, "Could not get messages from queue. No connection available.", 0 TSRMLS_CC);
		return;
	}

	channel = AMQP_GET_CHANNEL(queue);
	AMQP_VERIFY_CHANNEL(channel, "Could not get messages from queue.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not get messages from queue.");

	/* Build basic.get request */
	s.ticket = 0,
	s.queue = amqp_cstring_bytes(queue->name);
	s.no_ack = (AMQP_AUTOACK & flags) ? 1 : 0;

	/* Get the next message using basic.get */
	amqp_send_method(
		connection->connection_resource->connection_state,
		channel->channel_id,
		AMQP_BASIC_GET_METHOD,
		&s
	);

	/* Read the message off of the channel */
	MAKE_STD_ZVAL(message);
	read = read_message_from_channel(connection->connection_resource->connection_state, message TSRMLS_CC);

	if (read == AMQP_READ_SUCCESS) {
		COPY_PZVAL_TO_ZVAL(*return_value, message);
	} else {
		zval_ptr_dtor(&message);
		RETURN_FALSE;
    	}
}
/* }}} */


/* {{{ proto array AMQPQueue::consume(callback, [flags = <bitmask>, consumer_tag]);
consume the message
return array messages
*/
PHP_METHOD(amqp_queue_class, consume)
{
	zval *id;
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

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Of|ls", &id, amqp_queue_class_entry, &fci, &fci_cache, &flags, &consumer_tag, &consumer_tag_len) == FAILURE) {
		return;
	}

	/* Pull the queue out */
	queue = (amqp_queue_object *)zend_object_store_get_object(id TSRMLS_CC);

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

	do {
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
			fci.retval_ptr_ptr = &retval_ptr;

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

			/* Call the function, and track the return value */
 			if (zend_call_function(&fci, &fci_cache TSRMLS_CC) == SUCCESS && fci.retval_ptr_ptr && *fci.retval_ptr_ptr) {
 				COPY_PZVAL_TO_ZVAL(*return_value, *fci.retval_ptr_ptr);
			}

			/* Check if user land function wants to bail */
			if (EG(exception) || (Z_TYPE_P(return_value) == IS_BOOL && !Z_BVAL_P(return_value))) {
				function_call_succeeded = 0;
			}

			/* Clean up our mess */
			zend_fcall_info_args_clear(&fci, 1);
			zval_ptr_dtor(&params);
			zval_ptr_dtor(&message);
		} else {
			zval_ptr_dtor(&message);
		}

	} while (read != AMQP_READ_ERROR && function_call_succeeded == 1);
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

	amqp_basic_ack_t s;
	int res;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ol|l", &id, amqp_queue_class_entry, &deliveryTag, &flags ) == FAILURE) {
		return;
	}

	queue = (amqp_queue_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Check that the given connection has a channel, before trying to pull the connection off the stack */
	if (queue->is_connected != '\1') {
		zend_throw_exception(amqp_queue_exception_class_entry, "Could not ack message. No connection available.", 0 TSRMLS_CC);
		return;
	}

	channel = AMQP_GET_CHANNEL(queue);
	AMQP_VERIFY_CHANNEL(channel, "Could not ack message.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not ack message.");

	s.delivery_tag = deliveryTag;
	s.multiple = (AMQP_MULTIPLE & flags) ? 1 : 0;

	res = amqp_send_method(
		connection->connection_resource->connection_state,
		channel->channel_id,
		AMQP_BASIC_ACK_METHOD,
		&s
	);

	if (res) {
		channel->is_connected = 0;
		zend_throw_exception_ex(amqp_queue_exception_class_entry, 0 TSRMLS_CC, "Could not ack message, error code=%d", res);
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

	amqp_basic_nack_t s;
	int res;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ol|l", &id, amqp_queue_class_entry, &deliveryTag, &flags ) == FAILURE) {
		return;
	}

	queue = (amqp_queue_object *)zend_object_store_get_object(id TSRMLS_CC);
	/* Check that the given connection has a channel, before trying to pull the connection off the stack */
	if (queue->is_connected != '\1') {
		zend_throw_exception(amqp_queue_exception_class_entry, "Could not nack message. No connection available.", 0 TSRMLS_CC);
		return;
	}

	channel = AMQP_GET_CHANNEL(queue);
	AMQP_VERIFY_CHANNEL(channel, "Could not nack message.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not nack message.");

	s.delivery_tag = deliveryTag;
	s.multiple = (AMQP_MULTIPLE & flags) ? 1 : 0;
	s.requeue = (AMQP_REQUEUE & flags) ? 1 : 0;

	res = amqp_send_method(
		connection->connection_resource->connection_state,
		channel->channel_id,
		AMQP_BASIC_NACK_METHOD,
		&s
	);

	if (res) {
		channel->is_connected = 0;
		zend_throw_exception_ex(amqp_queue_exception_class_entry, 0 TSRMLS_CC, "Could not nack message, error code=%d", res);
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

	amqp_basic_reject_t s;
	int res;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ol|l", &id, amqp_queue_class_entry, &deliveryTag, &flags ) == FAILURE) {
		return;
	}

	queue = (amqp_queue_object *)zend_object_store_get_object(id TSRMLS_CC);
	/* Check that the given connection has a channel, before trying to pull the connection off the stack */
	if (queue->is_connected != '\1') {
		zend_throw_exception(amqp_queue_exception_class_entry, "Could not reject message. No connection available.", 0 TSRMLS_CC);
		return;
	}

	channel = AMQP_GET_CHANNEL(queue);
	AMQP_VERIFY_CHANNEL(channel, "Could not reject message.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not reject message.");

	s.delivery_tag = deliveryTag;
	s.requeue = (AMQP_REQUEUE & flags) ? 1 : 0;

	res = amqp_send_method(
		connection->connection_resource->connection_state,
		channel->channel_id,
		AMQP_BASIC_REJECT_METHOD,
		&s
	);

	if (res) {
		channel->is_connected = 0;
		zend_throw_exception_ex(amqp_queue_exception_class_entry, 0 TSRMLS_CC, "Could not reject message, error code=%d", res);
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

	amqp_rpc_reply_t res;
	amqp_rpc_reply_t result;
	amqp_queue_purge_t s;
	amqp_method_number_t method_ok = AMQP_QUEUE_PURGE_OK_METHOD;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, amqp_queue_class_entry) == FAILURE) {
		return;
	}

	queue = (amqp_queue_object *)zend_object_store_get_object(id TSRMLS_CC);

	/* Check that the given connection has a channel, before trying to pull the connection off the stack */
	if (queue->is_connected != '\1') {
		zend_throw_exception(amqp_queue_exception_class_entry,	"Could not purge queue. No connection available.", 0 TSRMLS_CC);
		return;
	}

	channel = AMQP_GET_CHANNEL(queue);
	AMQP_VERIFY_CHANNEL(channel, "Could not purge queue.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not purge queue.");

	s.ticket		= 0;
	s.queue.len		= queue->name_len;
	s.queue.bytes	= queue->name;
	s.nowait		= 0;

	result = amqp_simple_rpc(
		connection->connection_resource->connection_state,
		channel->channel_id,
		AMQP_QUEUE_PURGE_METHOD,
		&method_ok,
		&s
	);

	res = AMQP_RPC_REPLY_T_CAST result;

	if (res.reply_type != AMQP_RESPONSE_NORMAL) {
		char str[256];
		char **pstr = (char **)&str;
		amqp_error(res, pstr);
		channel->is_connected = 0;
		zend_throw_exception(amqp_queue_exception_class_entry, *pstr, 0 TSRMLS_CC);
		amqp_maybe_release_buffers(connection->connection_resource->connection_state);
		return;
	}
	amqp_maybe_release_buffers(connection->connection_resource->connection_state);

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

	char *consumer_tag = NULL;
	int consumer_tag_len=0;
	amqp_rpc_reply_t res;
	amqp_rpc_reply_t result;
	amqp_basic_cancel_t s;
	amqp_method_number_t method_ok = AMQP_BASIC_CANCEL_OK_METHOD;


	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O|s", &id, amqp_queue_class_entry, &consumer_tag, &consumer_tag_len) == FAILURE) {
		return;
	}

	queue = (amqp_queue_object *)zend_object_store_get_object(id TSRMLS_CC);
	/* Check that the given connection has a channel, before trying to pull the connection off the stack */
	if (queue->is_connected != '\1') {
		zend_throw_exception(amqp_queue_exception_class_entry, "Could not cancel queue. No connection available.", 0 TSRMLS_CC);
		return;
	}

	channel = AMQP_GET_CHANNEL(queue);
	AMQP_VERIFY_CHANNEL(channel, "Could not cancel queue.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not cancel queue.");

	if (consumer_tag_len) {
		s.consumer_tag.len = consumer_tag_len;
		s.consumer_tag.bytes = consumer_tag;
		s.nowait = 0;
	} else {
		s.consumer_tag.len = queue->consumer_tag_len;
		s.consumer_tag.bytes = queue->consumer_tag;
		s.nowait = 0;
	}

	result = amqp_simple_rpc(
		connection->connection_resource->connection_state,
		channel->channel_id,
		AMQP_BASIC_CANCEL_METHOD,
		&method_ok,
		&s
	);

	res = AMQP_RPC_REPLY_T_CAST result;

	if (res.reply_type != AMQP_RESPONSE_NORMAL) {
		char str[256];
		char **pstr = (char **)&str;
		amqp_error(res, pstr);
		channel->is_connected = 0;
		zend_throw_exception(amqp_queue_exception_class_entry, *pstr, 0 TSRMLS_CC);
		amqp_maybe_release_buffers(connection->connection_resource->connection_state);
		return;
	}
	amqp_maybe_release_buffers(connection->connection_resource->connection_state);

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto int AMQPQueue::unbind(string exchangeName, [string routingKey]);
unbind queue from exchange
*/
PHP_METHOD(amqp_queue_class, unbind)
{
	zval *id;
	amqp_queue_object *queue;
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	char *exchange_name;
	int exchange_name_len;
	char *keyname = NULL;
	int keyname_len = 0;

	amqp_rpc_reply_t res;
	amqp_queue_unbind_t s;
	amqp_method_number_t method_ok = AMQP_QUEUE_UNBIND_OK_METHOD;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os|s", &id, amqp_queue_class_entry, &exchange_name, &exchange_name_len, &keyname, &keyname_len) == FAILURE) {
		return;
	}

	queue = (amqp_queue_object *)zend_object_store_get_object(id TSRMLS_CC);
	/* Check that the given connection has a channel, before trying to pull the connection off the stack */
	if (queue->is_connected != '\1') {
		zend_throw_exception(amqp_queue_exception_class_entry, "Could not unbind queue. No connection available.", 0 TSRMLS_CC);
		return;
	}

	channel = AMQP_GET_CHANNEL(queue);
	AMQP_VERIFY_CHANNEL(channel, "Could not unbind queue.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not unbind queue.");

	s.ticket				= 0;
	s.queue.len				= queue->name_len;
	s.queue.bytes			= queue->name;
	s.exchange.len			= exchange_name_len;
	s.exchange.bytes		= exchange_name;
	s.routing_key.len		= keyname_len;
	s.routing_key.bytes		= keyname;
	s.arguments.num_entries = 0;
	s.arguments.entries		= NULL;

	res = AMQP_RPC_REPLY_T_CAST amqp_simple_rpc(
		connection->connection_resource->connection_state,
		channel->channel_id,
		AMQP_QUEUE_UNBIND_METHOD,
		&method_ok,
		&s);

	if (res.reply_type != AMQP_RESPONSE_NORMAL) {
		char str[256];
		char ** pstr = (char **) &str;
		amqp_error(res, pstr);

		channel->is_connected = 0;
		zend_throw_exception(amqp_queue_exception_class_entry, *pstr, 0 TSRMLS_CC);
		amqp_maybe_release_buffers(connection->connection_resource->connection_state);
		return;
	}
	amqp_maybe_release_buffers(connection->connection_resource->connection_state);

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto int AMQPQueue::delete([long flags = AMQP_NOPARAM]]);
delete queue
*/
PHP_METHOD(amqp_queue_class, delete)
{
	zval *id;
	amqp_queue_object *queue;
	amqp_channel_object *channel;
	amqp_connection_object *connection;

	long flags = AMQP_NOPARAM;

	amqp_rpc_reply_t res;
	amqp_rpc_reply_t result;
	amqp_queue_delete_t s;
	amqp_method_number_t method_ok = AMQP_QUEUE_DELETE_OK_METHOD;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O|l", &id, amqp_queue_class_entry, &flags) == FAILURE) {
		return;
	}

	queue = (amqp_queue_object *)zend_object_store_get_object(id TSRMLS_CC);
	/* Check that the given connection has a channel, before trying to pull the connection off the stack */
	if (queue->is_connected != '\1') {
		zend_throw_exception(amqp_queue_exception_class_entry, "Could not delete queue. No connection available.", 0 TSRMLS_CC);
		return;
	}

	channel = AMQP_GET_CHANNEL(queue);
	AMQP_VERIFY_CHANNEL(channel, "Could not delete queue.");

	connection = AMQP_GET_CONNECTION(channel);
	AMQP_VERIFY_CONNECTION(connection, "Could not delete queue.");

	s.queue.len		= queue->name_len;
	s.queue.bytes	= queue->name;
	s.ticket		= 0;
	s.if_unused		= (AMQP_IFUNUSED & flags) ? 1 : 0;
	s.if_empty		= (AMQP_IFEMPTY & flags) ? 1 : 0;
	s.nowait		= 0;

	result = amqp_simple_rpc(
		connection->connection_resource->connection_state,
		channel->channel_id,
		AMQP_QUEUE_DELETE_METHOD,
		&method_ok,
		&s
	);

	res = AMQP_RPC_REPLY_T_CAST result;

	if (res.reply_type != AMQP_RESPONSE_NORMAL) {
		char str[256];
		char **pstr = (char **)&str;
		amqp_error(res, pstr);
		channel->is_connected = 0;
		zend_throw_exception(amqp_queue_exception_class_entry, *pstr, 0 TSRMLS_CC);
		amqp_maybe_release_buffers(connection->connection_resource->connection_state);
		return;
	}
	amqp_maybe_release_buffers(connection->connection_resource->connection_state);

	RETURN_TRUE;
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
