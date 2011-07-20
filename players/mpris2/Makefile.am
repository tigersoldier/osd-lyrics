mpris2_PYTHON = mpris2.py

mpris2dir = $(pkglibdir)/players/mpris2


servicedir = $(datadir)/dbus-1/services
service_in_files = org.osdlyrics.PlayerProxy.Mpris2.service.in
service_DATA = $(service_in_files:.service.in=.service)

EXTRA_DIST = \
	$(service_in_files) \
	$(NULL)

$(service_DATA): $(service_in_files)
	@sed -e "s|\@pkglibdir\@|$(pkglibdir)|" -e "s|\@PYTHON\@|$(PYTHON)|" $< > $@

CLEANFILES = \
	org.osdlyrics.PlayerProxy.Mpris2.service \
	$(NULL)
