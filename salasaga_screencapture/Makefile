include Makefile.config

OFILES = \
	flame-capture.o

all: flame-capture

flame-capture.o:
	$(CC) $(CFLAGS) -c flame-capture.c -o flame-capture.o

install:
	$(INSTALL) -d $(bindir)
	$(INSTALL_PROGRAM) -m 0755 flame-capture $(bindir)

uninstall:
	if [ -f "$(bindir)/flame-capture" ]; then \
		rm -f $(bindir)/flame-capture; \
	fi

	if [ -d "$(bindir)" ]; then \
		rmdir --ignore-fail-on-non-empty $(bindir); \
	fi

clean:
	rm -f $(OFILES)
	rm -f flame-capture flame-capture.exe

distclean: clean
	rm -Rf Rules.make config.make Makefile.config autom4te.cache config.log config.status configure

maintainer-clean: distclean
	rm -f configure
