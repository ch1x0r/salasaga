include Makefile.config

OFILES = \
	functions/base64_decode.o \
	functions/base64_encode.o \
	functions/calculate_object_boundaries.o \
	functions/compress_layers.o \
	functions/compress_layers_inner.o \
	functions/construct_timeline_widget.o \
	functions/create_film_strip.o \
	functions/create_menu_bar.o \
	functions/create_resolution_selector.o \
	functions/create_time_line.o \
	functions/create_timeline_slider.o \
	functions/create_toolbar.o \
	functions/create_tooltips.o \
	functions/create_working_area.o \
	functions/destroy_slide.o \
	functions/detect_collisions.o \
	functions/disable_layer_toolbar_buttons.o \
	functions/disable_main_toolbar_buttons.o \
	functions/display_dialog_empty.o \
	functions/display_dialog_highlight.o \
	functions/display_dialog_image.o \
	functions/display_dialog_mouse.o \
	functions/display_dialog_text.o \
	functions/display_warning.o \
	functions/draw_bounding_box.o \
	functions/draw_highlight_box.o \
	functions/draw_timeline.o \
	functions/draw_workspace.o \
	functions/enable_layer_toolbar_buttons.o \
	functions/enable_main_toolbar_buttons.o \
	functions/event_size_allocate_received.o \
	functions/film_strip_handle_changed.o \
	functions/film_strip_handle_released.o \
	functions/film_strip_slide_clicked.o \
	functions/flame_read.o \
	functions/flash_create_tag_bitmap.o \
	functions/flash_create_tag_mouse.o \
	functions/flash_create_tag_text.o \
	functions/flash_create_tag_highlight.o \
	functions/image_crop.o \
	functions/layer_delete.o \
	functions/layer_edit.o \
	functions/layer_move_down.o \
	functions/layer_move_up.o \
	functions/layer_new_highlight.o \
	functions/layer_new_image.o \
	functions/layer_new_mouse.o \
	functions/layer_new_text.o \
	functions/logger_simple.o \
	functions/logger_with_domain.o \
	functions/menu_edit_preferences.o \
	functions/menu_enable.o \
	functions/menu_export_flash_animation.o \
	functions/menu_export_flash_inner.o \
	functions/menu_export_svg_animation.o \
	functions/menu_export_svg_animation_slide.o \
	functions/menu_file_new.o \
	functions/menu_file_open.o \
	functions/menu_file_save.o \
	functions/menu_file_save_layer.o \
	functions/menu_file_save_slide.o \
	functions/menu_help_about.o \
	functions/menu_help_register.o \
	functions/menu_help_survey.o \
	functions/menu_help_website.o \
	functions/menu_screenshots_capture.o \
	functions/menu_screenshots_import.o \
	functions/project_crop.o \
	functions/quit_event.o \
	functions/regenerate_film_strip_thumbnails.o \
	functions/regenerate_timeline_duration_images.o \
	functions/resolution_selector_changed.o \
	functions/save_preferences_and_exit.o \
	functions/slide_delete.o \
	functions/slide_insert.o \
	functions/slide_move_bottom.o \
	functions/slide_move_down.o \
	functions/slide_move_top.o \
	functions/slide_move_up.o \
	functions/slide_name_reset.o \
	functions/slide_name_set.o \
	functions/sound_beep.o \
	functions/timeline_edited_name.o \
	functions/timeline_edited_x_offset_finish.o \
	functions/timeline_edited_x_offset_start.o \
	functions/timeline_edited_y_offset_finish.o \
	functions/timeline_edited_y_offset_start.o \
	functions/uri_decode.o \
	functions/uri_encode_base64.o \
	functions/working_area_button_press_event.o \
	functions/working_area_button_release_event.o \
	functions/working_area_expose_event.o \
	functions/working_area_motion_notify_event.o \
	functions/zoom_selector_changed.o \
	flame-edit.o

SOUNDS = \
	generic.wav

ICON_MAIN = \
	flame-icon.ico

ICON_PNG = \
	flame-icon.png

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

MOUSE_SCALABLE = \
	standard.svg

MOUSE_PNG = \
	standard.png

DESKTOP_FILE = \
	flameproject.desktop

all: flame-edit

base64_decode.o:
	$(CC) $(CFLAGS) -c base64_decode.c -o base64_decode.o

base64_encode.o:
	$(CC) $(CFLAGS) -c base64_encode.c -o base64_encode.o

calculate_object_boundaries.o:
	$(CC) $(CFLAGS) -c calculate_object_boundaries.c -o calculate_object_boundaries.o

compress_layers.o:
	$(CC) $(CFLAGS) -c compress_layers.c -o compress_layers.o

compress_layers_inner.o:
	$(CC) $(CFLAGS) -c compress_layers_inner.c -o compress_layers_inner.o

construct_timeline_widget.o:
	$(CC) $(CFLAGS) -c construct_timeline_widget.c -o construct_timeline_widget.o

create_film_strip.o:
	$(CC) $(CFLAGS) -c create_film_strip.c -o create_film_strip.o

create_menu_bar.o:
	$(CC) $(CFLAGS) -c create_menu_bar.c -o create_menu_bar.o

create_resolution_selector.o:
	$(CC) $(CFLAGS) -c create_resolution_selector.c -o create_resolution_selector.o

create_time_line.o:
	$(CC) $(CFLAGS) -c create_time_line.c -o create_time_line.o

create_timeline_slider.o:
	$(CC) $(CFLAGS) -c create_timeline_slider.c -o create_timeline_slider.o

create_toolbar.o:
	$(CC) $(CFLAGS) -c create_toolbar.c -o create_toolbar.o

create_tooltips.o:
	$(CC) $(CFLAGS) -c create_tooltips.c -o create_tooltips.o

create_working_area.o:
	$(CC) $(CFLAGS) -c create_working_area.c -o create_working_area.o

destroy_slide.o:
	$(CC) $(CFLAGS) -c destroy_slide.c -o destroy_slide.o

detect_collisions.o:
	$(CC) $(CFLAGS) -c detect_collisions.c -o detect_collisions.o

disable_layer_toolbar_buttons.o:
	$(CC) $(CFLAGS) -c disable_layer_toolbar_buttons.c -o disable_layer_toolbar_buttons.o

disable_main_toolbar_buttons.o:
	$(CC) $(CFLAGS) -c disable_main_toolbar_buttons.c -o disable_main_toolbar_buttons.o

display_dialog_empty.o:
	$(CC) $(CFLAGS) -c display_dialog_empty.c -o display_dialog_empty.o

display_dialog_highlight.o:
	$(CC) $(CFLAGS) -c display_dialog_highlight.c -o display_dialog_highlight.o

display_dialog_image.o:
	$(CC) $(CFLAGS) -c display_dialog_image.c -o display_dialog_image.o

display_dialog_mouse.o:
	$(CC) $(CFLAGS) -c display_dialog_mouse.c -o display_dialog_mouse.o

display_dialog_text.o:
	$(CC) $(CFLAGS) -c display_dialog_text.c -o display_dialog_text.o

display_warning.o:
	$(CC) $(CFLAGS) -c display_warning.c -o display_warning.o

draw_bounding_box.o:
	$(CC) $(CFLAGS) -c draw_bounding_box.c -o draw_bounding_box.o

draw_highlight_box.o:
	$(CC) $(CFLAGS) -c draw_highlight_box.c -o draw_highlight_box.o

draw_timeline.o:
	$(CC) $(CFLAGS) -c draw_timeline.c -o draw_timeline.o

draw_workspace.o:
	$(CC) $(CFLAGS) -c draw_workspace.c -o draw_workspace.o

enable_layer_toolbar_buttons.o:
	$(CC) $(CFLAGS) -c enable_layer_toolbar_buttons.c -o enable_layer_toolbar_buttons.o

enable_main_toolbar_buttons.o:
	$(CC) $(CFLAGS) -c enable_main_toolbar_buttons.c -o enable_main_toolbar_buttons.o

event_size_allocate_received.o:
	$(CC) $(CFLAGS) -c event_size_allocate_received.c -o event_size_allocate_received.o

film_strip_handle_changed.o:
	$(CC) $(CFLAGS) -c film_strip_handle_changed.c -o film_strip_handle_changed.o

film_strip_handle_released.o:
	$(CC) $(CFLAGS) -c film_strip_handle_released.c -o film_strip_handle_released.o

film_strip_slide_clicked.o:
	$(CC) $(CFLAGS) -c film_strip_slide_clicked.c -o film_strip_slide_clicked.o

flame_read.o:
	$(CC) $(CFLAGS) -c flame_read.c -o flame_read.o

flash_create_tag_bitmap.o:
	$(CC) $(CFLAGS) -c .sh_create_tag_bitmapc -o flash_create_tag_bitmap.o

flash_create_tag_mouse.o:
	$(CC) $(CFLAGS) -c flash_create_tag_mouse.c -o flash_create_tag_mouse.o

flash_create_tag_text.o:
	$(CC) $(CFLAGS) -c flash_create_tag_text.c -o flash_create_tag_text.o

flash_create_tag_highlight.o:
	$(CC) $(CFLAGS) -c flash_create_tag_highlight.c -o flash_create_tag_highlight.o

image_crop.o:
	$(CC) $(CFLAGS) -c image_crop.c -o image_crop.o

layer_delete.o:
	$(CC) $(CFLAGS) -c layer_delete.c -o layer_delete.o

layer_edit.o:
	$(CC) $(CFLAGS) -c layer_edit.c -o layer_edit.o

layer_move_down.o:
	$(CC) $(CFLAGS) -c layer_move_down.c -o layer_move_down.o

layer_move_up.o:
	$(CC) $(CFLAGS) -c layer_move_up.c -o layer_move_up.o

layer_new_highlight.o:
	$(CC) $(CFLAGS) -c layer_new_highlight.c -o layer_new_highlight.o

layer_new_image.o:
	$(CC) $(CFLAGS) -c layer_new_image.c -o layer_new_image.o

layer_new_mouse.o:
	$(CC) $(CFLAGS) -c layer_new_mouse.c -o layer_new_mouse.o

layer_new_text.o:
	$(CC) $(CFLAGS) -c layer_new_text.c -o layer_new_text.o

logger_simple.o:
	$(CC) $(CFLAGS) -c logger_simple.c -o logger_simple.o

logger_with_domain.o:
	$(CC) $(CFLAGS) -c logger_with_domain.c -o logger_with_domain.o

menu_edit_preferences.o:
	$(CC) $(CFLAGS) -c menu_edit_preferences.c -o menu_edit_preferences.o

menu_enable.o:
	$(CC) $(CFLAGS) -c menu_enable.c -o menu_enable.o

menu_export_flash_animation.o:
	$(CC) $(CFLAGS) -c menu_export_flash_animation.c -o menu_export_flash_animation.o

menu_export_flash_inner.o:
	$(CC) $(CFLAGS) -c menu_export_flash_inner.c -o menu_export_flash_inner.o

menu_export_svg_animation.o:
	$(CC) $(CFLAGS) -c menu_export_svg_animation.c -o menu_export_svg_animation.o

menu_export_svg_animation_slide.o:
	$(CC) $(CFLAGS) -c menu_export_svg_animation_slide.c -o menu_export_svg_animation_slide.o

menu_file_new.o:
	$(CC) $(CFLAGS) -c menu_file_new.c -o menu_file_new.o

menu_file_open.o:
	$(CC) $(CFLAGS) -c menu_file_open.c -o menu_file_open.o

menu_file_save.o:
	$(CC) $(CFLAGS) -c menu_file_save.c -o menu_file_save.o

menu_file_save_layer.o:
	$(CC) $(CFLAGS) -c menu_file_save_layer.c -o menu_file_save_layer.o

menu_file_save_slide.o:
	$(CC) $(CFLAGS) -c menu_file_save_slide.c -o menu_file_save_slide.o

menu_help_about.o:
	$(CC) $(CFLAGS) -c menu_help_about.c -o menu_help_about.o

menu_help_register.o:
	$(CC) $(CFLAGS) -c menu_help_register.c -o menu_help_register.o

menu_help_survey.o:
	$(CC) $(CFLAGS) -c menu_help_survey.c -o menu_help_survey.o

menu_help_website.o:
	$(CC) $(CFLAGS) -c menu_help_website.c -o menu_help_website.o

menu_screenshots_capture.o:
	$(CC) $(CFLAGS) -c menu_screenshots_capture.c -o menu_screenshots_capture.o

menu_screenshots_import.o:
	$(CC) $(CFLAGS) -c menu_screenshots_import.c -o menu_screenshots_import.o

project_crop.o:
	$(CC) $(CFLAGS) -c project_crop.c -o project_crop.o

quit_event.o:
	$(CC) $(CFLAGS) -c quit_event.c -o quit_event.o

regenerate_film_strip_thumbnails.o:
	$(CC) $(CFLAGS) -c regenerate_film_strip_thumbnails.c -o regenerate_film_strip_thumbnails.o

regenerate_timeline_duration_images.o:
	$(CC) $(CFLAGS) -c regenerate_timeline_duration_images.c -o regenerate_timeline_duration_images.o

resolution_selector_changed.o:
	$(CC) $(CFLAGS) -c resolution_selector_changed.c -o resolution_selector_changed.o

save_preferences_and_exit.o:
	$(CC) $(CFLAGS) -c save_preferences_and_exit.c -o save_preferences_and_exit.o

slide_delete.o:
	$(CC) $(CFLAGS) -c slide_delete.c -o slide_delete.o

slide_insert.o:
	$(CC) $(CFLAGS) -c slide_insert.c -o slide_insert.o

slide_move_bottom.o:
	$(CC) $(CFLAGS) -c slide_move_bottom.c -o slide_move_bottom.o

slide_move_down.o:
	$(CC) $(CFLAGS) -c slide_move_down.c -o slide_move_down.o

slide_move_top.o:
	$(CC) $(CFLAGS) -c slide_move_top.c -o slide_move_top.o

slide_move_up.o:
	$(CC) $(CFLAGS) -c slide_move_up.c -o slide_move_up.o

slide_name_reset.o:
	$(CC) $(CFLAGS) -c slide_name_reset.c -o slide_name_reset.o

slide_name_set.o:
	$(CC) $(CFLAGS) -c slide_name_set.c -o slide_name_set.o

sound_beep.o:
	$(CC) $(CFLAGS) -c sound_beep.c -o sound_beep.o

timeline_edited_name.o:
	$(CC) $(CFLAGS) -c timeline_edited_name.c -o timeline_edited_name.o

timeline_edited_x_offset_finish.o:
	$(CC) $(CFLAGS) -c timeline_edited_x_offset_finish.c -o timeline_edited_x_offset_finish.o

timeline_edited_x_offset_start.o:
	$(CC) $(CFLAGS) -c timeline_edited_x_offset_start.c -o timeline_edited_x_offset_start.o

timeline_edited_y_offset_finish.o:
	$(CC) $(CFLAGS) -c timeline_edited_y_offset_finish.c -o timeline_edited_y_offset_finish.o

timeline_edited_y_offset_start.o:
	$(CC) $(CFLAGS) -c timeline_edited_y_offset_start.c -o timeline_edited_y_offset_start.o

uri_decode.o:
	$(CC) $(CFLAGS) -c uri_decode.c -o uri_decode.o

uri_encode_base64.o:
	$(CC) $(CFLAGS) -c uri_encode_base64.c -o uri_encode_base64.o

working_area_button_press_event.o:
	$(CC) $(CFLAGS) -c working_area_button_press_event.c -o working_area_button_press_event.o

working_area_button_release_event.o:
	$(CC) $(CFLAGS) -c working_area_button_release_event.c -o working_area_button_release_event.o

working_area_expose_event.o:
	$(CC) $(CFLAGS) -c working_area_expose_event.c -o working_area_expose_event.o

working_area_motion_notify_event.o:
	$(CC) $(CFLAGS) -c working_area_motion_notify_event.c -o working_area_motion_notify_event.o

zoom_selector_changed.o:
	$(CC) $(CFLAGS) -c zoom_selector_changed.c -o zoom_selector_changed.o

flame-edit.o:
	$(CC) $(CFLAGS) -c flame-edit.c -o flame-edit.o

flame-edit: $(OFILES)
	$(CC) $(CFLAGS) $(OFILES) $(LDFLAGS) $(LIBS) -o flame-edit

install:
	$(INSTALL) -d $(bindir)
	$(INSTALL_PROGRAM) -m 0755 flame-edit $(bindir)
	$(INSTALL) -d $(datadir)/applications/
	$(INSTALL_DATA) $(DESKTOP_FILE) $(datadir)/applications/
	$(INSTALL) -d $(datadir)/flame/icons/
	$(INSTALL) -d $(datadir)/pixmaps/
	cd images; $(INSTALL_DATA) $(ICON_MAIN) $(datadir)/flame/icons/
	cd images; $(INSTALL_DATA) $(ICON_PNG) $(datadir)/pixmaps/
	$(INSTALL) -d $(datadir)/flame/icons/scalable/
	cd images; $(INSTALL_DATA) $(ICONS_SCALABLE) $(datadir)/flame/icons/scalable/
	$(INSTALL) -d $(datadir)/flame/icons/scalable/pointers/
	cd images/pointers; $(INSTALL_DATA) $(MOUSE_SCALABLE) $(datadir)/flame/icons/scalable/pointers/
	$(INSTALL) -d $(datadir)/flame/icons/72x72/
	cd images; $(INSTALL_DATA) $(ICONS_SIXTY) $(datadir)/flame/icons/72x72/
	$(INSTALL) -d $(datadir)/flame/icons/72x72/pointers/
	cd images/pointers; $(INSTALL_DATA) $(MOUSE_PNG) $(datadir)/flame/icons/72x72/pointers/
	$(INSTALL) -d $(datadir)/flame/sounds/
	cd sounds; $(INSTALL_DATA) $(SOUNDS) $(datadir)/flame/sounds/

uninstall:
	if [ -f "$(bindir)/flame-edit" ]; then \
		rm -f $(bindir)/flame-edit; \
	fi

	if [ -d "$(datadir)/flame/icons/scalable" ]; then \
		cd $(datadir)/flame/icons/scalable/pointers/; \
		rm -f $(MOUSE_SCALABLE); \
		rmdir $(datadir)/flame/icons/scalable/pointers; \
		cd $(datadir)/flame/icons/scalable/; \
		rm -f $(ICONS_SCALABLE); \
		rmdir $(datadir)/flame/icons/scalable; \
	fi

	if [ -d "$(datadir)/flame/icons/72x72" ]; then \
		cd $(datadir)/flame/icons/72x72/pointers/; \
		rm -f $(MOUSE_PNG); \
		rmdir $(datadir)/flame/icons/72x72/pointers; \
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

	if [ -f "$(datadir)/pixmaps/$(ICON_PNG)" ]; then \
		rm -f $(datadir)/pixmaps/$(ICON_PNG); \
		rmdir $(datadir)/pixmaps; \
	fi

	if [ -f "$(datadir)/applications/$(DESKTOP_FILE)" ]; then \
		rm -f $(datadir)/applications/$(DESKTOP_FILE); \
		rmdir $(datadir)/applications; \
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
