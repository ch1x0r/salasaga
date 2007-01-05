/*
 * $Id$
 *
 * Flame Project: Include file for global variables
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

#ifndef __EXTERNS_H__
#define __EXTERNS_H__

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Variables defined elsewhere
extern GdkPixmap			*backing_store;
extern GList				*boundary_list;
extern guint				capture_height;
extern guint				capture_width;
extern guint				capture_x;
extern guint				capture_y;
extern GList				*current_slide;
extern gfloat				export_time_counter;
extern GString				*icon_extension;
extern GString				*icon_path;
extern GString				*file_name;
extern GtkWidget			*film_strip;
extern GtkScrolledWindow	*film_strip_container;
extern guint				frames_per_second;
extern GtkWidget			*main_drawing_area;
extern GtkWidget			*main_window;
extern GtkItemFactory		*menu_bar;
extern GtkTable				*message_bar;
extern gboolean				mouse_dragging;
extern GdkPixbuf			*mouse_ptr_pixbuf;
extern GIOChannel			*output_file;
extern gulong				resolution_callback;
extern GtkComboBox			*resolution_selector;
extern GtkWidget			*right_side;
extern GList				*slides;
extern GtkWidget			*status_bar;
extern guint				statusbar_context;
extern gint					stored_x;
extern gint					stored_y;
extern GtkWidget			*time_line_container;
extern GtkWidget			*time_line_vbox;
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
extern GString				*default_output_folder;
extern guint				default_output_height;
extern guint				default_output_quality;
extern guint				default_output_width;
extern GString				*default_project_folder;
extern guint				default_slide_length;
extern guint				icon_height;
extern guint				preview_width;
extern guint				scaling_quality;
extern GString				*screenshots_folder;

extern GString				*output_folder;
extern guint				output_height;
extern guint				output_quality;
extern guint				output_width;
extern GString				*project_folder;
extern guint				project_height;
extern GString				*project_name;
extern guint				project_width;
extern guint				slide_length;

extern ResolutionStructure	res_array[];
extern gint					num_res_items;


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __EXTERNS_H__


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.12  2007/01/05 06:46:47  vapour
 * Hard coded to expect icons and similar in /usr/share/.
 *
 * Revision 1.11  2006/12/30 12:59:58  vapour
 * Updated with new path info for the shared files, which should make packaging easier.
 *
 * Revision 1.10  2006/07/09 08:05:44  vapour
 * Added global externs to support the svg image loading test and mouse pointer graphic.
 *
 * Revision 1.9  2006/07/02 09:25:18  vapour
 * Updated for the rewritten layer toolbar icons code.
 *
 * Revision 1.8  2006/06/27 13:36:47  vapour
 * Add externs for the re-written main toolbar functions.
 *
 * Revision 1.7  2006/05/31 13:59:24  vapour
 * Added the capture globals.
 *
 * Revision 1.6  2006/05/28 09:37:45  vapour
 * + Moved some structures from the main function to the global context.
 * + Re-tab aligned some variables for my Linux Eclipse.
 *
 * Revision 1.5  2006/05/17 11:21:27  vapour
 * Added the export_time_counter global variable.
 *
 * Revision 1.4  2006/05/15 13:41:53  vapour
 * Sorted externs alphabetically.
 *
 * Revision 1.3  2006/04/21 17:47:18  vapour
 * + Updated header with clearer copyright and license details.
 * + Moved the History section to the end of the file.
 *
 * Revision 1.2  2006/04/16 06:04:04  vapour
 * Removed header info copied from my local repository.
 *
 * Revision 1.1  2006/04/13 15:59:54  vapour
 * Initial version, copied from my local CVS repository.
 */
