bin_PROGRAMS = salasaga
salasaga_SOURCES = global_functions.c \
	cairo/create_cairo_pixbuf_pattern.c \
	cairo/render_text_string.c \
	call_back/delete_key_release_event.c \
	call_back/dump_selection_info.c \
	call_back/film_strip_drag_motion.c \
	call_back/quit_event.c \
	call_back/text_layer_dialog_bg_colour_changed.c \
	call_back/text_layer_dialog_fg_colour_changed.c \
	call_back/text_layer_dialog_font_changed.c \
	call_back/text_layer_dialog_insert_text_signal_triggered.c \
	call_back/text_layer_dialog_selection_changed.c \
	call_back/text_layer_dialog_size_changed.c \
	call_back/text_layer_dialog_validate_buffer_tag_quantity.c \
	call_back/track_window_state.c \
	call_back/transition_type_changed.c \
	dialog/display_dialog_empty.c \
	dialog/display_dialog_highlight.c \
	dialog/display_dialog_image.c \
	dialog/display_dialog_mouse.c \
	dialog/display_dialog_save_warning.c \
	dialog/display_dialog_text.c \
	dialog/display_warning.c \
	export/html/export_html_wrapper.c \
	export/swf/export_swf_choose_resolution_index.c \
	export/swf/export_swf_control_bar.c \
	export/swf/export_swf_create_layer_elements.c \
	export/swf/export_swf_create_shape.c \
	export/swf/export_swf_inner.c \
	export/swf/export_swf_process_element.c \
	export/swf/swf_add_mouse_click.c \
	export/swf/swf_shape_from_image_file.c \
	film_strip/create_film_strip.c \
	film_strip/film_strip_button_clicked.c \
	film_strip/film_strip_create_thumbnail.c \
	film_strip/film_strip_handle_changed.c \
	film_strip/film_strip_handle_released.c \
	film_strip/film_strip_slide_clicked.c \
	film_strip/regenerate_film_strip_thumbnails.c \
	layer/compress_layers.c \
	layer/compress_layers_inner.c \
	layer/get_layer_position.c \
	layer/image_crop.c \
	layer/image_resize.c \
	layer/layer_copy.c \
	layer/layer_delete.c \
	layer/layer_duplicate.c \
	layer/layer_edit.c \
	layer/layer_free.c \
	layer/layer_move_down.c \
	layer/layer_move_up.c \
	layer/layer_new_highlight.c \
	layer/layer_new_highlight_inner.c \
	layer/layer_new_image.c \
	layer/layer_new_image_inner.c \
	layer/layer_new_mouse.c \
	layer/layer_new_mouse_inner.c \
	layer/layer_new_text.c \
	layer/layer_new_text_inner.c \
	layer/layer_paste.c \
	menu/create_menu_bar.c \
	menu/menu_edit_preferences.c \
	menu/menu_edit_redo.c \
	menu/menu_edit_undo.c \
	menu/menu_enable.c \
	menu/menu_export_swf.c \
	menu/menu_export_layer.c \
	menu/menu_export_slide.c \
	menu/menu_file_close.c \
	menu/menu_file_new.c \
	menu/menu_file_open.c \
	menu/menu_file_save.c \
	menu/menu_file_save_as.c \
	menu/menu_help_about.c \
	menu/menu_help_forum.c \
	menu/menu_help_register.c \
	menu/menu_help_survey.c \
	menu/menu_help_support.c \
	menu/menu_help_website.c \
	menu/menu_help_wiki.c \
	menu/menu_project_properties.c \
	menu/menu_screenshots_capture.c \
	menu/menu_screenshots_capture_full_screen.c \
	menu/menu_screenshots_import.c \
	other/gtk_text_buffer_duplicate.c \
	other/key_bind.c \
	other/load_fonts.c \
	other/logger_simple.c \
	other/logger_with_domain.c \
	other/project_adjust_dimensions.c \
	other/validate_value.c \
	other/widget_focus.c \
	preference/application_preferences.c \
	preference/preferences_load.c \
	preference/project_preferences.c \
	preference/save_preferences_and_exit.c \
	read/read_project.c \
	read/read_empty_layer.c \
	read/read_highlight_layer.c \
	read/read_image_layer.c \
	read/read_mouse_layer.c \
	read/read_text_layer.c \
	resolution_selector/create_resolution_selector.c \
	resolution_selector/resolution_selector_changed.c \
	save/save_layer.c \
	save/save_slide.c \
	save/save_text_tags.c \
	slide/slide_delete.c \
	slide/slide_duration.c \
	slide/slide_free.c \
	slide/slide_insert.c \
	slide/slide_move_bottom.c \
	slide/slide_move_down.c \
	slide/slide_move_top.c \
	slide/slide_move_up.c \
	status_icon/create_status_icon.c \
	status_icon/status_icon_activate.c \
	status_icon/status_icon_iconify.c \
	status_icon/status_icon_popup.c \
	status_icon/status_icon_restore.c \
	status_icon/status_icon_take_screenshot.c \
	text_tags/create_text_tags.c \
	text_tags/get_selection_fg_colour.c \
	text_tags/get_selection_font_face.c \
	text_tags/get_selection_font_size.c \
	text_tags/reset_global_text_tags_table.c \
	text_tags/text_layer_create_colour_tag.c \
	text_tags/text_layer_create_font_size_tag.c \
	time_line/create_time_line.c \
	time_line/draw_timeline.c \
	time_line/time_line.c \
	time_line/time_line_class_init.c \
	time_line/time_line_get_cursor_position.c \
	time_line/time_line_get_left_border_width.c \
	time_line/time_line_get_selected_layer_num.c \
	time_line/time_line_get_type.c \
	time_line/time_line_internal_make_widgets.c \
	time_line/time_line_init.c \
	time_line/time_line_internal_draw_cursor.c \
	time_line/time_line_internal_draw_guide_line.c \
	time_line/time_line_internal_draw_layer_duration.c \
	time_line/time_line_internal_draw_layer_info.c \
	time_line/time_line_internal_draw_layer_name.c \
	time_line/time_line_internal_draw_selection_highlight.c \
	time_line/time_line_internal_initialise_bg_image.c \
	time_line/time_line_internal_initialise_display_buffer.c \
	time_line/time_line_internal_invalidate_layer_area.c \
	time_line/time_line_internal_redraw_layer_bg.c \
	time_line/time_line_internal_redraw_bg_area.c \
	time_line/top_left_button_release_event.c\
	time_line/top_left_button_press_event.c\
	time_line/top_left_motion_notify_event.c\
	time_line/top_right_button_press_event.c\
	time_line/top_right_button_release_event.c\
	time_line/top_right_motion_notify_event.c\
	time_line/bot_left_button_press_event.c\
	time_line/bot_left_button_release_event.c\
	time_line/bot_left_motion_notify_event.c\
	time_line/bot_right_button_press_event.c\
	time_line/bot_right_button_release_event.c\
	time_line/bot_right_motion_notify_event.c\
	time_line/realize_allocate_table.c\
	time_line/expose_table.c\
	time_line/size_allocate_table.c\
	time_line/expose_event_top_left.c\
	time_line/expose_event_top_right.c\
	time_line/expose_event_bot_right.c\
	time_line/expose_event_bot_left.c\
	time_line/time_line_new.c \
	time_line/time_line_regenerate_widget.c \
	time_line/time_line_set_cursor_position.c \
	time_line/time_line_set_selected_layer_num.c \
	time_line/time_line_set_stored_slide_duration.c \
	tool_bars/create_toolbar.c \
	tool_bars/disable_layer_toolbar_buttons.c \
	tool_bars/disable_main_toolbar_buttons.c \
	tool_bars/enable_layer_toolbar_buttons.c \
	tool_bars/enable_main_toolbar_buttons.c \
	tool_bars/shared_toolbar_functions.c \
	undo_redo/undo_functions.c \
	working_area/calculate_object_boundaries.c \
	working_area/create_working_area.c \
	working_area/detect_collisions.c \
	working_area/draw_bounding_box.c \
	working_area/draw_handle_box.c \
	working_area/draw_highlight_box.c \
	working_area/draw_layer_start_and_end_points.c \
	working_area/draw_resize_handles.c \
	working_area/draw_workspace.c \
	working_area/event_size_allocate_received.c \
	working_area/working_area_button_press_event.c \
	working_area/working_area_button_release_event.c \
	working_area/working_area_expose_event.c \
	working_area/working_area_motion_notify_event.c \
	zoom_selector/create_zoom_selector.c \
	zoom_selector/zoom_selector_changed.c \
	../salasaga.c
salasaga_LDADD = $(DEPS_LIBS) $(GTK_LIBS) $(ZLIB) $(XLIB) $(MINGLIB) $(LIBXML_LIBS) $(GCONF_LIBS) $(LIBGNOME_LIBS) 
AM_CPPFLAGS = $(DEPS_CFLAGS) $(GTK_CFLAGS) $(LIBXML_CFLAGS) $(LIBGNOME_CFLAGS) \
	-DFONT_OUTLINE_DIR=\""$(DESTDIR)/$(datadir)/salasaga/font_outline"\" \
	-DFONT_TTF_DIR=\""$(DESTDIR)/$(datadir)/salasaga/ttf"\" \
	-DIMAGES_PNG_DIR=\""$(DESTDIR)/$(datadir)/salasaga/icons/72x72"\" \
	-DIMAGES_SVG_DIR=\""$(DESTDIR)/$(datadir)/salasaga/icons/scalable"\" \
	-DLOCALEDIR=\""$(DESTDIR)/$(datadir)/locale"\" \
	-DMOUSE_PTR_PNG_DIR=\""$(DESTDIR)/$(datadir)/salasaga/icons/72x72/pointers"\" \
	-DMOUSE_PTR_SVG_DIR=\""$(DESTDIR)/$(datadir)/salasaga/icons/scalable/pointers"\" \
	-DSOUND_DIR=\""$(DESTDIR)/$(datadir)/salasaga/sounds/"\" \
	-DSTATUS_ICON_DIR=\""$(DESTDIR)/$(datadir)/pixmaps/"\"

DISTCLEANFILES = Makefile.in
