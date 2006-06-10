/*
 * $Id$
 *
 * Flame Project: Header file for all non specific GUI related funtions
 * 
 * Copyright (C) 2006 Justin Clift <justin@postgresql.org>
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

#ifndef __GUIFUNCTIONS_H__
#define __GUIFUNCTIONS_H__

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Include required GTK widgets, in case this is used from something that hasn't already
#include <gtk/gtkwidget.h>
#include <gtk/gtkvbox.h>

// Menu functions
void menu_edit_preferences(void);
void menu_export_flash_animation(void);
void menu_export_svg_animation(void);
void menu_file_new(void);
void menu_file_open(void);
void menu_file_save(void);
void menu_help_about(void);
void menu_help_register(void);
void menu_help_survey(void);
void menu_help_website(void);
void menu_screenshots_capture(void);
void menu_screenshots_import(void);

// Other functions
GdkPixbuf *compress_layers(GList *which_slide, guint width, guint height);
void compress_layers_inner(gpointer element, gpointer user_data);
GtkWidget *construct_timeline_widget(slide *slide_data);
GtkWidget *create_resolution_selector(ResolutionStructure *res_array, guint num_resolutions, guint initial_width, guint initial_height);
void create_tooltips(void);
gboolean display_dialog_highlight(layer *tmp_layer, gchar *dialog_title);
gboolean display_dialog_image(layer *tmp_layer, gchar *dialog_title, gboolean request_file);
gboolean display_dialog_text(layer *tmp_layer, gchar *dialog_title);
gint display_warning(gchar *warning_string);
void draw_bounding_box(GtkWidget *widget, GdkRegion *region);
void draw_highlight_box(GdkPixbuf *tmp_pixbuf, gint x_offset, gint y_offset, gint width, gint height, guint32 fill_color, guint32 border_color);
void draw_thumbnail(GList *which_slide);
void draw_timeline(void);
void draw_workspace(void);
void image_crop(void);
void layer_delete(void);
void layer_edit(void);
void layer_move_down(void);
void layer_move_up(void);
void layer_new_highlight(void);
void layer_new_image(void);
void layer_new_text(void);
void project_crop(void);
void refresh_film_strip(void);
void slide_delete(void);
void slide_insert(void);
void slide_move_down(void);
void slide_move_up(void);
void slide_name_reset(void);
void slide_name_set(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __GUIFUNCTIONS_H__


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.7  2006/06/10 15:01:57  vapour
 * + Removed File -> Save as function.
 * + Added functions to set and reset slide names.
 *
 * Revision 1.6  2006/06/08 12:14:20  vapour
 * Created a new function that generates tooltips.
 *
 * Revision 1.5  2006/06/06 12:22:47  vapour
 * Improved the alphabetical order of functions.
 *
 * Revision 1.4  2006/05/23 13:12:55  vapour
 * Added an initial function to create the selection of available output resolutions dynamically from a list.
 *
 * Revision 1.3  2006/04/21 17:49:27  vapour
 * + Updated header with clearer copyright and license details.
 * + Moved the History section to the end of the file.
 *
 * Revision 1.2  2006/04/16 05:55:12  vapour
 * Removed header info copied from my local repository.
 *
 * Revision 1.1  2006/04/13 15:59:54  vapour
 * Initial version, copied from my local CVS repository.
 */
