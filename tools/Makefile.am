bin_SCRIPTS = osdlyrics-create-lyricsource

osdlyricstoolsdir = $(pkglibdir)/tools
osdlyricstools_PYTHON = \
	create-lyricsource.py \
	$(NULL)

osdlyrics-create-lyricsource: osdlyrics-create-lyricsource.in
	@sed -e "s|\@pkglibdir\@|$(pkglibdir)|" -e "s|\@PYTHON\@|$(PYTHON)|" $< > $@

CLEANFILES = \
	osdlyrics-create-lyricsource \
	$(NULL)
