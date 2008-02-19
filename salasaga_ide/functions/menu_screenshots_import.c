/*
 * $Id$
 *
 * Flame Project: Function called when the user selects Screenshots -> Import from the top menu 
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


// Standard includes
#include <stdlib.h>
#include <unistd.h>

// GTK includes
#include <glib/gstdio.h>
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"
#include "create_tooltips.h"
#include "display_warning.h"
#include "draw_timeline.h"
#include "draw_workspace.h"
#include "enable_layer_toolbar_buttons.h"
#include "enable_main_toolbar_buttons.h"
#include "menu_enable.h"
#include "regenerate_timeline_duration_images.h"


void menu_screenshots_import(void)
{
	// Local variables
	GDir				*dir_ptr;					// Pointer to the directory entry structure
	const gchar			*dir_entry;					// Holds a file name
	GSList				*entries = NULL;			// Holds a list of screen shot file names
	GError				*error = NULL;				// Pointer to error return structure
	GdkPixbufFormat		*file_format;
	GtkTreeIter			film_strip_iter;
	gboolean			image_differences;			// Used to indicate that screenshots have differing sizes
	gint				image_height;
	gint				image_width;
	guint				largest_height = 0;			// Holds the height of the largest screenshot thus far
	guint				largest_width = 0;			// Holds the width of the largest screenshot thus far
	GtkTreePath			*new_path;					// Path used to select the new film strip thumbnail
	gint				num_screenshots = 0;		// Switch to track if other screenshots already exist
	gint				num_slides;					// Number of slides in the whole slide list
	guint				recent_message;				// Message identifier, for newest status bar message
	gint				return_code = 0;			// Receives return code
	gint				return_code_int;
	gint				slide_position;				// Which slide in the slide list do we have selected?
	gboolean			using_first_screenshot;		// Used to indicate there's no project active, so we're using the dimensions of the first screenshot

	GtkWidget			*tmp_dialog;
	GdkPixbuf			*tmp_gdk_pixbuf;			// Temporary GDK Pixbuf
	layer_image			*tmp_image_ob;				// Temporary image layer
	gint				tmp_int = 0;				// Temporary integer
	GtkTreeIter			*tmp_iter;					// Temporary GtkTreeIter
	layer				*tmp_layer;					// Temporary layer
	GdkRectangle		tmp_rect = {0, 0, status_bar->allocation.width, status_bar->allocation.height};  // Temporary rectangle covering the area of the status bar
	slide				*tmp_slide;					// Temporary slide
	GString				*tmp_string;				// Temporary string


	// Initialise various things
	tmp_string = g_string_new(NULL);
	image_differences = FALSE;
	using_first_screenshot = FALSE;

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
				// * The directory entry has the correct file extension too, so it's very likely one of our screenshots *
				g_string_printf(tmp_string, g_build_path(G_DIR_SEPARATOR_S, screenshots_folder->str, dir_entry, NULL));
				file_format = gdk_pixbuf_get_file_info(tmp_string->str, &image_width, &image_height);
				if (NULL != file_format)
				{
					// * The file format was recognised *

					// If a project is already active, then we have existing dimensions we need to conform to
					if ((TRUE == project_active) || (TRUE == using_first_screenshot))
					{
						// Make a note if the width and height of this screenshot differs from the existing project dimensions
						if (image_width != project_width)
							image_differences = TRUE;
						if (image_height != project_height)
							image_differences = TRUE;
					} else
					{
						// There isn't a project loaded yet, so we take the dimensions of the first screenshot as the new project dimensions
						project_width = image_width;
						project_height = image_height;

						using_first_screenshot = TRUE;
					}

					// We add this screenshot to the list
					entries = g_slist_append(entries, g_strdup_printf("%s", dir_entry));
					num_screenshots += 1;
				}
			}
		}
	}

	// If screenshots of differing dimensions were found, warn the user and let them cancel out of the import
	if (TRUE == image_differences)
	{
		if (TRUE == project_active)
		{
			g_string_printf(tmp_string, "Not all of the screenshots are of the same size, or some differ from the size of the project.  If you proceed, they will be scaled to the same size as the project.  Do you want to proceed?");
		} else
		{
			g_string_printf(tmp_string, "Not all of the screenshots are of the same size, or some differ from the size of the project.  If you proceed, they will all be scaled to the size of the first one.  Do you want to proceed?");
		}

		// Display the warning dialog
		tmp_dialog = gtk_message_dialog_new(GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO, tmp_string->str);
		return_code_int = gtk_dialog_run(GTK_DIALOG(tmp_dialog));

		// Was the NO button pressed?
		if (GTK_RESPONSE_NO == return_code_int)
		{
			// * The user does not want to proceed *

			// Destroy the dialog box and return
			gtk_widget_destroy(tmp_dialog);
			return;	
		}

		// The user wants to proceed
		gtk_widget_destroy(tmp_dialog);
	}

	// If no screenshots were found, alert the user and return to the calling function
	if (0 == num_screenshots)
	{
		// Display the warning message using our function
		g_string_printf(tmp_string, "Error ED05: No screenshots found in screenshot folder:\n\n  %s\n\nThey are case sensitive and must be named:\n\n  <ProjectName><Sequential Number>.png\n\ni.e.:\n\n\t%s0001.png\n\t%s0002.png", screenshots_folder->str, project_name->str, project_name->str);
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
	for (tmp_int = 1; tmp_int <= num_screenshots; tmp_int++)
	{
		// Remove the previous status bar message
		gtk_statusbar_remove(GTK_STATUSBAR(status_bar), statusbar_context, recent_message);

		// * The loaded image file becomes a background layer image for a new slide *

		// Allocate a new slide structure for use
		tmp_slide = g_new(slide, 1);
		tmp_slide->layers = NULL;
		tmp_slide->duration = slide_length;

		// Allocate a new layer structure for use in the slide
		tmp_layer = g_new(layer, 1);
		tmp_layer->x_offset_start = 0;
		tmp_layer->y_offset_start = 0;
		tmp_layer->x_offset_finish = 0;
		tmp_layer->y_offset_finish = 0;

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
		tmp_image_ob->image_path = g_string_new(NULL);  // Images don't have a path after they've been imported, as we delete them!
		if (FALSE == project_active)
		{
			// This is the first screenshot, so we make the project size the same dimensions as it
			tmp_image_ob->image_data = gdk_pixbuf_new_from_file(tmp_string->str, NULL);  // Load the image again, at full size.  It's the background layer
			project_width = gdk_pixbuf_get_width(tmp_image_ob->image_data);
			project_height = gdk_pixbuf_get_height(tmp_image_ob->image_data);

			// Set the global toggle that a project is now active
			project_active = TRUE;
		} else
		{
			// This is not the first screenshot, so it will be loaded and scaled to the size of the existing project dimensions
			tmp_image_ob->image_data = gdk_pixbuf_new_from_file_at_size(tmp_string->str, project_width, project_height, NULL);
		}
		tmp_image_ob->width = project_width;
		tmp_image_ob->height = project_height;
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
		tmp_layer->visible = TRUE;
		tmp_layer->background = TRUE;
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

	// Update the project with the new height and width
	project_height = largest_height;
	project_width = largest_width;

	// If not presently set, make the first imported slide the present slide
	if (NULL == current_slide)
	{
		current_slide = g_list_first(slides);
	}

	// Select the correct thumbnail in the film strip and scroll to display it
	slides = g_list_first(slides);
	num_slides = g_list_length(slides);
	slide_position = g_list_position(slides, current_slide);
	new_path = gtk_tree_path_new_from_indices(slide_position, -1);
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(film_strip_view), new_path, NULL, FALSE);
	gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(film_strip_view), new_path, NULL, TRUE, 0.5, 0.0);

	// Generate the timeline duration image(s)
	regenerate_timeline_duration_images(tmp_slide);

	// Create the slide tooltip(s)
	create_tooltips();

	// Get the presently selected zoom level
	g_string_printf(tmp_string, "%s", gtk_combo_box_get_active_text(GTK_COMBO_BOX(zoom_selector)));

	// Parse and store the zoom level
	tmp_int = g_ascii_strncasecmp(tmp_string->str, "F", 1);
	if (0 == tmp_int)
	{
		// "Fit to width" is selected, so work out the zoom level by figuring out how much space the widget really has
		//  (Look at the alloation of it's parent widget)
		//  Reduce the width calculated by 24 pixels (guessed) to give space for widget borders and such
		zoom = (guint) (((float) (right_side->allocation.width - 24) / (float) project_width) * 100);
	} else
	{
		tmp_string = g_string_truncate(tmp_string, tmp_string->len - 1);
		zoom = atoi(tmp_string->str);
	}

	// Calculate and set the display size of the working area
	working_width = (project_width * zoom) / 100;
	working_height = (project_height * zoom) / 100;

	// Free the existing backing store for the workspace
	if (NULL != backing_store)
	{
		g_object_unref(backing_store);
		backing_store = NULL;
	}

	// Draw the workspace area
	draw_workspace();

	// Draw the timeline area
	draw_timeline();

	// Enable the project based menu items
	menu_enable("/Project", TRUE);
	menu_enable("/Slide", TRUE);
	menu_enable("/Layer", TRUE);
	menu_enable("/Export", TRUE);

	// Enable the toolbar buttons
	enable_layer_toolbar_buttons();
	enable_main_toolbar_buttons();

	// Free the memory allocated in this function
	g_string_free(tmp_string, TRUE);
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.13  2008/02/19 17:44:29  vapour
 * Added code to select the first new screenshot when imported into an empty project, otherwise some other functions (i.e. slide delete) which rely on it don't work properly.
 *
 * Revision 1.12  2008/02/19 13:37:46  vapour
 * Improved the wording of the warning message about screenshot size differing.
 *
 * Revision 1.11  2008/02/12 14:18:33  vapour
 * Updated to use the new visibility and background fields in the layer structure.
 *
 * Revision 1.10  2008/02/12 05:26:25  vapour
 * Adjusted to work with the new, slightly simplified layer structure.
 *
 * Revision 1.9  2008/02/11 12:21:53  vapour
 * Updated to calculate the size of the initial working area better.
 *
 * Revision 1.8  2008/02/06 09:57:48  vapour
 * All screenshots are now loaded at the same size, giving the user a warning and chance to abort first if needed.
 *
 * Revision 1.7  2008/02/04 17:08:45  vapour
 *  + Removed unnecessary includes.
 *
 * Revision 1.6  2008/02/04 10:37:22  vapour
 * Updated to enable the Project top menu bar option after screenshots are imported.
 *
 * Revision 1.5  2008/01/21 11:25:34  vapour
 *  + Fixed a bug, in that slide duration wasn't being properly set.
 *  + A timeline duration image is now created for the new background layer.
 *
 * Revision 1.4  2008/01/15 16:19:03  vapour
 * Updated copyright notice to include 2008.
 *
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
