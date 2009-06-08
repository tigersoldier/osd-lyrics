bin_PROGRAMS = osd-lyrics

lib_LIBRARIES = \
	libol_osd_window.a \
	libol_player.a \
	libol_utils.a \
	libol_utils_dbus.a \
	libol_lrc_parser.a \
	libol_lrc_utility.a \
	libol_lrc_fetch.a \
	libol_trayicon.a \
	libol_utils_dcop.a

#ol_player_test
#TEST = $(check_PROGRAMS)

INCLUDES = \
	@GTK2_CFLAGS@ \
	@DBUS_GLIB_CFLAGS@ \
	@CURL_CFLAGS@
AM_CPPFLAGS = \
	-DDATADIR='"$(datadir)"' \
	-DICONDIR='"$(OL_ICONDIR)"'

osd_lyrics_SOURCES = \
	ol_main.c

osd_lyrics_DEPENDENCIES = \
	libol_osd_window.a \
	libol_utils.a \
	libol_player.a \
	libol_utils_dbus.a \
	libol_lrc_parser.a \
	libol_lrc_utility.a \
	libol_lrc_fetch.a \
	libol_trayicon.a \
	libol_utils_dcop.a
#osd_lyrics_CXXFLAGS = @GTK2_CFLAGS@
osd_lyrics_LDADD = \
	libol_osd_window.a \
	libol_utils.a \
	libol_player.a \
	libol_utils_dbus.a \
	libol_lrc_parser.a \
	libol_lrc_utility.a \
	libol_lrc_fetch.a \
	libol_trayicon.a \
	libol_utils_dcop.a \
	@GTK2_LIBS@ \
	@DBUS_GLIB_LIBS@ \
	@CURL_LIBS@
osd_lyrics_LDFLAGS = -L$(top_srcdir)/src/

# Librarys

libol_osd_window_a_SOURCES = \
	ol_osd_window.c \
	ol_osd_window.h \
	ol_osd_render.c

libol_player_a_SOURCES = \
	ol_player.c \
	ol_player_banshee.c \
	ol_player_exaile.c \
	ol_player_amarok1.c \
	ol_player_amarok2.c \
	ol_player_mpris.c

libol_player_a_LIBADD = \
	libol_utils.a \
	libol_utils_dcop.a
libol_player_a_DEPENDENCIES = \
	libol_utils.a \
	libol_utils_dcop.a

libol_utils_a_SOURCES = \
	ol_utils.c \
	ol_utils.h

libol_utils_dbus_a_SOURCES = \
	ol_utils_dbus.c \
	ol_utils_dbus.h

libol_utils_dcop_a_SOURCES = \
	ol_utils_dcop.c \
	ol_utils_dcop.h

libol_lrc_parser_a_SOURCES = \
	ol_lrc_parser.c \
	ol_lrc_parser.h

libol_lrc_utility_a_SOURCES = \
	ol_lrc_utility.c \
	ol_lrc_utility.h
libol_lrc_utility_a_LIBADD = \
	libol_lrc_parser.a
libol_lrc_utility_a_DEPENDENCIES = libol_lrc_parser.a

libol_trayicon_a_SOURCES = \
	ol_trayicon.c \
	ol_trayicon.h
libol_trayicon_a_LIBADD = \
	libol_trayicon.a
libol_trayicon_a_DEPENDENCIES = libol_osd_window.a

libol_lrc_fetch_a_SOURCES = \
	ol_lrc_fetch_sogou.c \
	ol_lrc_fetch_utils.c

SUBDIRS = tests
