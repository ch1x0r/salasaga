include Makefile.config

all: salasaga_screencapture/Makefile.config salasaga_ide/Makefile.config

salasaga_screencapture/Makefile.config:
	cd salasaga_screencapture; autoconf; CFLAGS="$(CFLAGS)" CPPFLAGS="$(CPPFLAGS)" CXXFLAGS="$(CXXFLAGS)" CPP="$(CPP)" CXX="$(CXX)" F90FLAGS="$(F90FLAGS)" LDFLAGS="$(LDFLAGS)" FCFLAGS="$(FCFLAGS)" OBJC="$(OBJC)" INSTALL="$(INSTALL)" OBJCFLAGS="$(OBJCFLAGS)" FFLAGS="$(FFLAGS)" CC="$(CC)" ./configure --prefix=${prefix}; make

salasaga_ide/Makefile.config:
	cd salasaga_ide; autoconf; CFLAGS="$(CFLAGS)" CPPFLAGS="$(CPPFLAGS)" CXXFLAGS="$(CXXFLAGS)" CPP="$(CPP)" CXX="$(CXX)" F90FLAGS="$(F90FLAGS)" LDFLAGS="$(LDFLAGS)" FCFLAGS="$(FCFLAGS)" OBJC="$(OBJC)" INSTALL="$(INSTALL)" OBJCFLAGS="$(OBJCFLAGS)" FFLAGS="$(FFLAGS)" CC="$(CC)" ./configure --prefix=${prefix}; make

install: salasaga_screencapture/Makefile.config salasaga_ide/Makefile.config
	cd salasaga_screencapture; make install
	cd salasaga_ide; make install

uninstall: salasaga_screencapture/Makefile.config salasaga_ide/Makefile.config
	cd salasaga_screencapture; make uninstall
	cd salasaga_ide; make uninstall

clean: salasaga_screencapture/Makefile.config salasaga_ide/Makefile.config
	cd salasaga_screencapture; make clean
	cd salasaga_ide; make clean

distclean: salasaga_screencapture/Makefile.config salasaga_ide/Makefile.config
	cd salasaga_screencapture; make distclean
	cd salasaga_ide; make distclean
	rm -f configure config.log config.status Makefile.config autom4te.cache/*
	rmdir --ignore-fail-on-non-empty autom4te.cache

