trayicondir = ${datadir}/icons/hicolor/64x64/apps
dist_trayicon_DATA = \
	osdlyrics-trayicon.png 
guidir = @OL_GUIDIR@
dist_gui_DATA = \
	dialogs.glade
menudir = ${datadir}/applications
dist_menu_DATA = osdlyrics.desktop
icondir = ${datadir}/icons/hicolor/64x64/apps
dist_icon_DATA = osdlyrics.png

animdir = ${datadir}/icons/hicolor/16x16/animations
dist_anim_DATA = osdlyrics-loading.png

osd_icondir = ${datadir}/icons/hicolor/scalable/actions
dist_osd_icon_DATA = \
	osdlyrics-osd-play.svg \
	osdlyrics-osd-pause.svg \
	osdlyrics-osd-stop.svg \
	osdlyrics-osd-next.svg \
	osdlyrics-osd-prev.svg \
	osdlyrics-scroll-close.svg

bgdir = ${datadir}/icons/hicolor/scalable/apps
dist_bg_DATA = osdlyrics-osd-bg.svg

defaulticondir = @OL_ICONDIR@
dist_defaulticon_DATA = osdlyrics.png
#svgdir = ${datadir}/icons/hicolor/scalable/apps
#svg_DATA = osdlyrics.svg

bin_SCRIPTS = osdlyrics-daemon
osdlyricsdaemondir = $(pkgdatadir)/setup

servicedir = $(datadir)/dbus-1/services
service_in_files = \
	org.osdlyrics.osdlyrics.service.in \
	org.osdlyrics.Config.service.in\
	$(NULL)

service_DATA = $(service_in_files:.service.in=.service)
EXTRA_DIST = \
	$(service_in_files) \
	osdlyrics-daemon.in \
	$(NULL)

osdlyrics-daemon: osdlyrics-daemon.in
	@sed -e "s|\@pkglibdir\@|$(pkglibdir)|" -e "s|\@PYTHON\@|$(PYTHON)|" $< > $@

org.osdlyrics.osdlyrics.service: org.osdlyrics.osdlyrics.service.in
	@sed -e "s|\@pkglibdir\@|$(pkglibdir)|" -e "s|\@PYTHON\@|$(PYTHON)|" $< > $@

org.osdlyrics.Config.service: org.osdlyrics.Config.service.in
	@sed -e "s|\@pkglibdir\@|$(pkglibdir)|" -e "s|\@PYTHON\@|$(PYTHON)|" $< > $@

CLEANFILES = \
	org.osdlyrics.osdlyrics.service \
	org.osdlyrics.Config.service \
	$(bin_SCRIPTS) \
	$(NULL)
