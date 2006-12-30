include Makefile.config

OFILES = \
	backend.o \
	callbacks.o \
	flame-edit.o \
	gui-functions.o

SOUNDS = \
	sounds/generic.wav

ICONS_SIXTY = \
	images/add_highlight.png \
	images/add_image.png \
	images/add_mouse.png \
	images/add_text.png \
	images/capture.png \
	images/crop.png \
	images/delete.png \
	images/down_arrow.png \
	images/edit.png \
	images/export_flash.png \
	images/export_svg.png \
	images/import.png \
	images/up_arrow.png \

ICONS_SCALABLE = \
	images/add_highlight.svg \
	images/add_image.svg \
	images/add_mouse.svg \
	images/add_text.svg \
	images/capture.svg \
	images/crop.svg \
	images/delete.svg \
	images/down_arrow.svg \
	images/edit.svg \
	images/export_flash.svg \
	images/export_svg.svg \
	images/import.svg \
	images/up_arrow.svg \

all: flame-edit

backend.o:
	$(CC) $(CFLAGS) -c backend.c -o backend.o

callbacks.o:
	$(CC) $(CFLAGS) -c callbacks.c -o callbacks.o

flame-edit.o:
	$(CC) $(CFLAGS) -c flame-edit.c -o flame-edit.o

gui-functions.o:
	$(CC) $(CFLAGS) -c gui-functions.c -o gui-functions.o

flame-edit: $(OFILES)
	$(CC) $(CFLAGS) $(OFILES) $(LDFLAGS) $(LIBS) -o flame-edit

install:
	$(INSTALL) -d $(bindir)
	$(INSTALL_PROGRAM) -m 0755 flame-edit $(bindir)
	$(INSTALL) -d $(datadir)/flame/icons/scalable/
	$(INSTALL_DATA) $(ICONS_SCALABLE) $(datadir)/flame/icons/scalable/
	$(INSTALL) -d $(datadir)/flame/icons/72x72/
	$(INSTALL_DATA) $(ICONS_SIXTY) $(datadir)/flame/icons/72x72/
	$(INSTALL) -d $(datadir)/flame/sounds/
	$(INSTALL_DATA) $(SOUNDS) $(datadir)/flame/sounds/

clean:
	rm -f $(OFILES)
	rm -f flame-edit flame-edit.exe

distclean: clean
	rm -Rf Rules.make config.make Makefile.config autom4te.cache config.log config.status configure

maintainer-clean: distclean
	rm -f configure
