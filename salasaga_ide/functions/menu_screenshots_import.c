/*
 * $Id$
 *
 * Flame Project: Function called when the user selects Screenshots -> Import from the top menu 
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

// Standard includes
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

// GTK includes
#include <glib/gstdio.h>
#include <gtk/gtk.h>

// Gnome includes
#include <libgnome/gnome-url.h>

// XML includes
#include <libxml/xmlsave.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
	#include "flame-keycapture.h"
#endif

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"
#include "create_tooltips.h"
#include "display_warning.h"
#include "draw_workspace.h"
#include "draw_timeline.h"
#include "enable_layer_toolbar_buttons.h"
#include "enable_main_toolbar_buttons.h"
#include "menu_enable.h"


void menu_screenshots_import(void)
{
	// Local variables
	GDir				*dir_ptr;				// Pointer to the directory entry structure
	const gchar			*dir_entry;				// Holds a file name

	GSList				*entries = NULL;		// Holds a list of screen shot file names
	GtkTreeIter			film_strip_iter;
	guint				largest_height = 0;		// Holds the height of the largest screenshot thus far
	guint				largest_width = 0;		// Holds the width of the largest screenshot thus far

	gint				num_screenshots = 0;	// Switch to track if other screenshots already exist
	gint				return_code = 0;
	GError				*error = NULL;			// Pointer to error return structure

	guint				recent_message;			// Message identifier, for newest status bar message

	slide				*tmp_slide;				// Temporary slide
	layer				*tmp_layer;				// Temporary layer
	layer_image			*tmp_image_ob;			// Temporary image layer
	GtkTreeIter			*tmp_iter;				// Temporary GtkTreeIter
	GdkPixbuf			*tmp_gdk_pixbuf;		// Temporary GDK Pixbuf
	GString				*tmp_string;			// Temporary string
	gint				tmp_int = 0;			// Temporary integer
	GdkRectangle		tmp_rect = {0,			// Temporary rectangle covering the area of the status bar
						    0,
						    status_bar->allocation.width,
						    status_bar->allocation.height};


	// Initialise various things
	tmp_string = g_string_new(NULL);

	// * We know the path to get the screenshots from (screenshots_folder), and their prefix name (project_name),
	//   so we make a list of them and add them to the slides linked list *

	// * Check if the screenshots folder exists *
	if (!(dir_ptr = g_dir_open(screenshots_folder->str, 0, &error)))
	{
		// * Something went wrong when opening the screenshots folder *
		if (G_FILE_ERROR_NOENT != error->code)
		{
			// * The error was something other than the folder not existing, which we could cope with *

			// Display the warning message using our function
			g_string_printf(tmp_string, "Error ED03: Something went wrong opening the screenshots folder '%s': %s", screenshots_folder->str, error->message);
			display_warning(tmp_string->str);

			// Free the memory allocated in this function
			g_string_free(tmp_string, TRUE);
			g_error_free(error);
			exit(3);
		}

		// The screenshots folder doesn't exist.  Notify the user and direct them to change this in the preferences menu
		display_warning("Error ED04: The screenshots folder doesn't exist.  Please update this in the project preferences and try again");
		g_error_free(error);
		return;
	}

	// * Scan the given directory for screenshot files to import *
	while ((dir_entry = g_dir_read_name(dir_ptr)) != NULL)
	{
		// Look for files starting with the same name as the project
		if (g_str_has_prefix(dir_entry, project_name->str))
		{
			// The directory entry starts with the correct prefix, now let's check the file extension
			if (g_str_has_suffix(dir_entry, ".png"))
			{
				// The directory entry has the correct file extension too, so it's very likely one of our screenshots
				// We add it to the list of screenshot entries we know about
				entries = g_slist_append(entries, g_strdup_printf("%s", dir_entry));
				num_screenshots += 1;
			}
		}
	}

	// If no screenshots were found, alert the user and return to the calling function
	if (0 == num_screenshots)
	{
		// Display the warning message using our function
		g_string_printf(tmp_string, "Error ED05: No screenshots found in screenshot folder:\n\n  %s\n\nThey must be named:\n\n  <ProjectName><Sequential Number>.<File Extension>\n\ni.e.:\n\n\t%s0001.png\n\t%s0002.png\n\t<etc>\n\nThey are case sensitive as well.", screenshots_folder->str, project_name->str, project_name->str);
		display_warning(tmp_string->str);

		return;
	}

	// Use the status bar to communicate the number of screenshots found
	g_string_printf(tmp_string, "Found %u screenshots.", num_screenshots);
	recent_message = gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, tmp_string->str);
	gdk_flush();

	// Redraw the main window
	gtk_widget_draw(status_bar, &tmp_rect);

	// * Load the screenshots *
	// fixme5: Would it be better to just do the first few, and background the rest for later?
	for (tmp_int = 1; tmp_int <= num_screenshots; tmp_int++)
	{
		// Remove the previous status bar message
		gtk_statusbar_remove(GTK_STATUSBAR(status_bar), statusbar_context, recent_message);

		// * The loaded image file becomes a background layer image for a new slide *

		// Allocate a new slide structure for use
		tmp_slide = g_new(slide, 1);
		tmp_slide->layers = NULL;

		// Allocate a new layer structure for use in the slide
		tmp_layer = g_new(layer, 1);

		// Work out the full path to the image file
		g_string_printf(tmp_string, "%s%c", screenshots_folder->str, G_DIR_SEPARATOR);
		tmp_string = g_string_append(tmp_string, g_slist_nth(entries, tmp_int - 1)->data);

		// Load the image file(s) into a thumbnail sized pixel buffer
		tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_string->str, preview_width, -1, &error);
		if (NULL == tmp_gdk_pixbuf)
		{
			// * Something went wrong when loading the screenshot *

			// Display a warning message using our function
			g_string_printf(tmp_string, "Error ED06: Something went wrong when loading the screenshot '%s'", tmp_string->str);
			display_warning(tmp_string->str);
			g_string_free(tmp_string, TRUE);

			return;
		}

		// * Create the new background layer *

		// Construct a new image object
		tmp_image_ob = g_new(layer_image, 1);
		tmp_image_ob->x_offset_start = 0;
		tmp_image_ob->y_offset_start = 0;
		tmp_image_ob->x_offset_finish = 0;
		tmp_image_ob->y_offset_finish = 0;
		tmp_image_ob->image_path = g_string_new(tmp_string->str);
		tmp_image_ob->image_data = gdk_pixbuf_new_from_file(tmp_string->str, NULL);  // Load the image again, at full size.  It's the background layer
		tmp_image_ob->width = gdk_pixbuf_get_width(tmp_image_ob->image_data);
		tmp_image_ob->height = gdk_pixbuf_get_height(tmp_image_ob->image_data);
		tmp_image_ob->modified = FALSE;

		// If the new image is larger than the others loaded, we keep the new dimensions
		if (tmp_image_ob->height > largest_height)
		{
			largest_height = tmp_image_ob->height;
		}
		if (tmp_image_ob->width > largest_width)
		{
			largest_width = tmp_image_ob->width;
		}

		// Wrap the background layer info around it
		tmp_layer->object_data = (GObject *) tmp_image_ob;
		tmp_layer->object_type = TYPE_GDK_PIXBUF;
		tmp_layer->start_frame = 0;
		tmp_layer->finish_frame = slide_length;
		tmp_layer->name = g_string_new("Background");
		tmp_layer->external_link = g_string_new(NULL);

		// Add the background layer to the new slide being created
		tmp_slide->layers = g_list_append(tmp_slide->layers, tmp_layer);

		// Create the List store the slide layer data is kept in
		tmp_iter = g_new(GtkTreeIter, 1);
		tmp_layer->row_iter = tmp_iter;
		tmp_slide->layer_store = gtk_list_store_new(TIMELINE_N_COLUMNS,  // TIMELINE_N_COLUMNS
									G_TYPE_STRING,  // TIMELINE_NAME
									G_TYPE_BOOLEAN,  // TIMELINE_VISIBILITY
									GDK_TYPE_PIXBUF,  // TIMELINE_DURATION
									G_TYPE_UINT,  // TIMELINE_X_OFF_START
									G_TYPE_UINT,  // TIMELINE_Y_OFF_START
									G_TYPE_UINT,  // TIMELINE_X_OFF_FINISH
									G_TYPE_UINT);  // TIMELINE_Y_OFF_FINISH
		gtk_list_store_append(tmp_slide->layer_store, tmp_iter);
		gtk_list_store_set(tmp_slide->layer_store, tmp_iter,
						TIMELINE_NAME, tmp_layer->name->str,
						TIMELINE_VISIBILITY, TRUE,
						TIMELINE_DURATION, NULL,
						TIMELINE_X_OFF_START, 0,
						TIMELINE_Y_OFF_START, 0,
						TIMELINE_X_OFF_FINISH, 0,
						TIMELINE_Y_OFF_FINISH, 0,
						-1);

		// Add the thumbnail to the new slide structure
		tmp_slide->thumbnail = GTK_IMAGE(gtk_image_new_from_pixbuf(tmp_gdk_pixbuf));

		// Mark the name for the slide as unset
		tmp_slide->name = NULL;
		tmp_slide->tooltip = NULL;

		// Set the timeline widget for the slide to NULL, so we know to create it later on
		tmp_slide->timeline_widget = NULL;

		// Add the thumbnail to the GtkListView based film strip
		gtk_list_store_append (film_strip_store, &film_strip_iter);  // Acquire an iterator
		gtk_list_store_set (film_strip_store, &film_strip_iter, 0, gtk_image_get_pixbuf(tmp_slide->thumbnail), -1);

		// Add the temporary slide to the slides GList
		slides = g_list_append(slides, tmp_slide);

		// Delete the screenshot file just loaded
		return_code = g_remove(tmp_string->str);
		if (-1 == return_code)
		{
			// * Something went wrong when deleting the screenshot *

			// Display a warning message using our function
			g_string_printf(tmp_string, "Error ED85: Something went wrong when deleting the screenshot file '%s'", tmp_string->str);
			display_warning(tmp_string->str);
		}

		// Update the status bar with a progress counter
		g_string_printf(tmp_string, "Loaded image %u of %u.", tmp_int, num_screenshots);
		recent_message = gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, tmp_string->str);
		gdk_flush();

		// Redraw the main window
		gtk_widget_draw(status_bar, &tmp_rect);
	}

	// Free the temporary GString
	g_string_free(tmp_string, TRUE);

	// Update the project with the new height and width
	project_height = largest_height;
	project_width = largest_width;

	// If not presently set, make the first slide the present slide
	if (NULL == current_slide)
	{
		current_slide = g_list_first(slides);
	}

	// Recreate the slide tooltips
	create_tooltips();

	// Draw the workspace area
	draw_workspace();

	// Draw the timeline area
	draw_timeline();

	// Enable the project based menu items
	menu_enable("/Slide", TRUE);
	menu_enable("/Layer", TRUE);
	menu_enable("/Export", TRUE);

	// Enable the toolbar buttons
	enable_layer_toolbar_buttons();
	enable_main_toolbar_buttons();
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.3  2007/10/07 14:23:46  vapour
 * Aligned the whitespace comment padding for eclipse.
 *
 * Revision 1.2  2007/10/06 11:38:28  vapour
 * Continued adjusting function include definitions.
 *
 * Revision 1.1  2007/09/29 04:22:16  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */
