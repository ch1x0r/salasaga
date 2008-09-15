bin_PROGRAMS = salasaga_screencapture

salasaga_screencapture_SOURCES = display_warning.c \
	validate_value.c \
	salasaga_screencapture.c

salasaga_screencapture_LDADD = $(DEPS_LIBS) $(GTK_LIBS) $(ZLIB)

AM_CPPFLAGS = $(DEPS_CFLAGS) $(GTK_CFLAGS) \
	-DLOCALEDIR=\""$(DESTDIR)/$(datadir)/salasaga/locale"\"

SUBDIRS = intl po

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = config/config.rpath m4/ChangeLog