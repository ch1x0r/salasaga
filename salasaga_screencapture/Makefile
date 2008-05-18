include Makefile.config

OFILES = \
	display_warning.o \
	salasaga_screencapture.o \
	validate_value.o

all: salasaga_screencapture

display_warning.o:
	$(CC) $(CFLAGS) -c display_warning.c -o display_warning.o

salasaga_screencapture.o:
	$(CC) $(CFLAGS) -c salasaga_screencapture.c -o salasaga_screencapture.o

validate_value.o:
	$(CC) $(CFLAGS) -c validate_value.c -o validate_value.o

salasaga_screencapture: $(OFILES)
	$(CC) $(CFLAGS) $(OFILES) $(LDFLAGS) $(LIBS) -o salasaga_screencapture

install:
	$(INSTALL) -d $(DESTDIR)$(bindir)
	$(INSTALL_PROGRAM) -m 0755 salasaga_screencapture $(DESTDIR)$(bindir)

uninstall:
	if [ -f "$(DESTDIR)$(bindir)/salasaga_screencapture" ]; then \
		rm -f $(DESTDIR)$(bindir)/salasaga_screencapture; \
	fi

	if [ -d "$(DESTDIR)$(bindir)" ]; then \
		rmdir --ignore-fail-on-non-empty $(DESTDIR)$(bindir); \
	fi

clean:
	rm -f $(OFILES)
	rm -f salasaga_screencapture salasaga_screencapture.exe

distclean: clean
	rm -Rf Rules.make config.make Makefile.config autom4te.cache config.log config.status configure

maintainer-clean: distclean
	rm -f configure
