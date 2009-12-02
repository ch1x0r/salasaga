/*
 * $Id$
 *
 * Salasaga: Include file for global variables
 *
 * Copyright (C) 2005-2009 Justin Clift <justin@salasaga.org>
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

#ifndef __EXTERNS_H__
#define __EXTERNS_H__

// FreeType includes
#include <ft2build.h>
#include FT_FREETYPE_H

// Ming include
#include <ming.h>

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Variables defined elsewhere
extern GList				*boundary_list;
extern cairo_font_face_t	*cairo_font_face[FONT_COUNT];
extern guint				capture_height;
extern guint				capture_width;
extern guint				capture_x;
extern guint				capture_y;
extern gboolean				changes_made;
extern layer				*copy_layer;
extern GList				*current_slide;
extern guint				debug_level;
extern gboolean				display_help_text;
extern guint				end_behaviour;
extern guint				end_point_status;
extern gfloat				export_time_counter;
extern SWFFont				fdb_font_object[FONT_COUNT];
extern GString				*file_name;
extern gboolean				film_strip_being_resized;
extern GtkTreeViewColumn	*film_strip_column;
extern GtkScrolledWindow	*film_strip_container;
extern GtkListStore			*film_strip_store;
extern GtkWidget			*film_strip_view;
extern guint				frames_per_second;
extern GdkPixmap			*front_store;
extern FT_Face				ft_font_face[FONT_COUNT];
extern GString				*icon_extension;
extern GString				*icon_path;
extern gboolean				info_display;
extern GString				*info_link;
extern GString				*info_link_target;
extern GtkTextBuffer		*info_text;
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
extern gboolean				mouse_click_double_added;
extern gboolean				mouse_click_single_added;
extern gboolean				mouse_click_triple_added;
extern gboolean				mouse_dragging;
extern GdkPixbuf			*mouse_ptr_pixbuf;
extern GString				*mouse_ptr_string;
extern gboolean				new_layer_selected;
extern GIOChannel			*output_file;
extern gboolean				project_active;
extern gulong				resolution_callback;
extern GtkComboBox			*resolution_selector;
extern GdkRectangle			resize_handles_rect[8];
extern guint				resize_handles_status;
extern guint				resize_handle_size;
extern GtkWidget			*right_side;
extern gint					screenshot_command_num;
extern guint				screenshot_delay_time;
extern gboolean				screenshot_key_warning;
extern gboolean				screenshots_enabled;
extern gboolean				show_control_bar;
extern GList				*slides;
extern guint				start_behaviour;
extern GtkWidget			*status_bar;
extern GtkStatusIcon		*status_icon;
extern gint					stored_x;
extern gint					stored_y;
extern gint					table_x_padding;
extern gint					table_y_padding;
extern GSList				*text_tags_fg_colour_slist;
extern GtkTextTag			*text_tags_fonts[FONT_COUNT];
extern GSList				*text_tags_size_slist;
extern GtkTextTagTable		*text_tags_table;
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
extern gfloat				default_slide_duration;
extern GString				*default_zoom_level;
extern guint				icon_height;
extern gfloat				default_layer_duration;
extern guint				preview_width;
extern GString				*screenshots_folder;

extern GString				*output_folder;
extern guint				output_height;
extern guint				output_width;
extern GString				*project_folder;
extern guint				project_height;
extern GString				*project_name;
extern guint				project_width;

// Field descriptions used for bounds and validation
extern validatation_entry	valid_fields[];

extern gchar				*salasaga_font_names[FONT_COUNT];

#ifdef _WIN32
// Windows only variables
extern HHOOK				win32_keyboard_hook_handle;
#endif


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __EXTERNS_H__
