bin_PROGRAMS = osd-lyrics

#ol_player_test
#TEST = $(check_PROGRAMS)

INCLUDES = \
	@GTK2_CFLAGS@ \
	@DBUS_GLIB_CFLAGS@ \
	@CURL_CFLAGS@ \
	@MPD_CFLAGS@ \
	@XMMS2_CFLAGS@ \
	@NOTIFY_CFLAGS@ \
	@SQLITE_CFLAGS@ \
	-I$(top_srcdir)/lib/chardetect/include
AM_CPPFLAGS = \
	-DDATADIR='"$(datadir)"' \
	-DICONDIR='"$(OL_ICONDIR)"' \
	-DGUIDIR='"$(OL_GUIDIR)"' 
AM_LDFLAGS = \
	-L$(top_srcdir)/lib/chardetect
LDADD = \
	@GTK2_LIBS@ \
	@DBUS_GLIB_LIBS@ \
	@CURL_LIBS@ \
	@MPD_LIBS@ \
	@XMMS2_LIBS@ \
	@NOTIFY_LIBS@ \
	@SQLITE_LIBS@ \
	$(top_srcdir)/lib/chardetect/chardetect.a \
	-lstdc++

osd_lyrics_SOURCES = \
	ol_main.c \
	ol_config.c \
	ol_gui.c \
	ol_about.c \
	ol_option.c \
	ol_osd_window.c \
	ol_osd_toolbar.c \
	ol_osd_render.c \
	ol_osd_module.c \
	ol_image_button.c \
	ol_player.c \
	ol_player_banshee.c \
	ol_player_exaile02.c \
	ol_player_exaile03.c \
	ol_player_amarok1.c \
	ol_player_amarok2.c \
	ol_player_mpris.c \
	ol_player_audacious.c \
	ol_player_quodlibet.c \
	ol_player_songbird.c \
	ol_player_xmms2.c \
	ol_player_rhythmbox.c \
	ol_player_moc.c \
	ol_player_mpd.c \
	ol_elapse_emulator.c \
	ol_md5.c \
	ol_utils.c \
	ol_utils_dbus.c \
	ol_utils_dcop.c \
	ol_utils_cmdline.c \
	ol_lrc_parser.c \
	ol_lrc.c \
	ol_search_dialog.c \
	ol_trayicon.c \
	ol_menu.c \
	ol_lrc_fetch.c \
	ol_lrc_fetch_module.c \
	ol_lrc_fetch_ui.c \
	ol_lrc_fetch_sogou.c \
	ol_lrc_fetch_qianqian.c \
	ol_lrc_fetch_utils.c \
	ol_lrc_candidate_list.c \
	ol_lrc_engine_list.c \
	ol_music_info.c \
	ol_color.c \
	ol_eggaccelerators.c \
	ol_keybinder.c \
	ol_keybindings.c \
	ol_commands.c \
	ol_path_pattern.c \
	ol_lyric_manage.c \
	ol_cell_renderer_button.c \
	ol_fork.c \
	ol_marshal.c \
	ol_stock.c \
	ol_notify.c \
	ol_lrclib.c

#osd_lyrics_CXXFLAGS = @GTK2_CFLAGS@


# Librarys

SUBDIRS = tests

localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@
