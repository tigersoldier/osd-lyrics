check_PROGRAMS = \
	ol_osd_test \
	ol_player_test\
	ol_lrc_parser_test\
	ol_lrc_utility_test \
	ol_lrc_fetch_test \
	ol_config_test \
	ol_color_test

INCLUDES = @GTK2_CFLAGS@ \
	@DBUS_GLIB_CFLAGS@ \
	-I$(top_srcdir)/src
# Tests
ol_osd_test_SOURCES = ol_osd_test.c \
	$(top_srcdir)/src/ol_osd_window.c \
	$(top_srcdir)/src/ol_osd_render.c \
	$(top_srcdir)/src/ol_color.c \
	$(top_srcdir)/src/ol_utils.c
ol_osd_test_LDADD = \
	@GTK2_LIBS@

ol_player_test_SOURCES = ol_player_test.c \
	$(top_srcdir)/src/ol_player.c \
	$(top_srcdir)/src/ol_player_amarok1.c \
	$(top_srcdir)/src/ol_player_amarok2.c \
	$(top_srcdir)/src/ol_player_banshee.c \
	$(top_srcdir)/src/ol_player_mpris.c \
	$(top_srcdir)/src/ol_player_audacious.c \
	$(top_srcdir)/src/ol_player_rhythmbox.c \
	$(top_srcdir)/src/ol_utils.c \
	$(top_srcdir)/src/ol_utils_dbus.c \
	$(top_srcdir)/src/ol_utils_dcop.c 
ol_player_test_LDADD = \
	@GTK2_LIBS@ \
	@DBUS_GLIB_LIBS@

ol_lrc_parser_test_SOURCES = ol_lrc_parser_test.c
ol_lrc_parser_test_LDADD = \
	$(top_srcdir)/src/libol_lrc_parser.a \
	$(top_srcdir)/src/libol_lrc_utility.a

ol_lrc_utility_test_SOURCES = ol_lrc_utility_test.c
ol_lrc_utility_test_LDADD = \
	$(top_srcdir)/src/libol_lrc_parser.a \
	$(top_srcdir)/src/libol_lrc_utility.a

ol_lrc_fetch_test_SOURCES = ol_lrc_fetch_test.c \
	$(top_srcdir)/src/ol_lrc_fetch.c \
	$(top_srcdir)/src/ol_lrc_fetch_utils.c \
	$(top_srcdir)/src/ol_lrc_fetch_sogou.c \
	$(top_srcdir)/src/ol_utils.c
ol_lrc_fetch_test_LDADD = \
	@CURL_LIBS@ \
	@GTK2_LIBS@

ol_config_test_SOURCES = \
	ol_config_test.c \
	$(top_srcdir)/src/ol_config.c
ol_config_test_LDADD = \
	@GTK2_LIBS@

ol_color_test_SOURCES = \
	ol_color_test.c \
	$(top_srcdir)/src/ol_color.c
ol_color_test_LDADD = \
	@GTK2_LIBS@