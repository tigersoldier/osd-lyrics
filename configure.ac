#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([osd-lyrics], [0.1], [http://code.google.com/p/osd-lyrics/issues/list])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/ol_osd_window.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_RANLIB
AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/tests/Makefile])

# Checks for libraries.
PKG_CHECK_MODULES([GTK2], [gtk+-2.0 >= 2.8.0])
PKG_CHECK_MODULES([DBUS_GLIB], [dbus-glib-1])
PKG_CHECK_MODULES([CURL], [libcurl])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_OUTPUT
