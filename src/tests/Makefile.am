check_PROGRAMS = \
	ol_osd_test \
	ol_player_test\
	ol_lrc_parser_test\
	ol_lrc_test \
	ol_lrc_fetch_test \
	ol_config_test \
	ol_color_test \
	ol_lrc_fetch_ui_test \
	ol_lrc_fetch_module_test \
	ol_path_test \
	ol_utils_cmdline_test \
	ol_fork_test \
	ol_music_info_test \
	ol_lrclib_test

INCLUDES = @GTK2_CFLAGS@ \
	@DBUS_GLIB_CFLAGS@ \
	@MPD_CFLAGS@ \
	@XMMS2_CFLAGS@ \
	@SQLITE_CFLAGS@ \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/lib/chardetect/include
AM_CPPFLAGS = \
	-DDATADIR='"$(datadir)"' \
	-DICONDIR='"$(OL_ICONDIR)"' \
	-DGUIDIR='"$(OL_GUIDIR)"'
LDADD = \
	@GTK2_LIBS@ \
	@DBUS_GLIB_LIBS@ \
	@XMMS2_LIBS@ \
	@MPD_LIBS@ \
	@CURL_LIBS@ \
	@SQLITE_LIBS@ \
	$(top_srcdir)/lib/chardetect/chardetect.a \
	-lstdc++
# Tests
ol_utils_cmdline_test_SOURCES = ol_utils_cmdline_test.c \
	$(top_srcdir)/src/ol_debug.c \
	$(top_srcdir)/src/ol_utils_cmdline.c

ol_osd_test_SOURCES = ol_osd_test.c \
	$(top_srcdir)/src/ol_osd_window.c \
	$(top_srcdir)/src/ol_osd_render.c \
	$(top_srcdir)/src/ol_debug.c \
	$(top_srcdir)/src/ol_color.c \
	$(top_srcdir)/src/ol_image_button.c \
	$(top_srcdir)/src/ol_utils.c

ol_player_test_SOURCES = ol_player_test.c \
	$(top_srcdir)/src/ol_debug.c \
	$(top_srcdir)/src/ol_player.c \
	$(top_srcdir)/src/ol_player_amarok1.c \
	$(top_srcdir)/src/ol_player_amarok2.c \
	$(top_srcdir)/src/ol_player_banshee.c \
	$(top_srcdir)/src/ol_player_mpris.c \
	$(top_srcdir)/src/ol_player_exaile02.c \
	$(top_srcdir)/src/ol_player_exaile03.c \
	$(top_srcdir)/src/ol_player_audacious.c \
	$(top_srcdir)/src/ol_player_rhythmbox.c \
	$(top_srcdir)/src/ol_player_mpd.c \
	$(top_srcdir)/src/ol_player_moc.c \
	$(top_srcdir)/src/ol_player_xmms2.c \
	$(top_srcdir)/src/ol_player_songbird.c \
	$(top_srcdir)/src/ol_player_quodlibet.c \
	$(top_srcdir)/src/ol_player_juk.c \
	$(top_srcdir)/src/ol_player_qmmp.c \
	$(top_srcdir)/src/ol_elapse_emulator.c \
	$(top_srcdir)/src/ol_config.c \
	$(top_srcdir)/src/ol_marshal.c \
	$(top_srcdir)/src/ol_utils.c \
	$(top_srcdir)/src/ol_utils_cmdline.c \
	$(top_srcdir)/src/ol_utils_dbus.c \
	$(top_srcdir)/src/ol_utils_dcop.c \
	$(top_srcdir)/src/ol_music_info.c 

ol_lrc_parser_test_SOURCES = ol_lrc_parser_test.c \
	$(top_srcdir)/src/ol_debug.c \
	$(top_srcdir)/src/ol_lrc_parser.c \
	$(top_srcdir)/src/ol_utils.c

ol_lrc_test_SOURCES = ol_lrc_test.c \
	$(top_srcdir)/src/ol_debug.c \
	$(top_srcdir)/src/ol_lrc.c \
	$(top_srcdir)/src/ol_utils.c \
	$(top_srcdir)/src/ol_lrc_parser.c 

ol_lrc_fetch_test_SOURCES = ol_lrc_fetch_test.c \
	$(top_srcdir)/src/ol_debug.c \
	$(top_srcdir)/src/ol_lrc_fetch.c \
	$(top_srcdir)/src/ol_lrc_fetch_utils.c \
	$(top_srcdir)/src/ol_lrc_fetch_sogou.c \
	$(top_srcdir)/src/ol_lrc_fetch_qianqian.c \
	$(top_srcdir)/src/ol_lrc_fetch_minilyrics.c \
	$(top_srcdir)/src/ol_music_info.c \
	$(top_srcdir)/src/ol_md5.c \
	$(top_srcdir)/src/ol_utils.c

ol_config_test_SOURCES = \
	$(top_srcdir)/src/ol_debug.c \
	ol_config_test.c \
	$(top_srcdir)/src/ol_config.c \
	$(top_srcdir)/src/ol_marshal.c

ol_color_test_SOURCES = \
	$(top_srcdir)/src/ol_debug.c \
	ol_color_test.c \
	$(top_srcdir)/src/ol_color.c

ol_lrc_fetch_ui_test_SOURCES = ol_lrc_fetch_ui_test.c \
	$(top_srcdir)/src/ol_debug.c \
	$(top_srcdir)/src/ol_lrc_fetch.c \
	$(top_srcdir)/src/ol_lrc_fetch_utils.c \
	$(top_srcdir)/src/ol_lrc_fetch_sogou.c \
	$(top_srcdir)/src/ol_lrc_fetch_qianqian.c \
	$(top_srcdir)/src/ol_lrc_fetch_minilyrics.c \
	$(top_srcdir)/src/ol_lrc_fetch_module.c \
	$(top_srcdir)/src/ol_md5.c \
	$(top_srcdir)/src/ol_fork.c \
	$(top_srcdir)/src/ol_music_info.c \
	$(top_srcdir)/src/ol_utils.c \
	$(top_srcdir)/src/ol_lrc_fetch_ui.c \
	$(top_srcdir)/src/ol_lrc_candidate_list.c \
	$(top_srcdir)/src/ol_config.c \
	$(top_srcdir)/src/ol_marshal.c \
	$(top_srcdir)/src/ol_gui.c

ol_lrc_fetch_module_test_SOURCES = ol_lrc_fetch_module_test.c \
	$(top_srcdir)/src/ol_debug.c \
	$(top_srcdir)/src/ol_lrc_fetch.c \
	$(top_srcdir)/src/ol_lrc_fetch_module.c \
	$(top_srcdir)/src/ol_lrc_fetch_utils.c \
	$(top_srcdir)/src/ol_lrc_fetch_sogou.c \
	$(top_srcdir)/src/ol_lrc_fetch_qianqian.c \
	$(top_srcdir)/src/ol_lrc_fetch_minilyrics.c \
	$(top_srcdir)/src/ol_music_info.c \
	$(top_srcdir)/src/ol_md5.c \
	$(top_srcdir)/src/ol_utils.c \
	$(top_srcdir)/src/ol_lrc_fetch_ui.c \
	$(top_srcdir)/src/ol_lrc_candidate_list.c \
	$(top_srcdir)/src/ol_fork.c \
	$(top_srcdir)/src/ol_config.c \
	$(top_srcdir)/src/ol_marshal.c \
	$(top_srcdir)/src/ol_gui.c 

ol_path_test_SOURCES = \
	 ol_path_test.c \
	$(top_srcdir)/src/ol_debug.c \
	$(top_srcdir)/src/ol_path_pattern.c \
	$(top_srcdir)/src/ol_utils.c \
	$(top_srcdir)/src/ol_music_info.c

ol_fork_test_SOURCES = \
	ol_fork_test.c \
	$(top_srcdir)/src/ol_debug.c \
	$(top_srcdir)/src/ol_fork.c

ol_music_info_test_SOURCES = \
	ol_music_info_test.c \
	$(top_srcdir)/src/ol_debug.c \
	$(top_srcdir)/src/ol_music_info.c \
	$(top_srcdir)/src/ol_utils.c

ol_lrclib_test_SOURCES = \
	ol_lrclib_test.c \
	$(top_srcdir)/src/ol_debug.c \
	$(top_srcdir)/src/ol_lrclib.c \
	$(top_srcdir)/src/ol_music_info.c \
	$(top_srcdir)/src/ol_utils.c
