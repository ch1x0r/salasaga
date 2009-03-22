desktop_filedir = $(datadir)/applications
desktop_file_DATA = salasaga.desktop

AM_CPPFLAGS = -DICON_PNG_DIR=\""$(datadir)/pixmaps"\"

EXTRA_DIST = salasaga.desktop.in

CLEANFILES = salasaga.desktop

do_subst = sed -e 's,[@]ICON_PNG_DIR[@],$(datadir)/pixmaps,g'

salasaga.desktop: salasaga.desktop.in
	$(do_subst) < $(srcdir)/salasaga.desktop.in > salasaga.desktop

DISTCLEANFILES = Makefile.in
