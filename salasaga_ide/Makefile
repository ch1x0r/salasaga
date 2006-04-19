include Makefile.config

OFILES = \
	backend.o \
	callbacks.o \
	flame-edit.o \
	gui-functions.o


all: flame-edit

$(OFILES):
	$(CC) $(CFLAGS) backend.c $(LDFLAGS) -o 

backend.o:
	$(CC) $(CFLAGS) -c backend.c -o backend.o

callbacks.o:
	$(CC) $(CFLAGS) -c callbacks.c -o callbacks.o

flame-edit.o:
	$(CC) $(CFLAGS) -c flame-edit.c -o flame-edit.o

gui-functions.o:
	$(CC) $(CFLAGS) -c gui-functions.c -o gui-functions.o

flame-edit: $(OFILES)
	$(CC) $(CFLAGS) $(OFILES) $(LDFLAGS) -o flame-edit

install: 
	$(INSTALL_PROGRAM) -m 0755 flame-edit $(bindir)

clean:
	rm -f $(OFILES)
	rm -f flame-edit flame-edit.exe

distclean: clean
	rm -Rf Rules.make config.make Makefile.config autom4te.cache config.log config.status

maintainer-clean: distclean
	rm -f configure
