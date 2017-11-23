/*
 * This file is part of the pdezwart/php-amqp PHP extension.
 *
 * Copyright (c) php-amqp contributors
 *
 * Licensed under the MIT license: http://opensource.org/licenses/MIT
 *
 * For the full copyright and license information, please view the
 * LICENSE file that was distributed with this source or visit
 * http://opensource.org/licenses/MIT
 */

#include "php.h"

extern zend_class_entry *amqp_connection_class_entry;

int php_amqp_connect(amqp_connection_object *amqp_connection, zend_bool persistent, INTERNAL_FUNCTION_PARAMETERS);
void php_amqp_disconnect_force(amqp_connection_resource *resource TSRMLS_DC);


PHP_MINIT_FUNCTION(amqp_connection);

/*
*Local variables:
*tab-width: 4
*c-basic-offset: 4
*End:
*vim600: noet sw=4 ts=4 fdm=marker
*vim<600: noet sw=4 ts=4
*/
