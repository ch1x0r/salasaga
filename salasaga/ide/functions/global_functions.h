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
guint get_debug_level();
GdkColor get_default_text_fg_colour();
gint get_default_text_font_face();
gdouble get_default_text_font_size();
gboolean get_display_help_text();
guint get_end_behaviour();
guint get_end_point_status();
gboolean get_film_strip_being_resized();
GtkWidget *get_film_strip_view();
guint get_frames_per_second();
gboolean get_info_display();
gint get_invalidation_end_x();
gint get_invalidation_end_y();
gint get_invalidation_start_x();
gint get_invalidation_start_y();
GtkWidget *get_main_area();
GtkWidget *get_main_drawing_area();
GtkWidget *get_main_window();
gboolean get_mouse_click_double_added();
gboolean get_mouse_click_single_added();
gboolean get_mouse_click_triple_added();
gboolean get_mouse_dragging();
gboolean get_new_layer_selected();
gboolean get_project_active();
guint get_resize_handles_status();
guint get_resize_handle_size();
gulong get_resolution_callback();
GtkWidget *get_right_side();
gint get_screenshot_command_num();
guint get_screenshot_delay_time();
gboolean get_screenshot_key_warning();
gboolean get_screenshots_enabled();
gboolean get_show_control_bar();
guint get_start_behaviour();
GtkWidget *get_status_bar();
gint get_stored_x();
gint get_stored_y();
gint get_table_x_padding();
gint get_table_y_padding();
GtkWidget *get_time_line_container();
GtkWidget *get_time_line_vbox();
guint get_working_height();
guint get_working_width();
guint get_zoom();
void set_boundary_list(GList *new_boundary_list);
void set_cairo_font_face(guint index, cairo_font_face_t *new_cairo_font_face);
void set_capture_height(guint new_capture_height);
void set_capture_width(guint new_capture_width);
void set_capture_x(guint new_capture_x);
void set_capture_y(guint new_capture_y);
void set_changes_made(gboolean new_changes_made);
void set_copy_layer(layer *new_copy_layer);
void set_debug_level(guint new_debug_level);
void set_default_text_fg_colour(GdkColor new_default_text_fg_colour);
void set_default_text_font_face(gint new_default_text_font_face);
void set_default_text_font_size(gdouble new_default_text_font_size);
void set_display_help_text(gboolean new_display_help_text);
void set_end_behaviour(guint new_end_behaviour);
void set_end_point_status(guint new_end_point_status);
void set_film_strip_being_resized(gboolean new_film_strip_being_resized);
void set_film_strip_view(GtkWidget *new_film_strip_view);
void set_frames_per_second(guint new_frames_per_second);
void set_info_display(gboolean new_info_display);
void set_invalidation_end_x(gint new_invalidation_end_x);
void set_invalidation_end_y(gint new_invalidation_end_y);
void set_invalidation_start_x(gint new_invalidation_start_x);
void set_invalidation_start_y(gint new_invalidation_start_y);
void set_main_area(GtkWidget *new_main_area);
void set_main_drawing_area(GtkWidget *new_main_drawing_area);
void set_main_window(GtkWidget *new_main_window);
void set_mouse_click_double_added(gboolean new_mouse_click_double_added);
void set_mouse_click_single_added(gboolean new_mouse_click_single_added);
void set_mouse_click_triple_added(gboolean new_mouse_click_triple_added);
void set_mouse_dragging(gboolean new_mouse_dragging);
void set_new_layer_selected(gboolean new_new_layer_selected);
void set_project_active(gboolean new_project_active);
void set_resize_handles_status(guint new_resize_handles_status);
void set_resize_handle_size(guint new_resize_handle_size);
void set_right_side(GtkWidget *new_right_side);
void set_resolution_callback(gulong new_resolution_callback);
void set_screenshot_command_num(gint new_screenshot_command_num);
void set_screenshot_delay_time(guint new_screenshot_delay_time);
void set_screenshot_key_warning(gboolean new_screenshot_key_warning);
void set_screenshots_enabled(gboolean new_screenshots_enabled);
void set_show_control_bar(gboolean new_show_control_bar);
void set_start_behaviour(guint new_start_behaviour);
void set_status_bar(GtkWidget *new_status_bar);
void set_stored_x(gint new_stored_x);
void set_stored_y(gint new_stored_y);
void set_table_x_padding(gint new_table_x_padding);
void set_table_y_padding(gint new_table_y_padding);
void set_time_line_container(GtkWidget *new_time_line_container);
void set_time_line_vbox(GtkWidget *new_time_line_vbox);
void set_working_height(guint new_working_height);
void set_working_width(guint new_working_width);
void set_zoom(guint new_zoom);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* GLOBAL_FUNCTIONS_H_ */
