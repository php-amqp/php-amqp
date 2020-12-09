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

#ifndef PHP_AMQP_PHP5_SUPPORT_H
#define PHP_AMQP_PHP5_SUPPORT_H

typedef int PHP5to7_param_str_len_type_t;
typedef long PHP5to7_param_long_type_t;
typedef zval* PHP5to7_zval_t;

#define PHP5to7_MAYBE_SET_TO_NULL = NULL

#define PHP5to7_MAYBE_DEREF(zv) (*(zv))
#define PHP5to7_MAYBE_PTR(zv) (zv)
#define PHP5to7_MAYBE_PTR_TYPE PHP5to7_zval_t
#define PHP5to7_MAYBE_PARAM_PTR(zv) (&(zv))

#define PHP5to7_MAYBE_INIT(zv) MAKE_STD_ZVAL(zv);
#define PHP5to7_ARRAY_INIT(zv) array_init(zv);
#define PHP5to7_MAYBE_DESTROY(zv) zval_ptr_dtor(&(zv));
#define PHP5to7_MAYBE_DESTROY2(zv, pzv) zval_ptr_dtor(&pzv);

#define PHP5to7_ZVAL_STRINGL_DUP(z, s, l) ZVAL_STRINGL((z), (s), (l), 1)

#define PHP5to7_ADD_NEXT_INDEX_STRINGL_DUP(arg, str, length) add_next_index_stringl((arg), (str), (unsigned)(length), 1)

#define PHP5to7_ZEND_HASH_FIND(ht, str, len, res) \
        (zend_hash_find((ht), (str), (unsigned)(len), (void **) &(res)) != FAILURE)

#define PHP5to7_ZEND_HASH_STRLEN(len) (unsigned)((len) + 1)
#define PHP5to7_ZEND_HASH_DEL(ht, key, len) zend_hash_del_key_or_index((ht), (key), (unsigned)(len), 0, HASH_DEL_KEY);
#define PHP5to7_ZEND_HASH_ADD(ht, key, len, pData, nDataSize) (zend_hash_add((ht), (key), (unsigned)(len), &(pData), nDataSize, NULL) != FAILURE)
#define PHP5to7_ZEND_HASH_STR_UPD_MEM(ht, key, len, pData, nDataSize) PHP5to7_ZEND_HASH_ADD((ht), (key), (len), (pData), (nDataSize))
#define PHP5to7_ZEND_HASH_STR_FIND_PTR(ht, key, len, res) PHP5to7_ZEND_HASH_FIND((ht), (key), (len), (res))
#define PHP5to7_ZEND_HASH_STR_DEL(ht, key, len) PHP5to7_ZEND_HASH_DEL((ht), (key), (len))

#define PHP5to7_SET_FCI_RETVAL_PTR(fci, pzv) (fci).retval_ptr_ptr = &(pzv);
#define PHP5to7_CHECK_FCI_RETVAL_PTR(fci) ((fci).retval_ptr_ptr && *(fci).retval_ptr_ptr)

#define PHP5to7_IS_FALSE_P(pzv) ((Z_TYPE_P(pzv) == IS_BOOL && !Z_BVAL_P(pzv)))

#define PHP5to7_obj_free_zend_object void
#define PHP5to7_zend_object_value zend_object_value
#define PHP5to7_zend_register_internal_class_ex(ce, parent_ce) zend_register_internal_class_ex((ce), (parent_ce), NULL TSRMLS_CC)

#define PHP5to7_ECALLOC_CONNECTION_OBJECT(ce) (amqp_connection_object*)ecalloc(1, sizeof(amqp_connection_object))
#define PHP5to7_ECALLOC_CHANNEL_OBJECT(ce) (amqp_channel_object*)ecalloc(1, sizeof(amqp_channel_object))

#define PHP5to7_CASE_IS_BOOL case IS_BOOL

#define PHP5to7_READ_PROP_RV_PARAM_DECL
#define PHP5to7_READ_PROP_RV_PARAM_CC


#define PHP5to7_ZEND_REAL_HASH_KEY_T void

#define PHP5to7_ZEND_HASH_FOREACH_KEY_VAL(ht, num_key, real_key, key, key_len, data, val, pos) \
  for ( \
      zend_hash_internal_pointer_reset_ex((ht), &(pos)); \
      zend_hash_get_current_data_ex((ht), (void**) &(data), &(pos)) == SUCCESS && ((value) = *(data)); \
      zend_hash_move_forward_ex((ht), &(pos)) \
  )

#define PHP5to7_ZEND_HASH_KEY_IS_STRING(ht, real_key, key, key_len, num_key, pos) \
    (zend_hash_get_current_key_ex((ht), &(key), &(key_len), &(num_key), 0, &(pos)) == HASH_KEY_IS_STRING)

#define PHP5to7_ZEND_HASH_KEY_MAYBE_UNPACK(real_key, key, key_len)

#define PHP5to7_ZEND_HASH_FOREACH_END()

#define Z_TRY_ADDREF_P(pz) Z_ADDREF_P(pz)

/* Resources stuff */

typedef int PHP5to7_zend_resource_t;
typedef zend_rsrc_list_entry PHP5to7_zend_resource_store_t;
typedef zend_rsrc_list_entry PHP5to7_zend_resource_le_t;

#define PHP5to7_ZEND_RESOURCE_DTOR_ARG rsrc
#define Z_RES_P(le) (le)

#define PHP5to7_ZEND_RESOURCE_EMPTY 0
#define PHP5to7_ZEND_RESOURCE_LE_EMPTY NULL
#define PHP5to7_ZEND_RSRC_TYPE_P(le) Z_TYPE_P(le)
#define PHP5to7_ZEND_REGISTER_RESOURCE(rsrc_pointer, rsrc_type) ZEND_REGISTER_RESOURCE(NULL, (rsrc_pointer), (rsrc_type))

#define PHP5to7_PARENT_CLASS_NAME_C(name) , (name)

#define ZEND_ULONG_FMT "%" PRIu64
#define PHP5to7_ZEND_ACC_FINAL_CLASS ZEND_ACC_FINAL_CLASS

#define PHP5to8_OBJ_PROP(zv) (zv)

#endif //PHP_AMQP_PHP5_SUPPORT_H

/*
*Local variables:
*tab-width: 4
*c-basic-offset: 4
*End:
*vim600: noet sw=4 ts=4 fdm=marker
*vim<600: noet sw=4 ts=4
*/
