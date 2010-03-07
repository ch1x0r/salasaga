/*
 * $Id$
 *
 * Salasaga: Function called when the user selects Screenshots -> Import from the top menu
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

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../cairo/create_cairo_pixbuf_pattern.h"
#include "../layer/compress_layers.h"
#include "../dialog/display_warning.h"
#include "../preference/project_preferences.h"
#include "../time_line/draw_timeline.h"
#include "../time_line/time_line_set_selected_layer_num.h"
#include "../tool_bars/enable_layer_toolbar_buttons.h"
#include "../tool_bars/enable_main_toolbar_buttons.h"
#include "../working_area/draw_workspace.h"
#include "menu_enable.h"


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
	GString				*message;					// Used to construct message strings
	GtkTreePath			*new_path;					// Path used to select the new film strip thumbnail
	gint				num_screenshots = 0;		// Switch to track if other screenshots already exist
	GtkTreePath			*old_path = NULL;			// The old path, which we'll free
	guint				preview_height;				// The height we calculate a film strip thumbnail should be
	gfloat				project_ratio;				// Ratio of project height to width
	gint				return_code = 0;			// Receives return code
	gint				return_code_int;
	gint				slide_position;				// Which slide in the slide list do we have selected?
	GList				*this_slide = NULL;			// Temporary Glist
	GtkWidget			*tmp_dialog;
	layer_empty			*tmp_empty_ob;				// Temporary empty layer
	layer_image			*tmp_image_ob;				// Temporary image layer
	gint				tmp_int = 0;				// Temporary integer
	layer				*tmp_layer;					// Temporary layer
	GdkPixbuf			*tmp_pixbuf;				// Used to convert from a pixmap to a pixbuf
	GdkPixmap			*tmp_pixmap;				// Used when converting from a pixmap to a pixbuf
	GdkRectangle		tmp_rect = {0, 0, get_status_bar()->allocation.width, get_status_bar()->allocation.height};  // Temporary rectangle covering the area of the status bar
	slide				*tmp_slide;					// Temporary slide
	GString				*tmp_string;				// Temporary string
	gboolean			using_first_screenshot;		// Used to indicate there's no project active, so we're using the dimensions of the first screenshot


	// Initialise various things
	image_differences = FALSE;
	message = g_string_new(NULL);
	tmp_string = g_string_new(NULL);
	using_first_screenshot = FALSE;

	// Set the default information button display info if there isn't a project already active
	if (TRUE != get_project_active())
	{
		info_link = g_string_new("http://www.salasaga.org");
		info_link_target = g_string_new(_("_blank"));
		info_text = gtk_text_buffer_new(text_tags_table);
		gtk_text_buffer_set_text(GTK_TEXT_BUFFER(info_text), _("Created using Salasaga"), -1);
		set_info_display(TRUE);
	}

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
			g_string_printf(message, "%s ED03: %s '%s': %s", _("Error"), _("Something went wrong opening the screenshots folder."), screenshots_folder->str, error->message);
			display_warning(message->str);

			// Free the memory allocated in this function
			g_string_free(message, TRUE);
			g_string_free(tmp_string, TRUE);
			g_error_free(error);
			exit(3);
		}

		// The screenshots folder doesn't exist.  Notify the user and direct them to change this in the preferences menu
		g_string_printf(message, "%s ED04: %s", _("Error"), _("The screenshots folder doesn't exist.  Please update this in the project preferences and try again."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		g_error_free(error);
		return;
	}

	// * Scan the given directory for screenshot files to import *
	while ((dir_entry = g_dir_read_name(dir_ptr)) != NULL)
	{
		// Look for files starting with the word "screenshot"
		if (g_str_has_prefix(dir_entry, _("screenshot")))
		{
			// The directory entry starts with the correct prefix, now let's check the file extension
			if (g_str_has_suffix(dir_entry, ".png"))
			{
				// * The directory entry has the correct file extension too, so it's very likely one of our screenshots *
				g_string_printf(tmp_string, "%s", g_build_path(G_DIR_SEPARATOR_S, screenshots_folder->str, dir_entry, NULL));
				file_format = gdk_pixbuf_get_file_info(tmp_string->str, &image_width, &image_height);
				if (NULL != file_format)
				{
					// * The file format was recognised *

					// If a project is already active, then we have existing dimensions we need to conform to
					if ((TRUE == get_project_active()) || (TRUE == using_first_screenshot))
					{
						// Make a note if the width and height of this screenshot differs from the existing project dimensions
						if (image_width != get_project_width())
							image_differences = TRUE;
						if (image_height != get_project_height())
							image_differences = TRUE;
					} else
					{
						// There isn't a project loaded yet, so we take the dimensions of the first screenshot as the new project dimensions
						set_project_width(image_width);
						set_project_height(image_height);

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
		if (TRUE == get_project_active())
		{
			g_string_printf(tmp_string, "%s", _("Not all of the screenshots are of the same size, or some differ from the size of the project.  If you proceed, all screenshots different to the project size will become image layers.  Do you want to proceed?"));
		} else
		{
			g_string_printf(tmp_string, "%s", _("Not all of the screenshots are of the same size.  If you proceed, the project size will be set to the size of the first screenshot.  All screenshots of a different size to that will become image layers.  Do you want to proceed?"));
		}

		// Display the warning dialog
		tmp_dialog = gtk_message_dialog_new(GTK_WINDOW(get_main_window()), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO, "%s", tmp_string->str);
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
		g_string_printf(message, "%s ED05: %s", _("Error"), _("No screenshots found.  You need to take some screenshots before using Import."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		return;
	}

	// Sort the screenshots in numeric order
	entries = g_slist_sort(entries, (GCompareFunc) g_ascii_strcasecmp);

	// Use the status bar to communicate the number of screenshots found
	g_string_printf(tmp_string, " %u %s", num_screenshots, _("Screenshots found"));
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), tmp_string->str);
	gdk_flush();

	// Redraw the main window
	gtk_widget_draw(get_status_bar(), &tmp_rect);

	// * Load the screenshots *
	for (tmp_int = 1; tmp_int <= num_screenshots; tmp_int++)
	{
		// Local loop initialisation
		image_differences = FALSE;

		// Work out the full path to the image file
		g_string_printf(tmp_string, "%s%c", screenshots_folder->str, G_DIR_SEPARATOR);
		tmp_string = g_string_append(tmp_string, g_slist_nth(entries, tmp_int - 1)->data);

		// Get the size of the image
		gdk_pixbuf_get_file_info(tmp_string->str, &image_width, &image_height);

		// Is there a project already loaded?
		if (FALSE == get_project_active())
		{
			// This is the first screenshot, so we make the project size the same dimensions as it
			set_project_width(image_width);
			set_project_height(image_height);

			// Set the global toggle that a project is now active
			set_project_active(TRUE);
		} else
		{
			// The project size is already known, so if the size of this screenshot is different we make it a separate image layer instead
			if ((image_width != get_project_width()) || (image_height != get_project_height()))
			{
				// The image size is different from the project size, so we make a note of this
				image_differences = TRUE;
			}
		}

		// Determine the proper thumbnail height
		project_ratio = (gfloat) get_project_height() / (gfloat) get_project_width();
		preview_height = preview_width * project_ratio;

		// Construct a new image object and load the image data
		tmp_image_ob = g_new(layer_image, 1);
		tmp_image_ob->image_data = gdk_pixbuf_new_from_file(tmp_string->str, NULL);
		tmp_image_ob->width = gdk_pixbuf_get_width(tmp_image_ob->image_data);
		tmp_image_ob->height = gdk_pixbuf_get_height(tmp_image_ob->image_data);
		tmp_image_ob->modified = FALSE;

		// Create a cairo pattern from the image data
		tmp_image_ob->cairo_pattern = create_cairo_pixbuf_pattern(tmp_image_ob->image_data);
		if (NULL == tmp_image_ob->cairo_pattern)
		{
			// Something went wrong when creating the image pattern
			g_string_printf(message, "%s ED376: %s", _("Error"), _("Couldn't create an image pattern."));
			display_warning(message->str);

			// Free the memory used in this function
			g_string_free(message, TRUE);
			g_string_free(tmp_string, TRUE);
			return;
		}

		// Allocate a new slide structure for use
		tmp_slide = g_new(slide, 1);
		tmp_slide->layers = NULL;
		tmp_slide->duration = default_slide_duration;
		tmp_slide->scaled_cached_pixmap = NULL;
		tmp_slide->cached_pixmap_valid = FALSE;
		tmp_slide->num_layers = 1;

		// Allocate a new layer structure for use in the slide
		tmp_layer = g_new(layer, 1);
		tmp_layer->name = g_string_new(_("Background"));
		tmp_layer->start_time = 0.0;
		tmp_layer->duration = default_slide_duration;  // Slide duration rather than layer duration, because these become slides
		tmp_layer->x_offset_start = 0;
		tmp_layer->y_offset_start = 0;
		tmp_layer->x_offset_finish = 0;
		tmp_layer->y_offset_finish = 0;
		tmp_layer->visible = TRUE;
		tmp_layer->background = TRUE;
		tmp_layer->transition_in_type = TRANS_LAYER_NONE;
		tmp_layer->transition_in_duration = 0.0;
		tmp_layer->transition_out_type = TRANS_LAYER_NONE;
		tmp_layer->transition_out_duration = 0.0;
		tmp_layer->external_link = g_string_new(NULL);
		tmp_layer->external_link_window = g_string_new(_("_self"));

		// Should the background layer be the image itself, or an empty layer?
		if (FALSE == image_differences)
		{
			// * The background layer is the image itself *

			// Set the details of the layer
			tmp_layer->object_type = TYPE_GDK_PIXBUF;
			tmp_layer->object_data = (GObject *) tmp_image_ob;

			// Create the film strip thumbnail and add it to the new slide structure
			tmp_slide->thumbnail = gdk_pixbuf_scale_simple(tmp_image_ob->image_data, preview_width, preview_height, GDK_INTERP_TILES);
			if (NULL == tmp_slide->thumbnail)
			{
				// Something went wrong when loading the screenshot
				g_string_printf(message, "%s ED06: %s '%s'", _("Error"), _("Something went wrong when loading the screenshot"), tmp_string->str);
				display_warning(message->str);

				// Free the memory used in this function
				g_string_free(message, TRUE);
				g_string_free(tmp_string, TRUE);
				return;
			}

			// Add the background layer to the new slide being created
			tmp_slide->layers = g_list_append(tmp_slide->layers, tmp_layer);
		}
		else
		{
			// * The image should be in it's own layer, with an empty background layer instead *

			// * Create the empty background layer *
			tmp_empty_ob = g_new(layer_empty, 1);
			tmp_layer->object_type = TYPE_EMPTY;
			tmp_layer->object_data = (GObject *) tmp_empty_ob;
			tmp_empty_ob->bg_color.pixel = default_bg_colour.pixel;
			tmp_empty_ob->bg_color.blue = default_bg_colour.blue;
			tmp_empty_ob->bg_color.green = default_bg_colour.green;
			tmp_empty_ob->bg_color.red = default_bg_colour.red;

			// Add the empty background layer to the new slide being created
			tmp_slide->layers = g_list_append(tmp_slide->layers, tmp_layer);

			// * Create the image layer *
			tmp_layer = g_new(layer, 1);
			tmp_layer->object_type = TYPE_GDK_PIXBUF;
			tmp_layer->object_data = (GObject *) tmp_image_ob;
			tmp_layer->name = g_string_new(_("Image"));
			tmp_layer->start_time = 0.0;
			tmp_layer->duration = default_slide_duration;
			tmp_layer->x_offset_start = 0;
			tmp_layer->y_offset_start = 0;
			tmp_layer->x_offset_finish = 0;
			tmp_layer->y_offset_finish = 0;
			tmp_layer->visible = TRUE;
			tmp_layer->background = FALSE;  // This is not a background layer
			tmp_layer->transition_in_type = TRANS_LAYER_NONE;
			tmp_layer->transition_in_duration = 0.0;
			tmp_layer->transition_out_type = TRANS_LAYER_NONE;
			tmp_layer->transition_out_duration = 0.0;
			tmp_layer->external_link = g_string_new(NULL);
			tmp_layer->external_link_window = g_string_new(_("_self"));

			// Add the image layer to the new slide being created
			tmp_slide->layers = g_list_prepend(tmp_slide->layers, tmp_layer);
			tmp_slide->num_layers = 2;

			// Create the thumbnail for the slide
			this_slide = g_list_append(this_slide, tmp_slide);
			tmp_pixmap = compress_layers(this_slide, 0.0, get_project_width(), get_project_height());
			tmp_pixbuf = gdk_pixbuf_get_from_drawable(NULL, GDK_PIXMAP(tmp_pixmap), NULL, 0, 0, 0, 0, -1, -1);
			tmp_slide->thumbnail = gdk_pixbuf_scale_simple(GDK_PIXBUF(tmp_pixbuf), preview_width, preview_height, GDK_INTERP_TILES);
			g_list_free(this_slide);
			this_slide = NULL;
			g_object_unref(GDK_PIXBUF(tmp_pixbuf));
			g_object_unref(GDK_PIXMAP(tmp_pixmap));
		}

		// Mark the name for the slide as unset
		tmp_slide->name = NULL;

		// Set the timeline widget for the slide to NULL, so we know to create it later on
		tmp_slide->timeline_widget = NULL;

		// Add the thumbnail to the GtkListView based film strip
		gtk_list_store_append(film_strip_store, &film_strip_iter);  // Acquire an iterator
		gtk_list_store_set(film_strip_store, &film_strip_iter, 0, tmp_slide->thumbnail, -1);

		// Add the temporary slide to the slides GList
		slides = g_list_append(slides, tmp_slide);

		// Delete the screenshot file just loaded
		return_code = g_remove(tmp_string->str);
		if (-1 == return_code)
		{
			// * Something went wrong when deleting the screenshot *

			// Display a warning message using our function
			g_string_printf(message, "%s ED85: %s '%s'", _("Error"), _("Something went wrong when deleting the screenshot file"), tmp_string->str);
			display_warning(message->str);
		}

		// Update the status bar with a progress counter
		g_string_printf(tmp_string, " %s %u of %u", _("Loaded image"), tmp_int, num_screenshots);
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), tmp_string->str);
		gtk_progress_bar_pulse(GTK_PROGRESS_BAR(get_status_bar()));
		gdk_flush();

		// Redraw the main window
		gtk_widget_draw(get_status_bar(), &tmp_rect);
	}

	// If not presently set, make the first imported slide the present slide
	if (NULL == current_slide)
	{
		current_slide = g_list_first(slides);
	}

	// Select the correct thumbnail in the film strip and scroll to display it
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(get_film_strip_view()), &new_path, NULL);
	if (NULL != new_path)
		old_path = new_path;  // Make a backup of the old path, so we can free it
	slides = g_list_first(slides);
	slide_position = g_list_position(slides, current_slide);
	new_path = gtk_tree_path_new_from_indices(slide_position, -1);
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(get_film_strip_view()), new_path, NULL, FALSE);
	gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(get_film_strip_view()), new_path, NULL, TRUE, 0.5, 0.0);
	if (NULL != old_path)
		gtk_tree_path_free(old_path);  // Free the old path

	// Regenerate the timeline
	draw_timeline();
	time_line_set_selected_layer_num(GTK_WIDGET(((slide *) current_slide->data)->timeline_widget), 0);

	// Get the presently selected zoom level
	g_string_printf(tmp_string, "%s", gtk_combo_box_get_active_text(GTK_COMBO_BOX(zoom_selector)));

	// Parse and store the zoom level
	if ((0 == g_strcmp0("Fit to width", tmp_string->str)) || (0 == g_strcmp0(_("Fit to width"), tmp_string->str)))
	{
		// "Fit to width" is selected, so work out the zoom level by figuring out how much space the widget really has
		//  (Look at the allocation of it's parent widget)
		//  Reduce the width calculated by 24 pixels (guessed) to give space for widget borders and such
		set_zoom((guint) (((float) (get_right_side()->allocation.width - 24) / (float) get_project_width()) * 100));
	} else
	{
		tmp_string = g_string_truncate(tmp_string, tmp_string->len - 1);
		set_zoom(atoi(tmp_string->str));
	}

	// Calculate and set the display size of the working area
	set_working_width((get_project_width() * get_zoom()) / 100);
	set_working_height((get_project_height() * get_zoom()) / 100);

	// Resize the drawing area so it draws properly
	gtk_widget_set_size_request(GTK_WIDGET(get_main_drawing_area()), get_working_width(), get_working_height());

	// Free the existing front store for the workspace
	if (NULL != front_store)
	{
		g_object_unref(GDK_PIXMAP(front_store));
		front_store = NULL;
	}

	// Redraw the workspace
	draw_workspace();

	// Enable the project based menu items
	menu_enable(_("/Project"), TRUE);
	menu_enable(_("/Slide"), TRUE);
	menu_enable(_("/Layer"), TRUE);
	menu_enable(_("/Export"), TRUE);

	// Enable the toolbar buttons
	enable_layer_toolbar_buttons();
	enable_main_toolbar_buttons();

	// Set the changes made variable
	set_changes_made(TRUE);

	// Update the status bar
	if (1 == num_screenshots)
	{
		g_string_printf(tmp_string, " %u %s", num_screenshots, _("screenshot imported"));
	} else
	{
		g_string_printf(tmp_string, " %u %s", num_screenshots, _("screenshots imported"));
	}
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), tmp_string->str);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(get_status_bar()), 0.0);
	gdk_flush();

	// Free the memory allocated in this function
	g_string_free(tmp_string, TRUE);
}
