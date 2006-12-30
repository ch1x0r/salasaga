include Makefile.config

OFILES = \
	backend.o \
	callbacks.o \
	flame-edit.o \
	gui-functions.o

SOUNDS = \
	generic.wav

ICON_MAIN = \
	flame_logo.ico

ICONS_SIXTY = \
	add_highlight.png \
	add_highlight_grayed.png \
	add_image.png \
	add_image_grayed.png \
	add_mouse.png \
	add_mouse_grayed.png \
	add_text.png \
	add_text_grayed.png \
	capture.png \
	capture_grayed.png \
	crop.png \
	crop_grayed.png \
	delete.png \
	delete_grayed.png \
	down_arrow.png \
	down_arrow_grayed.png \
	edit.png \
	edit_grayed.png \
	export_flash.png \
	export_flash_grayed.png \
	export_svg.png \
	export_svg_grayed.png \
	flame_logo.png \
	import.png \
	import_grayed.png \
	up_arrow.png \
	up_arrow_grayed.png

ICONS_SCALABLE = \
	add_highlight.svg \
	add_highlight_grayed.svg \
	add_image.svg \
	add_image_grayed.svg \
	add_mouse.svg \
	add_mouse_grayed.svg \
	add_text.svg \
	add_text_grayed.svg \
	capture.svg \
	capture_grayed.svg \
	crop.svg \
	crop_grayed.svg \
	delete.svg \
	delete_grayed.svg \
	down_arrow.svg \
	down_arrow_grayed.svg \
	edit.svg \
	edit_grayed.svg \
	export_flash.svg \
	export_flash_grayed.svg \
	export_svg.svg \
	export_svg_grayed.svg \
	flame_logo.svg \
	import.svg \
	import_grayed.svg \
	up_arrow.svg \
	up_arrow_grayed.svg

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
	$(INSTALL) -d $(datadir)/flame/icons/
	cd images; $(INSTALL_DATA) $(ICON_MAIN) $(datadir)/flame/icons/
	$(INSTALL) -d $(datadir)/flame/icons/scalable/
	cd images; $(INSTALL_DATA) $(ICONS_SCALABLE) $(datadir)/flame/icons/scalable/
	$(INSTALL) -d $(datadir)/flame/icons/72x72/
	cd images; $(INSTALL_DATA) $(ICONS_SIXTY) $(datadir)/flame/icons/72x72/
	$(INSTALL) -d $(datadir)/flame/sounds/
	cd sounds; $(INSTALL_DATA) $(SOUNDS) $(datadir)/flame/sounds/

uninstall:
	if [ -f "$(bindir)/flame-edit" ]; then \
		rm -f $(bindir)/flame-edit; \
	fi

	if [ -d "$(datadir)/flame/icons/scalable" ]; then \
		cd $(datadir)/flame/icons/scalable/; \
		rm -f $(ICONS_SCALABLE); \
		rmdir $(datadir)/flame/icons/scalable; \
	fi

	if [ -d "$(datadir)/flame/icons/72x72" ]; then \
		cd $(datadir)/flame/icons/72x72/; \
		rm -f $(ICONS_SIXTY); \
		rmdir $(datadir)/flame/icons/72x72; \
	fi

	if [ -d "$(datadir)/flame/sounds" ]; then \
		cd $(datadir)/flame/sounds/; \
		rm -f $(SOUNDS); \
		rmdir $(datadir)/flame/sounds; \
	fi

	if [ -f "$(datadir)/flame/icons/$(ICON_MAIN)" ]; then \
		rm -f $(datadir)/flame/icons/$(ICON_MAIN); \
	fi

	if [ -d "$(datadir)/flame/icons" ]; then \
		rmdir $(datadir)/flame/icons; \
	fi

	if [ -d "$(datadir)/flame" ]; then \
		rmdir $(datadir)/flame; \
	fi

	if [ -d "$(bindir)" ]; then \
		rmdir --ignore-fail-on-non-empty $(bindir); \
	fi

	if [ -d "$(datadir)" ]; then \
		rmdir --ignore-fail-on-non-empty $(datadir); \
	fi

clean:
	rm -f $(OFILES)
	rm -f flame-edit flame-edit.exe

distclean: clean
	rm -Rf Rules.make config.make Makefile.config autom4te.cache config.log config.status configure

maintainer-clean: distclean
	rm -f configure
