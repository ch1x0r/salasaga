include Makefile.config

all: flame_capture/Makefile.config flame_edit/Makefile.config

flame_capture/Makefile.config:
	cd flame_capture; autoconf; ./configure --prefix=${prefix}; make

flame_edit/Makefile.config:
	cd flame_edit; autoconf; ./configure --prefix=${prefix}; make

install: flame_capture/Makefile.config flame_edit/Makefile.config
	cd flame_capture; make install
	cd flame_edit; make install

uninstall: flame_capture/Makefile.config flame_edit/Makefile.config
	cd flame_capture; make uninstall
	cd flame_edit; make uninstall

clean: flame_capture/Makefile.config flame_edit/Makefile.config
	cd flame_capture; make clean
	cd flame_edit; make clean

distclean: flame_capture/Makefile.config flame_edit/Makefile.config
	cd flame_capture; make distclean
	cd flame_edit; make distclean
	rm -f configure config.log config.status Makefile.config autom4te.cache/*
	rmdir --ignore-fail-on-non-empty autom4te.cache

