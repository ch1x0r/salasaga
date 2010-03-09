/*
 * $Id$
 *
 * Copyright (C) 2005-2010 Justin Clift <justin@salasaga.org>
 *
 * This file is part of Salasaga.
 *
 * Salasaga is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 */

#ifndef GLOBAL_FUNCTIONS_H_
#define GLOBAL_FUNCTIONS_H_

// FreeType includes
#include <ft2build.h>
#include FT_FREETYPE_H

// Ming include
#include <ming.h>

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


GList *get_boundary_list();
cairo_font_face_t *get_cairo_font_face(guint index);
guint get_capture_height();
guint get_capture_width();
guint get_capture_x();
guint get_capture_y();
gboolean get_changes_made();
layer *get_copy_layer();
GList *get_current_slide();
slide *get_current_slide_data();
gfloat get_current_slide_duration();
GList *get_current_slide_layers_pointer();
gint get_current_slide_num_layers();
GtkWidget *get_current_slide_timeline_widget();
guint get_debug_level();
GdkColor get_default_text_fg_colour();
gint get_default_text_font_face();
gdouble get_default_text_font_size();
gboolean get_display_help_text();
guint get_end_behaviour();
guint get_end_point_status();
SWFFont get_fdb_font_object(guint index);
gchar *get_file_name();
gsize get_file_name_length();
gboolean get_film_strip_being_resized();
GtkTreeViewColumn *get_film_strip_column();
GtkScrolledWindow *get_film_strip_container();
GtkListStore *get_film_strip_store();
GtkWidget *get_film_strip_view();
guint get_frames_per_second();
GdkPixmap *get_front_store();
FT_Face get_ft_font_face(guint index);
FT_Face *get_ft_font_face_ptr(guint index);
gchar *get_icon_extension();
gsize get_icon_extension_length();
gchar *get_icon_path();
gsize get_icon_path_length();
gboolean get_info_display();
GtkTextBuffer *get_info_text();
gint get_invalidation_end_x();
gint get_invalidation_end_y();
gint get_invalidation_start_x();
gint get_invalidation_start_y();
gchar *get_last_folder();
gsize get_last_folder_length();
GtkWidget *get_main_area();
GtkWidget *get_main_drawing_area();
GtkWidget *get_main_window();
GtkItemFactory *get_menu_bar();
GtkTable *get_message_bar();
gboolean get_mouse_click_double_added();
gboolean get_mouse_click_single_added();
gboolean get_mouse_click_triple_added();
gboolean get_mouse_dragging();
GdkPixbuf *get_mouse_ptr_pixbuf();
gchar *get_mouse_ptr_string();
gsize get_mouse_ptr_string_length();
gboolean get_new_layer_selected();
gboolean get_project_active();
guint get_resize_handle_size();
GdkRectangle *get_resize_handles_rect(guint index);
gint get_resize_handles_rect_height(guint index);
gint get_resize_handles_rect_width(guint index);
gint get_resize_handles_rect_x(guint index);
gint get_resize_handles_rect_y(guint index);
guint get_resize_handles_status();
gulong get_resolution_callback();
GtkComboBox *get_resolution_selector();
GtkWidget *get_right_side();
gchar *get_salasaga_font_name(guint index);
gint get_screenshot_command_num();
guint get_screenshot_delay_time();
gboolean get_screenshot_key_warning();
gboolean get_screenshots_enabled();
gboolean get_show_control_bar();
GList *get_slides();
guint get_start_behaviour();
GtkWidget *get_status_bar();
GtkStatusIcon *get_status_icon();
gint get_stored_x();
gint get_stored_y();
gint get_table_x_padding();
gint get_table_y_padding();
GSList *get_text_tags_fg_colour_slist();
GtkTextTag *get_text_tags_font(guint index);
GSList *get_text_tags_size_slist();
GtkTextTagTable *get_text_tags_table();
GtkWidget *get_time_line_container();
GtkWidget *get_time_line_vbox();
guint get_valid_fields_base_type(guint index);
guint get_valid_fields_capabilities(guint index);
gfloat get_valid_fields_min_value(guint index);
gfloat get_valid_fields_max_value(guint index);
gchar *get_valid_fields_name(guint index);
guint get_working_height();
guint get_working_width();
guint get_zoom();
GtkComboBox *get_zoom_selector();
void set_boundary_list(GList *new_boundary_list);
void set_cairo_font_face(guint index, cairo_font_face_t *new_cairo_font_face);
void set_capture_height(guint new_capture_height);
void set_capture_width(guint new_capture_width);
void set_capture_x(guint new_capture_x);
void set_capture_y(guint new_capture_y);
void set_changes_made(gboolean new_changes_made);
void set_copy_layer(layer *new_copy_layer);
void set_current_slide(GList *new_current_slide);
void set_current_slide_data(slide *new_data);
void set_current_slide_layers_pointer(GList *new_layer_pointer);
void set_current_slide_num_layers(gint new_num_layers);
void set_debug_level(guint new_debug_level);
void set_default_text_fg_colour(GdkColor new_default_text_fg_colour);
void set_default_text_font_face(gint new_default_text_font_face);
void set_default_text_font_size(gdouble new_default_text_font_size);
void set_display_help_text(gboolean new_display_help_text);
void set_end_behaviour(guint new_end_behaviour);
void set_end_point_status(guint new_end_point_status);
void set_fdb_font_object(guint index, SWFFont new_fdb_font_object);
void set_file_name(gchar *new_file_name);
void set_film_strip_being_resized(gboolean new_film_strip_being_resized);
void set_film_strip_column(GtkTreeViewColumn *new_film_strip_column);
void set_film_strip_container(GtkScrolledWindow *new_film_strip_container);
void set_film_strip_store(GtkListStore *new_film_strip_store);
void set_film_strip_view(GtkWidget *new_film_strip_view);
void set_frames_per_second(guint new_frames_per_second);
void set_front_store(GdkPixmap *new_front_store);
void set_ft_font_face(guint index, FT_Face new_ft_font_face);
void set_icon_extension(gchar *new_icon_extension);
void set_icon_path(gchar *new_icon_path);
void set_info_display(gboolean new_info_display);
void set_info_text(GtkTextBuffer *new_info_text);
void set_invalidation_end_x(gint new_invalidation_end_x);
void set_invalidation_end_y(gint new_invalidation_end_y);
void set_invalidation_start_x(gint new_invalidation_start_x);
void set_invalidation_start_y(gint new_invalidation_start_y);
void set_last_folder(gchar *new_last_folder);
void set_main_area(GtkWidget *new_main_area);
void set_main_drawing_area(GtkWidget *new_main_drawing_area);
void set_main_window(GtkWidget *new_main_window);
void set_menu_bar(GtkItemFactory *new_menu_bar);
void set_message_bar(GtkTable *new_message_bar);
void set_mouse_click_double_added(gboolean new_mouse_click_double_added);
void set_mouse_click_single_added(gboolean new_mouse_click_single_added);
void set_mouse_click_triple_added(gboolean new_mouse_click_triple_added);
void set_mouse_dragging(gboolean new_mouse_dragging);
void set_mouse_ptr_pixbuf(GdkPixbuf *new_mouse_ptr_pixbuf);
void set_mouse_ptr_string(gchar *new_mouse_ptr_string);
void set_new_layer_selected(gboolean new_new_layer_selected);
void set_project_active(gboolean new_project_active);
void set_resize_handle_size(guint new_resize_handle_size);
void set_resize_handles_rect_height(guint index, gint new_height);
void set_resize_handles_rect_width(guint index, gint new_width);
void set_resize_handles_rect_x(guint index, gint new_x);
void set_resize_handles_rect_y(guint index, gint new_y);
void set_resize_handles_status(guint new_resize_handles_status);
void set_right_side(GtkWidget *new_right_side);
void set_resolution_callback(gulong new_resolution_callback);
void set_resolution_selector(GtkComboBox *new_resolution_selector);
void set_screenshot_command_num(gint new_screenshot_command_num);
void set_screenshot_delay_time(guint new_screenshot_delay_time);
void set_screenshot_key_warning(gboolean new_screenshot_key_warning);
void set_screenshots_enabled(gboolean new_screenshots_enabled);
void set_show_control_bar(gboolean new_show_control_bar);
void set_slides(GList *new_slides);
void set_start_behaviour(guint new_start_behaviour);
void set_status_bar(GtkWidget *new_status_bar);
void set_status_icon(GtkStatusIcon *new_status_icon);
void set_stored_x(gint new_stored_x);
void set_stored_y(gint new_stored_y);
void set_table_x_padding(gint new_table_x_padding);
void set_table_y_padding(gint new_table_y_padding);
void set_text_tags_fg_colour_slist(GSList *new_text_tags_fg_colour_slist);
void set_text_tags_font(guint index, GtkTextTag *new_text_tags_font);
void set_text_tags_size_slist(GSList *new_text_tags_size_slist);
void set_text_tags_table(GtkTextTagTable *new_text_tags_table);
void set_time_line_container(GtkWidget *new_time_line_container);
void set_time_line_vbox(GtkWidget *new_time_line_vbox);
void set_working_height(guint new_working_height);
void set_working_width(guint new_working_width);
void set_zoom(guint new_zoom);
void set_zoom_selector(GtkComboBox *new_zoom_selector);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* GLOBAL_FUNCTIONS_H_ */
