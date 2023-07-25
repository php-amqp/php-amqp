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

#define PHP5to7_ZEND_HASH_FIND(ht, str, len, res) \
		((res = zend_hash_str_find((ht), (str), (size_t)(len - 1))) != NULL)

#define PHP5to7_ZEND_HASH_STRLEN(len) (size_t)((len) + 1)
#define PHP5to7_ZEND_HASH_DEL(ht, key, len) zend_hash_str_del_ind((ht), (key), (unsigned)(len - 1))
#define PHP5to7_ZEND_HASH_ADD(ht, key, len, pData, nDataSize) zend_hash_str_add((ht), (key), (unsigned)(len - 1), (pData))
#define PHP5to7_ZEND_HASH_STR_UPD_MEM(ht, key, len, pData, nDataSize) zend_hash_str_update_mem((ht), (key), (size_t)(len), &(pData), (nDataSize))
#define PHP5to7_ZEND_HASH_STR_FIND_PTR(ht, key, len, res) ((res = zend_hash_str_find_ptr((ht), (key), (size_t)(len))) != NULL)
#define PHP5to7_ZEND_HASH_STR_DEL(ht, key, len) zend_hash_str_del_ind((ht), (key), (unsigned)(len))



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
