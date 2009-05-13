check_PROGRAMS = \
	ol_osd_test \
	ol_player_test

INCLUDES = @GTK2_CFLAGS@ \
	@DBUS_GLIB_LIBS@ \
	-I$(top_srcdir)/src
# Tests
ol_osd_test_SOURCES = ol_osd_test.c
ol_osd_test_CPPFLAGS = @GTK2_CFLAGS@
ol_osd_test_LDADD = \
	$(top_srcdir)/src/libol_osd_window.a \
	@GTK2_LIBS@
ol_osd_test_DEPENDENCIES = $(top_srcdir)/src/libol_osd_window.a

ol_player_test_SOURCES = ol_player_test.c
ol_player_test_LDADD = \
	$(top_srcdir)/src/libol_player.a \
	$(top_srcdir)/src/libol_utils.a \
	$(top_srcdir)/src/libol_utils_dbus.a \
	@GTK2_LIBS@ \
	@DBUS_GLIB_LIBS@
ol_player_test_DEPENDENCIES = \
	$(top_srcdir)/src/libol_player.a \
	$(top_srcdir)/src/libol_utils.a \
	$(top_srcdir)/src/libol_utils_dbus.a
