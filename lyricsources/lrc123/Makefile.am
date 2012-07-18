lrc123_PYTHON = lrc123.py

lrc123dir = $(pkglibdir)/lyricsources/lrc123


servicedir = $(datadir)/dbus-1/services
service_in_files = org.osdlyrics.LyricSourcePlugin.lrc123.service.in
service_DATA = $(service_in_files:.service.in=.service)

EXTRA_DIST = \
	$(service_in_files) \
	$(NULL)

$(service_DATA): $(service_in_files)
	@sed -e "s|\@pkglibdir\@|$(pkglibdir)|" -e "s|\@PYTHON\@|$(PYTHON)|" $< > $@

CLEANFILES = \
	org.osdlyrics.LyricSourcePlugin.lrc123.service \
	$(NULL)
