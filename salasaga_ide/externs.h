/*
 * $Id$
 *
 * Flame Project: Include file for global variables
 * 
 * Copyright (C) 2007 Justin Clift <justin@postgresql.org>
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
extern guint				debug_level;
extern gfloat				export_time_counter;
extern GString				*icon_extension;
extern GString				*icon_path;
extern GString				*file_name;
extern gboolean				film_strip_being_resized;
extern GtkTreeViewColumn		*film_strip_column;
extern GtkScrolledWindow		*film_strip_container;
extern GtkListStore			*film_strip_store;
extern GtkWidget			*film_strip_view;
extern guint				frames_per_second;
extern GtkWidget			*main_drawing_area;
extern GtkWidget			*main_window;
extern GtkItemFactory			*menu_bar;
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
extern gint				stored_x;
extern gint				stored_y;
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

extern ResolutionStructure		res_array[];
extern gint				num_res_items;


#ifdef _WIN32
// Windows only variables
extern HHOOK					win32_keyboard_hook_handle;
#endif


// External includes (from backend.h)
#include "functions/base64_decode.h"
#include "functions/base64_encode.h"
#include "functions/calculate_object_boundaries.h"
#include "functions/destroy_slide.h"
#include "functions/destroy_slide.h"
#include "functions/detect_collisions.h"
#include "functions/flame_read.h"
#include "functions/logger_simple.h"
#include "functions/logger_with_domain.h"
#include "functions/menu_enable.h"
#include "functions/menu_export_flash_inner.h"
#include "functions/menu_export_svg_animation_slide.h"
#include "functions/menu_file_save_layer.h"
#include "functions/menu_file_save_slide.h"
#include "functions/regenerate_film_strip_thumbnails.h"
#include "functions/regenerate_timeline_duration_images.h"
#include "functions/save_preferences_and_exit.h"
#include "functions/sound_beep.h"
#include "functions/uri_decode.h"
#include "functions/uri_encode_base64.h"

// External includes (from callbacks.h)
#include "functions/event_size_allocate_received.h"
#include "functions/film_strip_handle_changed.h"
#include "functions/film_strip_handle_released.h"
#include "functions/film_strip_slide_clicked.h"
#include "functions/resolution_selector_changed.h"
#include "functions/timeline_edited_name.h"
#include "functions/timeline_edited_x_offset_finish.h"
#include "functions/timeline_edited_x_offset_start.h"
#include "functions/timeline_edited_y_offset_finish.h"
#include "functions/timeline_edited_y_offset_start.h"
#include "functions/working_area_button_press_event.h"
#include "functions/working_area_button_release_event.h"
#include "functions/working_area_expose_event.h"
#include "functions/working_area_motion_notify_event.h"
#include "functions/zoom_selector_changed.h"

// External includes (from flame-edit.c)
#include "functions/create_film_strip.h"
#include "functions/create_menu_bar.h"
#include "functions/create_time_line.h"
#include "functions/create_toolbar.h"
#include "functions/create_working_area.h"
#include "functions/quit_event.h"


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __EXTERNS_H__


/*
 * History
 * +++++++
 * 
 * $Log$
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
