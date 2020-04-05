dnl config.m4 for extension amqp

PHP_ARG_WITH(amqp, for amqp support,
[	--with-amqp						 Include amqp support])

PHP_ARG_WITH(librabbitmq-dir,	for amqp,
[	--with-librabbitmq-dir[=DIR]	 Set the path to librabbitmq install prefix.], yes)

dnl Set test wrapper binary to ignore any local ini settings
PHP_EXECUTABLE="$PWD/php-test-bin"

if test "$PHP_AMQP" != "no"; then
	AC_MSG_CHECKING([for supported PHP versions])
	PHP_REF_FOUND_VERSION=`${PHP_CONFIG} --version`
	PHP_REF_FOUND_VERNUM=`${PHP_CONFIG} --vernum`

	if test "$PHP_REF_FOUND_VERNUM" -lt "50600"; then
		AC_MSG_ERROR([PHP version not supported, >= 5.6 required, but $PHP_REF_FOUND_VERSION found])
	else
		AC_MSG_RESULT([supported ($PHP_REF_FOUND_VERSION)])
	fi

	AC_MSG_RESULT($PHP_AMQP)

	dnl # --with-amqp -> check with-path

	SEARCH_FOR="amqp_framing.h"
	AC_PATH_PROG(PKG_CONFIG, pkg-config, no)

	if test "$PHP_LIBRABBITMQ_DIR" = "yes" -a -x $PKG_CONFIG; then
		AC_MSG_CHECKING([for amqp using pkg-config])

		if ! $PKG_CONFIG --exists librabbitmq ; then
			AC_MSG_ERROR([librabbitmq not found])
		fi

		PHP_AMQP_VERSION=`$PKG_CONFIG librabbitmq --modversion`
		AC_MSG_RESULT([found version $PHP_AMQP_VERSION])

		if ! $PKG_CONFIG librabbitmq --atleast-version 0.7.1 ; then
			AC_MSG_ERROR([librabbitmq must be version 0.7.1 or greater])
		fi

		PHP_AMQP_LIBS=`$PKG_CONFIG librabbitmq --libs`
		PHP_AMQP_INCS=`$PKG_CONFIG librabbitmq --cflags`

	    PHP_EVAL_LIBLINE($PHP_AMQP_LIBS, AMQP_SHARED_LIBADD)
	    PHP_EVAL_INCLINE($PHP_AMQP_INCS)

	else
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
			AC_MSG_ERROR([Please reinstall the librabbitmq distribution itself or (re)install librabbitmq development package if it available in your system])
		fi

		dnl # --with-amqp -> add include path
		PHP_ADD_INCLUDE($AMQP_DIR/include)

		old_CFLAGS=$CFLAGS
		CFLAGS="$CFLAGS -I$AMQP_DIR/include"

		AC_CACHE_CHECK(for librabbitmq version, ac_cv_librabbitmq_version, [
			AC_RUN_IFELSE([AC_LANG_SOURCE([[
				#include "amqp.h"
				#include <stdio.h>

				int main ()
				{
					FILE *testfile = fopen("conftestval", "w");

					if (NULL == testfile) {
						return 1;
					}

					fprintf(testfile, "%s\n", AMQ_VERSION_STRING);
					fclose(testfile);

					return 0;
				}
			]])],[ac_cv_librabbitmq_version=`cat ./conftestval`],[ac_cv_librabbitmq_version=NONE],[ac_cv_librabbitmq_version=NONE])
		])

		CFLAGS=$old_CFLAGS

		if test "$ac_cv_librabbitmq_version" != "NONE"; then
			ac_IFS=$IFS
			IFS=.
			set $ac_cv_librabbitmq_version
			IFS=$ac_IFS
			LIBRABBITMQ_API_VERSION=`expr [$]1 \* 1000000 + [$]2 \* 1000 + [$]3`

			if test "$LIBRABBITMQ_API_VERSION" -lt 5001 ; then
				 AC_MSG_ERROR([librabbitmq must be version 0.5.2 or greater, $ac_cv_librabbitmq_version version given instead])
			fi

			if test "$LIBRABBITMQ_API_VERSION" -lt 6000 ; then
				 AC_MSG_WARN([librabbitmq 0.6.0 or greater recommended, current version is $ac_cv_librabbitmq_version])
			fi
		else
			AC_MSG_ERROR([could not determine librabbitmq version])
		fi

		dnl # --with-amqp -> check for lib and symbol presence
		LIBNAME=rabbitmq
		LIBSYMBOL=rabbitmq

		PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $AMQP_DIR/$PHP_LIBDIR, AMQP_SHARED_LIBADD)
	fi
	PHP_SUBST(AMQP_SHARED_LIBADD)

	AMQP_SOURCES="amqp.c amqp_type.c amqp_exchange.c amqp_queue.c amqp_connection.c amqp_connection_resource.c amqp_channel.c amqp_envelope.c amqp_basic_properties.c amqp_methods_handling.c amqp_timestamp.c amqp_decimal.c"

	PHP_NEW_EXTENSION(amqp, $AMQP_SOURCES, $ext_shared)
fi
