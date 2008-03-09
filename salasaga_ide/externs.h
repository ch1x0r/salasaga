/*
 * $Id$
 *
 * Flame Project: Include file for global variables
 * 
 * Copyright (C) 2007-2008 Justin Clift <justin@postgresql.org>
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
extern GtkWidget			*right_side;
extern gboolean				screenshots_enabled;
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


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.42  2008/03/09 14:49:32  vapour
 * Added global variable to hold the project preference for start behaviour.
 *
 * Revision 1.41  2008/03/06 00:10:02  vapour
 * Added a global variable to hold the desired end behaviour.
 *
 * Revision 1.40  2008/03/05 13:22:51  vapour
 * Expanded out the double buffering invalidation area global variables.
 *
 * Revision 1.39  2008/03/05 12:35:12  vapour
 * Added global variables to support double buffering.
 *
 * Revision 1.38  2008/03/05 09:10:35  vapour
 * Added a new global variable, to assist with the visual creation of new layers.
 *
 * Revision 1.37  2008/02/19 13:41:09  vapour
 * Removed scaling quality variable, added a default frames per second variable.
 *
 * Revision 1.36  2008/02/14 16:56:53  vapour
 * Added the valid fields array.
 *
 * Revision 1.35  2008/02/11 02:17:56  vapour
 * Added global variable to indicate whether screenshots should be enabled or not.
 *
 * Revision 1.34  2008/02/06 09:56:59  vapour
 *  + Added global project active variable.
 *  + Moved the ResolutionStructure into the create resolution selector function.
 *
 * Revision 1.33  2008/02/05 10:42:59  vapour
 * Added a global variable to support the default zoom level.
 *
 * Revision 1.32  2008/02/05 09:19:38  vapour
 *  + Removed support of output quality variable, as the concept is no longer relevant.
 *  + Made the main area variable a global, so we can resize the film strip width as needed.
 *
 * Revision 1.31  2008/02/04 14:25:20  vapour
 * Added global variables for table spacing.
 *
 * Revision 1.30  2008/01/31 01:05:50  vapour
 * Converted the swf control bar defines into variables, so they can be adjusted by the user in future.
 *
 * Revision 1.29  2008/01/19 07:11:10  vapour
 * Added font_path variable.
 *
 * Revision 1.28  2008/01/15 16:20:33  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.27  2007/10/06 11:36:39  vapour
 * Continued adjusting function include definitions.
 *
 * Revision 1.26  2007/10/04 11:50:30  vapour
 * Added initial (non working) stub functions for the creation of swf byte code for hightlight, mouse cursor, and text layers.
 *
 * Revision 1.25  2007/10/01 12:16:10  vapour
 * Added extern for the declaration of flash_create_tag_bitmap().
 *
 * Revision 1.24  2007/09/29 04:22:18  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 * Revision 1.23  2007/09/29 02:42:28  vapour
 * Broke flame-edit.c into its component functions.
 *
 * Revision 1.22  2007/09/28 12:05:08  vapour
 * Broke callbacks.c and callbacks.h into its component functions.
 *
 * Revision 1.21  2007/09/27 10:41:05  vapour
 * Broke backend.c and backend.h into its component functions.
 *
 * Revision 1.20  2007/09/19 13:31:49  vapour
 * Adding initial working code to set a keyboard hook for the Control Printscreen key through the flame-keycapture dll.
 *
 * Revision 1.19  2007/09/18 02:53:42  vapour
 * Updated copyright year to 2007.
 *
 * Revision 1.18  2007/07/09 12:42:53  vapour
 * Updated the film strip column resizing to work nicer.
 *
 * Revision 1.17  2007/07/09 12:25:18  vapour
 * Added a supporting global variable for the new film strip resizing functions.
 *
 * Revision 1.16  2007/07/03 14:28:30  vapour
 * Made the new debug_level global variable visible to the other files.
 *
 * Revision 1.15  2007/07/01 12:47:10  vapour
 * Removed the film_strip global variable, and removed an uneeded VBox out of the film strip structure.
 *
 * Revision 1.14  2007/06/30 06:04:19  vapour
 * The timeline and workspace area are now updated when a slide is selected in the film strip.  All done with the GtkTreeView approach now.
 *
 * Revision 1.13  2007/06/30 03:18:18  vapour
 * Began re-writing the film strip area to use a GtkListView widget instead of the hodge podge of event boxes, signal handlers, and other bits.
 *
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
