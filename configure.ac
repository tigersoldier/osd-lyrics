#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([osd-lyrics], [0.1], [http://code.google.com/p/osd-lyrics/issues/list])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/ol_main.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_RANLIB
AC_CONFIG_FILES([ po/Makefile.in
	Makefile
	src/Makefile
	src/tests/Makefile
	data/Makefile
])

#define paths
OL_ICONDIR="$datadir/osd-lyrics/icons"
AC_SUBST(OL_ICONDIR)

# gprof support
AC_ARG_WITH(gprof-support,
[  --with-gprof-support=DIR   Include gmon.out file renaming support 
  --without-gprof-support    Don't do gmon.out file renaming (default)],
[ case "$withval" in
  yes)
    AC_MSG_RESULT(yes)
    AC_DEFINE([WITH_GPROF_SUPPORT], [], [Support gprof profiling])
    CFLAGS="-pg $CFLAGS"
    LDFLAGS="-pg $LDFLAGS"
    AC_MSG_WARN(--with-gprof-support called without argument - using default)
    gmondir="/tmp"
    AC_SUBST(gmondir)
    ;;
  no)
    AC_MSG_RESULT(no)
    ;;
  *)
    AC_MSG_RESULT(yes)
    AC_DEFINE([WITH_GPROF_SUPPORT], [], [Support gprof profiling])
    CFLAGS="-pg $CFLAGS"
    LDFLAGS="-pg $LDFLAGS"
    gmondir="$withval"
    AC_SUBST(gmondir)
    ;;
  esac 
],
AC_MSG_RESULT(no))

# Checks for libraries.
PKG_CHECK_MODULES([GTK2], [gtk+-2.0 >= 2.8.0])
PKG_CHECK_MODULES([DBUS_GLIB], [dbus-glib-1])
PKG_CHECK_MODULES([CURL], [libcurl])
# i18n support from GNU gettext.
AM_GNU_GETTEXT_VERSION([0.17])
AM_GNU_GETTEXT([external])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_OUTPUT
