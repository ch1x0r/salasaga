# Copyright 1999-2007 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header$

inherit autotools eutils flag-o-matic multilib 

DESCRIPTION="An Integrated Development Environment for producing Animated SVG
files"
HOMEPAGE="http://www.flameproject.org/"
SRC_URI="http://downloads.sourceforge.net/flameproject/flameproject-0.7.7.dev-200709032220.tar.bz2"

LICENSE="LGPL-2"
SLOT="0"
KEYWORDS="~x86 ~amd64"
IUSE="debug"

RDEPEND="sys-devel/autoconf
	dev-util/pkgconfig
	media-libs/giflib
	dev-libs/glib
	x11-libs/gtk+
	gnome-base/libgnome
	media-libs/libpng
	media-libs/jpeg
	dev-libs/libxml2
	sys-libs/zlib
	gnome-base/gconf"

DEPEND="${RDEPEND}"

src_compile() {
	mv ${WORKDIR}/flame_project-0.7.7.dev ${WORKDIR}/${P}
	cd "${S}" || die "Directory not found"
	autoconf || die "Autoconfig failed"
	econf || die "econf failed"
	emake || die "emake failed"
}

src_install() {
	einstall || die "Install failed"
}

