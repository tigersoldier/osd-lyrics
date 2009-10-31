bin_PROGRAMS = osd-lyrics

#ol_player_test
#TEST = $(check_PROGRAMS)

INCLUDES = \
	@GTK2_CFLAGS@ \
	@DBUS_GLIB_CFLAGS@ \
	@CURL_CFLAGS@ \
	@GLADE_CFLAGS@
AM_CPPFLAGS = \
	-DDATADIR='"$(datadir)"' \
	-DICONDIR='"$(OL_ICONDIR)"' \
	-DGLADEDIR='"$(OL_GLADEDIR)"'

osd_lyrics_SOURCES = \
	ol_main.c \
	ol_config.c \
	ol_glade.c \
	ol_about.c \
	ol_option.c \
	ol_osd_window.c \
	ol_osd_render.c \
	ol_osd_module.c \
	ol_player.c \
	ol_player_banshee.c \
	ol_player_exaile02.c \
	ol_player_exaile03.c \
	ol_player_amarok1.c \
	ol_player_amarok2.c \
	ol_player_mpris.c \
	ol_player_audacious.c \
	ol_player_songbird.c \
	ol_player_xmms2.c \
	ol_player_rhythmbox.c \
	ol_elapse_emulator.c \
	ol_utils.c \
	ol_utils_dbus.c \
	ol_utils_dcop.c \
	ol_lrc_parser.c \
	ol_lrc_utility.c \
	ol_trayicon.c \
	ol_menu.c \
	ol_lrc_fetch.c \
	ol_lrc_fetch_module.c \
	ol_lrc_fetch_ui.c \
	ol_lrc_fetch_sogou.c \
	ol_lrc_fetch_qianqian.c \
	ol_lrc_fetch_utils.c \
	ol_music_info.c \
	ol_color.c \
	ol_eggaccelerators.c \
	ol_keybinder.c \
	ol_keybindings.c \
	ol_commands.c \
	ol_path_manage.c \
	ol_marshal.c

#osd_lyrics_CXXFLAGS = @GTK2_CFLAGS@
osd_lyrics_LDADD = \
	@GTK2_LIBS@ \
	@DBUS_GLIB_LIBS@ \
	@CURL_LIBS@ \
	@GLADE_LIBS@
osd_lyrics_LDFLAGS = -L$(top_srcdir)/src/

# Librarys

SUBDIRS = tests

localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@
