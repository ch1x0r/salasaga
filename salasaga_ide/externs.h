/*
 * $Id$
 *
 * Salasaga: Include file for global variables
 * 
 * Copyright (C) 2007-2008 Justin Clift <justin@salasaga.org>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 */

#ifndef __EXTERNS_H__
#define __EXTERNS_H__

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Variables defined elsewhere
extern GdkPixbuf			*backing_store;
extern GList				*boundary_list;
extern guint				capture_height;
extern guint				capture_width;
extern guint				capture_x;
extern guint				capture_y;
extern layer				*copy_layer;
extern GList				*current_slide;
extern guint				debug_level;
extern guint				end_behaviour;
extern gfloat				export_time_counter;
extern GString				*file_name;
extern gboolean				film_strip_being_resized;
extern GtkTreeViewColumn	*film_strip_column;
extern GtkScrolledWindow	*film_strip_container;
extern GtkListStore			*film_strip_store;
extern GtkWidget			*film_strip_view;
extern gchar				*font_path;
extern guint				frames_per_second;
extern GdkPixmap			*front_store;
extern GString				*icon_extension;
extern GString				*icon_path;
extern gint					invalidation_end_x;
extern gint					invalidation_end_y;
extern gint					invalidation_start_x;
extern gint					invalidation_start_y;
extern GString				*last_folder;
extern GtkWidget			*main_area;
extern GtkWidget			*main_drawing_area;
extern GtkWidget			*main_window;
extern GtkItemFactory		*menu_bar;
extern GtkTable				*message_bar;
extern gboolean				mouse_dragging;
extern GdkPixbuf			*mouse_ptr_pixbuf;
extern gboolean				new_layer_selected;
extern GIOChannel			*output_file;
extern gboolean				project_active;
extern gulong				resolution_callback;
extern GtkComboBox			*resolution_selector;
extern GdkRectangle			resize_handles_rect[8];
extern guint				resize_handles_status;
extern guint				resize_handle_size;
extern GtkWidget			*right_side;
extern gboolean				screenshots_enabled;
extern gint					screenshot_command_num;
extern gboolean				show_control_bar;
extern GList				*slides;
extern guint				start_behaviour;
extern GtkWidget			*status_bar;
extern guint				statusbar_context;
extern gint					stored_x;
extern gint					stored_y;
extern gint					table_x_padding;
extern gint					table_y_padding;
extern GtkWidget			*time_line_container;
extern GtkWidget			*time_line_vbox;
extern guint				unscaled_button_height;
extern guint				unscaled_button_spacing;
extern guint				unscaled_button_width;
extern GtkWidget			*working;
extern guint				working_height;
extern guint				working_width;
extern guint				zoom;
extern GtkComboBox			*zoom_selector;

extern GtkTooltips			*main_toolbar_tooltips;
extern GtkWidget			*main_toolbar_icons[MAIN_TB_COUNT];
extern GtkWidget			*main_toolbar_icons_gray[MAIN_TB_COUNT];
extern GtkToolItem			*main_toolbar_items[MAIN_TB_COUNT];
extern gulong				main_toolbar_signals[MAIN_TB_COUNT];

// Layer toolbar items
extern GtkTooltips			*layer_toolbar_tooltips;
extern GtkWidget			*layer_toolbar_icons[MAIN_TB_COUNT];
extern GtkWidget			*layer_toolbar_icons_gray[MAIN_TB_COUNT];
extern GtkToolItem			*layer_toolbar_items[MAIN_TB_COUNT];
extern gulong				layer_toolbar_signals[MAIN_TB_COUNT];

extern GdkColor				default_bg_colour;
extern guint				default_fps;
extern GString				*default_output_folder;
extern guint				default_output_height;
extern guint				default_output_width;
extern GString				*default_project_folder;
extern guint				default_slide_length;
extern GString				*default_zoom_level;
extern guint				icon_height;
extern guint				preview_width;
extern GString				*screenshots_folder;

extern GString				*output_folder;
extern guint				output_height;
extern guint				output_width;
extern GString				*project_folder;
extern guint				project_height;
extern GString				*project_name;
extern guint				project_width;
extern guint				slide_length;

// Field descriptions used for bounds and validation
extern validatation_entry	valid_fields[];

#ifdef _WIN32
// Windows only variables
extern HHOOK				win32_keyboard_hook_handle;
#endif


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __EXTERNS_H__
