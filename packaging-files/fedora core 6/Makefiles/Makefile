include Makefile.config

all: salasaga_screencapture/Makefile.config salasaga_ide/Makefile.config

salasaga_screen/Makefile.config:
	cd salasaga_screen; autoconf; ./configure --prefix=${prefix}; make

salasaga_ide/Makefile.config:
	cd salasaga_ide; autoconf; ./configure --prefix=${prefix}; make

install: salasaga_screen/Makefile.config salasaga_ide/Makefile.config
	cd salasaga_screen; make install
	cd salasaga_ide; make install

uninstall: salasaga_screen/Makefile.config salasaga_ide/Makefile.config
	cd salasaga_screen; make uninstall
	cd salasaga_ide; make uninstall

clean: salasaga_screen/Makefile.config salasaga_ide/Makefile.config
	cd salasaga_screen; make clean
	cd salasaga_ide; make clean

distclean: salasaga_screen/Makefile.config salasaga_ide/Makefile.config
	cd salasaga_screen; make distclean
	cd salasaga_ide; make distclean
	rm -f configure config.log config.status Makefile.config autom4te.cache/*
	rmdir --ignore-fail-on-non-empty autom4te.cache

