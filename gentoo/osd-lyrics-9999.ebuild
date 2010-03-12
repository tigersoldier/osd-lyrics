# Copyright 1999-2009 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI="2"

inherit autotools subversion versionator

MY_PN="${PN/-/}"
DESCRIPTION="An OSD lyric show compatable with various media players and lyrics fecting from web."
HOMEPAGE="http://code.google.com/p/osd-lyrics/"
ESVN_REPO_URI="http://${PN}.googlecode.com/svn/trunk/"

LICENSE="GPL-3"
SLOT="0"
KEYWORDS=""
IUSE="mpd xmms2"

DEPEND="
	dev-libs/dbus-glib
	gnome-base/libglade
	net-misc/curl
	x11-libs/gtk+
	mpd? ( media-libs/libmpd )
	xmms2? ( media-sound/xmms2 )"
RDEPEND="${DEPEND}"

src_prepare() {
	eautoreconf
}

src_configure() {
	econf $(use_enable mpd) $(use_enable xmms2)
}

src_compile() {
	emake DESTDIR="${D}" || die "Compile failed"
}

src_install() {
	emake DESTDIR="${D}" install || die "Install failed"
	dodoc AUTHORS ChangeLog NEWS* README*
}

pkg_postinst() {
	if ! use mpd ;then
		elog "MPD support is disabled. To compile with MPD support, emerge ${PN}"
		elog "with \`\`mpd'' USE flag"
	fi
}
