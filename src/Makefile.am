#bin_PROGRAMS = osd-lyrics

lib_LIBRARIES = \
	libol_osd_window.a \
	libol_player.a \
	libol_utils.a \
	libol_utils_dbus.a

#check_PROGRAMS = ol_osd_window_test ol_player_test
#TEST = $(check_PROGRAMS)

INCLUDES = `pkg-config --cflags gtk+-2.0 dbus-glib-1`

#osd_lyrics_SOURCES = 

libol_osd_window_a_SOURCES = ol_osd_window.c ol_osd_window.h

libol_player_a_SOURCES = \
	ol_player.c \
	ol_player.h ol \
	ol_player_banshee.c \
	ol_player_banshee.h \
	ol_player_exaile.c \
	ol_player_exaile.h

libol_utils_a_SOURCES = \
	ol_utils.c \
	ol_utils.h

libol_utils_dbus_a_SOURCES = \
	ol_utils_dbus.c \
	ol_utils_dbus.h
