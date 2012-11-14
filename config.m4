dnl $Id: config.m4 322428 2012-01-17 21:42:40Z pdezwart $
dnl config.m4 for extension amqp

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.
dnl amqp
dnl If your extension references something external, use with:


dnl Make sure that the comment is aligned:
PHP_ARG_WITH(amqp, for amqp support,
[  --with-amqp             Include amqp support])

PHP_ARG_WITH(librabbitmq-dir,  for amqp,
[  --with-librabbitmq-dir[=DIR]   Set the path to librabbit-mq install prefix.], yes)


if test "$PHP_AMQP" != "no"; then
	dnl Write more examples of tests here...

	AC_MSG_RESULT($PHP_AMQP)

	dnl # --with-amqp -> check with-path

	SEARCH_FOR="amqp_framing.h"

	AC_MSG_CHECKING([for amqp files in default path])
	if test "$PHP_LIBRABBITMQ_DIR" != "no" && test "$PHP_LIBRABBITMQ_DIR" != "yes"; then
		for i in $PHP_LIBRABBITMQ_DIR; do
			if test -r $i/include/$SEARCH_FOR;
				then
				AMQP_DIR=$i
				AC_MSG_RESULT(found in $i)
				break
			fi
		done
	else
		for i in $PHP_AMQP /usr/local /usr ; do
			if test -r $i/include/$SEARCH_FOR;
				then
				AMQP_DIR=$i
				AC_MSG_RESULT(found in $i)
				break
			fi
		done
	fi

	if test -z "$AMQP_DIR"; then
		AC_MSG_RESULT([not found])
		AC_MSG_ERROR([Please reinstall the librabbit-mq distribution])
	fi

	dnl # --with-amqp -> add include path

	PHP_ADD_INCLUDE($AMQP_DIR/include)

	dnl # --with-amqp -> check for lib and symbol presence

	LIBNAME=rabbitmq
	LIBSYMBOL=rabbitmq


	PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $AMQP_DIR/lib, AMQP_SHARED_LIBADD)
	PHP_SUBST(AMQP_SHARED_LIBADD)

	AMQP_SOURCES="amqp.c amqp_exchange.c amqp_queue.c amqp_connection.c amqp_channel.c amqp_envelope.c amqp_object_store.c"

	PHP_NEW_EXTENSION(amqp, $AMQP_SOURCES, $ext_shared)
fi
