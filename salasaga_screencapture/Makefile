include Makefile.config

OFILES = \
	flame-capture.o

all: flame-capture

flame-capture.o:
	$(CC) $(CFLAGS) -c flame-capture.c -o flame-capture.o

install:
	$(INSTALL) -d $(bindir)
	$(INSTALL_PROGRAM) -m 0755 flame-capture $(bindir)
clean:
	rm -f $(OFILES)
	rm -f flame-capture flame-capture.exe

distclean: clean
	rm -Rf Rules.make config.make Makefile.config autom4te.cache config.log config.status configure

maintainer-clean: distclean
	rm -f configure
