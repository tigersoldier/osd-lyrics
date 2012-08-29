mpris1_PYTHON = mpris1.py

mpris1dir = $(pkglibdir)/players/mpris1


servicedir = $(datadir)/dbus-1/services
service_in_files = org.osdlyrics.PlayerProxy.Mpris1.service.in
service_DATA = $(service_in_files:.service.in=.service)

EXTRA_DIST = \
	$(service_in_files) \
	$(NULL)

$(service_DATA): $(service_in_files)
	@sed -e "s|\@pkglibdir\@|$(pkglibdir)|" -e "s|\@PYTHON\@|$(PYTHON)|" $< > $@

CLEANFILES = \
	org.osdlyrics.PlayerProxy.Mpris1.service \
	$(NULL)
