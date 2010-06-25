bin_PROGRAMS = salasaga_screencapture

salasaga_screencapture_SOURCES = display_warning.c \
	validate_value.c \
	salasaga_screencapture.c

salasaga_screencapture_LDADD = $(DEPS_LIBS) $(GTK_LIBS) $(ZLIB) $(LIBNOTIFY_LIBS)
AM_CPPFLAGS = $(DEPS_CFLAGS) $(GTK_CFLAGS) \
	-DLOCALEDIR=\""$(DESTDIR)/$(datadir)/salasaga/locale"\"

DISTCLEANFILES = Makefile.in
