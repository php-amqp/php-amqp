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
