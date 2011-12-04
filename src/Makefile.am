bin_PROGRAMS = osdlyrics

#ol_player_test
#TEST = $(check_PROGRAMS)

AM_CPPFLAGS = \
	@GTK2_CFLAGS@ \
	@DBUS_GLIB_CFLAGS@ \
	@CURL_CFLAGS@ \
	@MPD_CFLAGS@ \
	@XMMS2_CFLAGS@ \
	@NOTIFY_CFLAGS@ \
	@SQLITE_CFLAGS@ \
	@X11_CFLAGS@ \
	@APP_INDICATOR_CFLAGS@ \
	-DDATADIR='"$(datadir)"' \
	-DICONDIR='"$(OL_ICONDIR)"' \
	-DGUIDIR='"$(OL_GUIDIR)"'
LDADD = \
	@GTK2_LIBS@ \
	@DBUS_GLIB_LIBS@ \
	@CURL_LIBS@ \
	@MPD_LIBS@ \
	@XMMS2_LIBS@ \
	@NOTIFY_LIBS@ \
	@SQLITE_LIBS@ \
	@X11_LIBS@ \
	@APP_INDICATOR_LIBS@ \
	-lm \
	$(NULL)

noinst_HEADERS = \
	gettext.h \
	ol_about.h \
	ol_app.h \
	ol_app_info.h \
	ol_app_chooser_widget.h \
	ol_cell_renderer_button.h \
	ol_color.h \
	ol_commands.h \
	ol_config.h \
	ol_config_property.h \
	ol_consts.h \
	ol_debug.h \
	ol_display_module.h \
	ol_eggaccelerators.h \
	ol_elapse_emulator.h \
	ol_fork.h \
	ol_gui.h \
	ol_image_button.h \
	ol_intl.h \
	ol_keybinder.h \
	ol_keybinding_settings.h \
	ol_keybindings.h \
	ol_lrc_candidate_list.h \
	ol_lrc_engine_list.h \
	ol_lrc_fetch.h \
	ol_lrc_fetch_module.h \
	ol_lrc_fetch_xiami.h \
	ol_lrc_fetch_ttplayer.h \
	ol_lrc_fetch_ui.h \
	ol_lrc_fetch_utils.h \
	ol_lrc.h \
	ol_lyrics.h \
	ol_marshal.h \
	ol_md5.h \
	ol_menu.h \
	ol_metadata.h \
	ol_notify.h \
	ol_option.h \
	ol_osd_module.h \
	ol_osd_render.h \
	ol_osd_toolbar.h \
	ol_osd_window.h \
	ol_path_pattern.h \
	ol_player.h \
	ol_scroll_module.h \
	ol_scroll_window.h \
	ol_search_dialog.h \
	ol_stock.h \
	ol_trayicon.h \
	ol_utils_cmdline.h \
	ol_utils_dbus.h \
	ol_utils_dcop.h \
	ol_utils_network.h \
	ol_gussian_blur.h \
	ol_player_chooser.h \
	ol_utils.h

osdlyrics_SOURCES = \
	ol_app_info.c \
	ol_app_chooser_widget.c \
	ol_debug.c \
	ol_main.c \
	ol_config.c \
	ol_gui.c \
	ol_about.c \
	ol_option.c \
	ol_display_module.c \
	ol_osd_window.c \
	ol_osd_toolbar.c \
	ol_osd_render.c \
	ol_osd_module.c \
	ol_scroll_module.c \
	ol_scroll_window.c \
	ol_image_button.c \
	ol_player.c \
	ol_elapse_emulator.c \
	ol_md5.c \
	ol_utils.c \
	ol_utils_dbus.c \
	ol_utils_dcop.c \
	ol_utils_cmdline.c \
	ol_utils_network.c \
	ol_lrc.c \
	ol_lyrics.c \
	ol_search_dialog.c \
	ol_trayicon.c \
	ol_menu.c \
	ol_lrc_fetch.c \
	ol_lrc_fetch_module.c \
	ol_lrc_fetch_ui.c \
	ol_lrc_fetch_xiami.c \
	ol_lrc_fetch_ttplayer.c \
	ol_lrc_fetch_utils.c \
	ol_lrc_candidate_list.c \
	ol_lrc_engine_list.c \
	ol_metadata.c \
	ol_color.c \
	ol_eggaccelerators.c \
	ol_keybinder.c \
	ol_keybindings.c \
	ol_commands.c \
	ol_path_pattern.c \
	ol_cell_renderer_button.c \
	ol_fork.c \
	ol_marshal.c \
	ol_stock.c \
	ol_notify.c \
	ol_player_chooser.c \
	ol_gussian_blur.c

#osd_lyrics_CXXFLAGS = @GTK2_CFLAGS@


# Librarys

SUBDIRS = tests

localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@
