#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
PACKAGE=osd-lyrics
PACKAGE_NAME="OSD Lyrics"
MAJOR_VER=0
MINOR_VER=2
BUILD=20100201

AC_INIT([osd-lyrics], [0.2.20100201], [http://code.google.com/p/osd-lyrics/issues/list])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/ol_main.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_RANLIB
AC_CONFIG_FILES([ po/Makefile.in
	Makefile
	dailydeb.sh
	src/Makefile
	src/tests/Makefile
	data/Makefile
])

#define paths
OL_ICONDIR="$datadir/$PACKAGE/icons"
AC_SUBST(OL_ICONDIR)
OL_GUIDIR="$datadir/$PACKAGE"
AC_SUBST(OL_GUIDIR)

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
])

AC_ARG_ENABLE([mpd],
  [AS_HELP_STRING([--disable-mpd], [Disables MPD support. You won't need libmpd if you use this option])],
  [],
  [enable_mpd=yes])

AC_ARG_ENABLE([xmms2],
  [AS_HELP_STRING([--disable-xmms2], [Disables XMMS2 support. You won't need libxmmsclient if you use this option])],
  [],
  [enable_xmms2=yes])

AC_ARG_ENABLE([amarok1],
  [AS_HELP_STRING([--enable-amarok1], [Enables Amarok 1.4 support])],
  [],
  [enable_amarok1=no])

AS_IF([test "x$enable_amarok1" != xno],
  [AC_DEFINE([ENABLE_AMAROK1], [1], [Define if compile to support Amarok 1.4])])

# Checks for libraries.
PKG_CHECK_MODULES([GTK2], [gtk+-2.0 >= 2.12.0, gthread-2.0, gmodule-export-2.0])
PKG_CHECK_MODULES([DBUS_GLIB], [dbus-glib-1])
PKG_CHECK_MODULES([CURL], [libcurl])
MPD_CFLAGS=
MPD_LIBS=
AS_IF([test "x$enable_mpd" != "xno"],
  [PKG_CHECK_MODULES([MPD], [libmpd],
    [AC_DEFINE([ENABLE_MPD], [1], [Define if compile to support MPD])
     #BUILD_MPD_SUPPORT=1
    ],
    [enable_mpd=no
     #BUILD_MPD_SUPPORT=0
     AC_MSG_FAILURE(
       [libmpd is not available, use --disable-mpd to disable MPD support])
    ])
  ],
  [BUILD_MPD_SUPPORT=0
   AC_SUBST(MPD_CFLAGS)
   AC_SUBST(MPD_LIBS)
  ]
)
XMMS2_CFLAGS=
XMMS2_LIBS=
AS_IF([test "x$enable_xmms2" != "xno"],
  [PKG_CHECK_MODULES([XMMS2], [xmms2-client],
    [AC_DEFINE([ENABLE_XMMS2], [1], [Define if compile to support XMMS2])
    dnl OLD_CFLAGS="$CFLAGS"
    dnl CFLAGS="$CFLAGS $XMMS2_CFLAGS"
    #AC_DEFINE([XMMS2_MAIN_VERSION], 0, [The main version of XMMS2 client])
    dnl AC_CHECK_HEADERS([xmmsc/xmmsv.h]) 
    dnl AC_COMPILE_IFELSE([
    dnl 	#include <xmmsclient/xmmsclient.h>
    dnl 	void foo () {
    dnl 		xmmsc_playback_seek_ms (NULL, 0, 0);
    dnl 	}
    dnl ], 
    dnl [AC_DEFINE([XMMS_SEEK_WHENCE], [1], [Define if xmms_playback_seek_ms takes whence parameter])])
    dnl CFLAGS="$OLD_CFLAGS"
    dnl PKG_CHECK_EXISTS(
    dnl 	[xmms2-client >= 0.6], 
    dnl 	[AC_DEFINE([XMMS2_HAVE_XMMSV], 1, [Whether we can use xmmsv_t])])
     #BUILD_XMMS2_SUPPORT=1
    #AC_SUBST(xmms2_version)
    ],
    [enable_xmms2=no
     #BUILD_XMMS2_SUPPORT=0
     AC_MSG_FAILURE(
       [xmms2-client is not available, use --disable-xmms2 to disable XMMS2 support])
    ])
  ],
  [BUILD_XMMS2_SUPPORT=0
   AC_SUBST(XMMS2_CFLAGS)
   AC_SUBST(XMMS2_LIBS)
  ]
)
#AM_CONDITIONAL(BUILD_MPD_SUPPORT, [test "x$enable_mpd" != "xno"])
#AC_SUBST(BUILD_MPD_SUPPORT)
# i18n support from GNU gettext.
AM_GNU_GETTEXT_VERSION([0.17])
AM_GNU_GETTEXT([external])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_OUTPUT

chmod +x dailydeb.sh