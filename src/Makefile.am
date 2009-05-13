bin_PROGRAMS = osd-lyrics

lib_LIBRARIES = \
	libol_osd_window.a \
	libol_player.a \
	libol_utils.a \
	libol_utils_dbus.a \
	libol_lrc_parser.a \
	libol_lrc_utility.a

#ol_player_test
#TEST = $(check_PROGRAMS)

INCLUDES = @GTK2_CFLAGS@ @DBUS_GLIB_CFLAGS@
#AM_CFLAGS = `pkg-config --cflags gtk+-2.0 dbus-glib-1`

osd_lyrics_SOURCES = \
	ol_main.c

osd_lyrics_DEPENDENCIES = \
	libol_osd_window.a \
	libol_utils.a \
	libol_player.a \
	libol_utils_dbus.a \
	libol_lrc_parser.a \
	libol_lrc_utility.a
#osd_lyrics_CXXFLAGS = @GTK2_CFLAGS@
osd_lyrics_LDADD = \
	libol_osd_window.a \
	libol_utils.a \
	libol_player.a \
	libol_utils_dbus.a \
	libol_lrc_parser.a \
	libol_lrc_utility.a \
	@GTK2_LIBS@ \
	@DBUS_GLIB_LIBS@
osd_lyrics_LDFLAGS = -L$(top_srcdir)/src/

# Librarys

libol_osd_window_a_SOURCES = ol_osd_window.c ol_osd_window.h

libol_player_a_SOURCES = \
	ol_player.c \
	ol_player.h \
	ol_player_banshee.c \
	ol_player_banshee.h \
	ol_player_exaile.c \
	ol_player_exaile.h
libol_player_a_LIBADD = \
	libol_utils.a
libol_player_a_DEPENDENCIES = libol_utils.a

libol_utils_a_SOURCES = \
	ol_utils.c \
	ol_utils.h

libol_utils_dbus_a_SOURCES = \
	ol_utils_dbus.c \
	ol_utils_dbus.h

libol_lrc_parser_a_SOURCES = \
	ol_lrc_parser.c \
	ol_lrc_parser.h

libol_lrc_utility_a_SOURCES = \
	ol_lrc_utility.c \
	ol_lrc_utility.h
libol_lrc_utility_a_LIBADD = \
	libol_lrc_parser.a
libol_lrc_utility_a_DEPENDENCIES = libol_lrc_parser.a

SUBDIRS = tests
