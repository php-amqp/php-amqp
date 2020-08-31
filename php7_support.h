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

#ifndef PHP_AMQP_PHP7_SUPPORT_H
#define PHP_AMQP_PHP7_SUPPORT_H

typedef size_t PHP5to7_param_str_len_type_t;
typedef zend_long PHP5to7_param_long_type_t;
typedef zval PHP5to7_zval_t;

#define PHP5to7_MAYBE_SET_TO_NULL

#define PHP5to7_MAYBE_DEREF(zv) (zv)
#define PHP5to7_MAYBE_PTR(zv) (&(zv))
#define PHP5to7_MAYBE_PTR_TYPE PHP5to7_zval_t *
#define PHP5to7_MAYBE_PARAM_PTR(zv) (zv)

#define PHP5to7_MAYBE_INIT(zv) ZVAL_UNDEF(&(zv))
#define PHP5to7_ARRAY_INIT(zv) array_init(&(zv));
#define PHP5to7_MAYBE_DESTROY(zv) if (!Z_ISUNDEF(zv)) { zval_ptr_dtor(&(zv)); }
#define PHP5to7_MAYBE_DESTROY2(zv, pzv) if (!Z_ISUNDEF(zv)) { zval_ptr_dtor(pzv); }

#define PHP5to7_ZVAL_STRINGL_DUP(z, s, l) ZVAL_STRINGL((z), (s), (l))
#define PHP5to7_ADD_NEXT_INDEX_STRINGL_DUP(arg, str, length) add_next_index_stringl((arg), (str), (size_t)(length))

#define PHP5to7_ZEND_HASH_FIND(ht, str, len, res) \
		((res = zend_hash_str_find((ht), (str), (size_t)(len - 1))) != NULL)

#define PHP5to7_ZEND_HASH_STRLEN(len) (PHP5to7_param_str_len_type_t)((len) + 1)
#define PHP5to7_ZEND_HASH_DEL(ht, key, len) zend_hash_str_del_ind((ht), (key), (unsigned)(len - 1))
#define PHP5to7_ZEND_HASH_ADD(ht, key, len, pData, nDataSize) zend_hash_str_add((ht), (key), (unsigned)(len - 1), (pData))
#define PHP5to7_ZEND_HASH_STR_UPD_MEM(ht, key, len, pData, nDataSize) zend_hash_str_update_mem((ht), (key), (size_t)(len), &(pData), (nDataSize))
#define PHP5to7_ZEND_HASH_STR_FIND_PTR(ht, key, len, res) ((res = zend_hash_str_find_ptr((ht), (key), (size_t)(len))) != NULL)
#define PHP5to7_ZEND_HASH_STR_DEL(ht, key, len) zend_hash_str_del_ind((ht), (key), (unsigned)(len))

#define PHP5to7_SET_FCI_RETVAL_PTR(fci, pzv) (fci).retval = (pzv);
#define PHP5to7_CHECK_FCI_RETVAL_PTR(fci) ((fci).retval)

#define PHP5to7_IS_FALSE_P(pzv) (Z_TYPE_P(pzv) == IS_FALSE)

#define PHP5to7_obj_free_zend_object zend_object
#define PHP5to7_zend_object_value zend_object *
#define PHP5to7_zend_register_internal_class_ex(ce, parent_ce) zend_register_internal_class_ex((ce), (parent_ce) TSRMLS_CC)

#define PHP5to7_ECALLOC_CONNECTION_OBJECT(ce) (amqp_connection_object*)ecalloc(1, sizeof(amqp_connection_object) + zend_object_properties_size(ce))
#define PHP5to7_ECALLOC_CHANNEL_OBJECT(ce) (amqp_channel_object*)ecalloc(1, sizeof(amqp_channel_object) + zend_object_properties_size(ce))

#define PHP5to7_CASE_IS_BOOL case IS_TRUE: case IS_FALSE

#define PHP5to7_READ_PROP_RV_PARAM_DECL zval rv;
#define PHP5to7_READ_PROP_RV_PARAM_CC , (&rv)

#define Z_BVAL_P(zval_p) (Z_TYPE_P(zval_p) == IS_TRUE)

#define PHP5to7_ZEND_REAL_HASH_KEY_T zend_string

#define PHP5to7_ZEND_HASH_FOREACH_KEY_VAL(ht, num_key, real_key, key, key_len, data, val, pos) \
  ZEND_HASH_FOREACH_KEY_VAL((ht), (num_key), (real_key), (val))

#define PHP5to7_ZEND_HASH_KEY_IS_STRING(ht, real_key, key, key_len, num_key, pos) \
    (real_key)

#define PHP5to7_ZEND_HASH_KEY_MAYBE_UNPACK(real_key, key, key_len) \
    (key_len) = ZSTR_LEN(real_key); \
    (key) = ZSTR_VAL(real_key);

#define PHP5to7_ZEND_HASH_FOREACH_END() ZEND_HASH_FOREACH_END();

/* Resources stuff */
typedef zend_resource* PHP5to7_zend_resource_t;
typedef zend_resource PHP5to7_zend_resource_store_t;
typedef zval PHP5to7_zend_resource_le_t;

#define PHP5to7_ZEND_RESOURCE_DTOR_ARG res
#define PHP5to7_ZEND_RESOURCE_EMPTY NULL
#define PHP5to7_ZEND_RESOURCE_LE_EMPTY NULL
#define PHP5to7_ZEND_RSRC_TYPE_P(le) (le)->type
#define PHP5to7_ZEND_REGISTER_RESOURCE(rsrc_pointer, rsrc_type) zend_register_resource((rsrc_pointer), (rsrc_type))

#define PHP5to7_PARENT_CLASS_NAME_C(name)

#define PHP5to7_ZEND_ACC_FINAL_CLASS ZEND_ACC_FINAL


/* Small change to let it build after a major internal change for php8.0
 * More info:
 * https://github.com/php/php-src/blob/php-8.0.0alpha3/UPGRADING.INTERNALS#L47
 */
#if PHP_MAJOR_VERSION >= 8

# define TSRMLS_DC
# define TSRMLS_D
# define TSRMLS_CC
# define TSRMLS_C

#define PHP5to8_OBJ_PROP(zv) Z_OBJ_P(zv)

#else

#define PHP5to8_OBJ_PROP(zv) (zv)

# endif

#endif //PHP_AMQP_PHP7_SUPPORT_H

/*
*Local variables:
*tab-width: 4
*c-basic-offset: 4
*End:
*vim600: noet sw=4 ts=4 fdm=marker
*vim<600: noet sw=4 ts=4
*/
