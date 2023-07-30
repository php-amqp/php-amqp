dnl config.m4 for extension amqp

PHP_ARG_WITH(amqp, for amqp support,
[	--with-amqp						 Include amqp support])

PHP_ARG_WITH(librabbitmq-dir,	for amqp,
[	--with-librabbitmq-dir[=DIR]	 Set the path to librabbitmq install prefix.], yes)

dnl Set test wrapper binary to ignore any local ini settings
PHP_EXECUTABLE="\$(top_srcdir)/php-test-bin"

if test "$PHP_AMQP" != "no"; then
	AC_MSG_CHECKING([for supported PHP versions])        
	PHP_REF_FOUND_VERSION=$PHP_VERSION
        PHP_REF_FOUND_VERNUM=$PHP_VERSION_ID
        if test -z "$PHP_REF_FOUND_VERNUM"; then
                if test -z "$PHP_CONFIG"; then
                        AC_MSG_ERROR([php-config not found])
                fi
                PHP_REF_FOUND_VERSION=`${PHP_CONFIG} --version`
                PHP_REF_FOUND_VERNUM=`${PHP_CONFIG} --vernum`
        fi

	if test "$PHP_REF_FOUND_VERNUM" -lt "50600"; then
		AC_MSG_ERROR([PHP version not supported, >= 5.6 required, but $PHP_REF_FOUND_VERSION found])
	else
		AC_MSG_RESULT([supported ($PHP_REF_FOUND_VERSION)])
	fi

	AC_MSG_RESULT($PHP_AMQP)

	dnl # --with-amqp -> check with-path

	NEW_LAYOUT=rabbitmq-c/framing.h
	OLD_LAYOUT=amqp_framing.h
	AC_PATH_PROG(PKG_CONFIG, pkg-config, no)

	HAVE_LIBRABBITMQ_NEW_LAYOUT=0

	if test "$PHP_LIBRABBITMQ_DIR" = "yes" -a -x $PKG_CONFIG; then
		AC_MSG_CHECKING([for amqp using pkg-config])

		if ! $PKG_CONFIG --exists librabbitmq ; then
			AC_MSG_ERROR([librabbitmq not found])
		fi

		LIBRABBITMQ_VERSION=`$PKG_CONFIG librabbitmq --modversion`
		AC_MSG_RESULT([found version $LIBRABBITMQ_VERSION])

		if ! $PKG_CONFIG librabbitmq --atleast-version 0.8.0 ; then
			AC_MSG_ERROR([librabbitmq must be version 0.8.0 or greater])
		fi

		if ! $PKG_CONFIG librabbitmq --atleast-version 0.10.0 ; then
            AC_MSG_WARN([librabbitmq 0.10.0 or greater recommended, current version is $LIBRABBITMQ_VERSION])
        fi

		if test -r `$PKG_CONFIG librabbitmq --variable=includedir`/$NEW_LAYOUT; then
		    HAVE_LIBRABBITMQ_NEW_LAYOUT=1
		fi

		PHP_AMQP_LIBS=`$PKG_CONFIG librabbitmq --libs`
		PHP_AMQP_INCS=`$PKG_CONFIG librabbitmq --cflags`

	    PHP_EVAL_LIBLINE($PHP_AMQP_LIBS, AMQP_SHARED_LIBADD)
	    PHP_EVAL_INCLINE($PHP_AMQP_INCS)

	else
		AC_MSG_CHECKING([for amqp files in default path])
		if test "$PHP_LIBRABBITMQ_DIR" != "no" && test "$PHP_LIBRABBITMQ_DIR" != "yes"; then
			for i in $PHP_LIBRABBITMQ_DIR; do
			    if test -r $i/include/$NEW_LAYOUT; then
                    AMQP_DIR=$i
                    HAVE_LIBRABBITMQ_NEW_LAYOUT=1
                    AC_MSG_RESULT(found in $i)
                    break
                fi
				if test -r $i/include/$OLD_LAYOUT; then
					AMQP_DIR=$i
					AC_MSG_RESULT(found in $i)
					break
				fi
			done
		else
			for i in $PHP_AMQP /usr/local /usr ; do
			    if test -r $i/include/$NEW_LAYOUT; then
                    AMQP_DIR=$i
                    HAVE_LIBRABBITMQ_NEW_LAYOUT=1
                    AC_MSG_RESULT(found in $i)
                    break
                fi
				if test -r $i/include/$OLD_LAYOUT; then
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

			if test "$LIBRABBITMQ_API_VERSION" -lt 8000 ; then
				 AC_MSG_ERROR([librabbitmq must be version 0.8.0 or greater, $ac_cv_librabbitmq_version version given instead])
			fi

			if test "$LIBRABBITMQ_API_VERSION" -lt 10000 ; then
				 AC_MSG_WARN([librabbitmq 0.10.0 or greater recommended, current version is $ac_cv_librabbitmq_version])
			fi
		else
			AC_MSG_ERROR([could not determine librabbitmq version])
		fi

		dnl # --with-amqp -> check for lib and symbol presence
		LIBNAME=rabbitmq
		LIBSYMBOL=rabbitmq

		PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $AMQP_DIR/$PHP_LIBDIR, AMQP_SHARED_LIBADD)
	fi

	AC_MSG_CHECKING([for new librabbitmq layout])
    AC_MSG_RESULT([${HAVE_LIBRABBITMQ_NEW_LAYOUT}])
    AC_DEFINE_UNQUOTED(HAVE_LIBRABBITMQ_NEW_LAYOUT, ${HAVE_LIBRABBITMQ_NEW_LAYOUT}, ["Librabbitmq new layout"])

	PHP_SUBST(AMQP_SHARED_LIBADD)

	AMQP_SOURCES="amqp.c amqp_envelope_exception.c amqp_type.c amqp_exchange.c amqp_queue.c amqp_connection.c amqp_connection_resource.c amqp_channel.c amqp_envelope.c amqp_basic_properties.c amqp_methods_handling.c amqp_timestamp.c amqp_decimal.c"

	PHP_NEW_EXTENSION(amqp, $AMQP_SOURCES, $ext_shared)
fi
