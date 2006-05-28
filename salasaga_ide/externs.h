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
extern GList				*current_slide;
extern gfloat				export_time_counter;
extern GString				*file_name;
extern GtkWidget			*film_strip;
extern GtkScrolledWindow	*film_strip_container;
extern guint				frames_per_second;
extern GtkWidget			*main_drawing_area;
extern GtkWidget			*main_window;
extern GtkItemFactory		*menu_bar;
extern GtkTable				*message_bar;
extern gboolean				mouse_dragging;
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
