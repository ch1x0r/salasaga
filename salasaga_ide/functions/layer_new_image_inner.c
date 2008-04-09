/*
 * $Id$
 *
 * Salasaga: Displays a dialog box asking for the values required to make a new image layer
 * 
 * Copyright (C) 2008 Justin Clift <justin@salasaga.org>
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


// GTK includes
#include <gtk/gtk.h>

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"
#include "display_warning.h"
#include "draw_timeline.h"
#include "draw_workspace.h"
#include "film_strip_create_thumbnail.h"
#include "validate_value.h"


void layer_new_image_inner(guint release_x, guint release_y)
{
	// Local variables
	GList				*layer_pointer;				// Points to the layers in the selected slide
	GdkPixbuf			*new_image_data;			// Receives the new image data
	gint				new_image_height;			// Receives the height of the new image
	gint				new_image_width;			// Receives the width of the new image
	GtkTreePath			*old_path = NULL;			// The old path, which we'll free
	GString				*path_gstring;				// Holds the file selection path
	GtkWidget			*path_widget;				// File selection widget
	slide				*slide_data;				// Pointer to the data for the current slide
	gboolean			useable_input;				// Used as a flag to indicate if all validation was successful
	GString				*valid_image_path;			// Receives the new image path once validated
	GString				*validated_string;			// Receives known good strings from the validation function

	layer_image			*tmp_image_ob;				// Temporary image layer object
	GtkTreeIter			*tmp_iter;					// Temporary iter
	layer				*tmp_layer;					// Temporary layer
	GtkTreePath			*tmp_path;					// Temporary GtkPath


	// If no project is loaded then don't run this function
	if (NULL == current_slide)
	{
		// Make a beep, then return
		gdk_beep();
		return;
	}

	// Initialise some things
	path_gstring = g_string_new(NULL);
	valid_image_path = g_string_new(NULL);

	// Change the cursor back to normal
	gdk_window_set_cursor(main_drawing_area->window, NULL);

	// * Create a new image layer in memory using reasonable defaults *
	
	// Simplify pointing to the current slide structure in memory
	slide_data = current_slide->data;

	// Create the image layer data
	tmp_image_ob = g_new(layer_image, 1);
	tmp_image_ob->modified = FALSE;

	// Constuct the new image layer with some defaults
	tmp_layer = g_new(layer, 1);
	tmp_layer->object_type = TYPE_GDK_PIXBUF;
	tmp_layer->object_data = (GObject *) tmp_image_ob;
	tmp_layer->start_time = 0.0;
	tmp_layer->duration = default_layer_duration;
	tmp_layer->x_offset_start = release_x;
	tmp_layer->y_offset_start = release_y;
	tmp_layer->x_offset_finish = release_x;
	tmp_layer->y_offset_finish = release_y;
	tmp_layer->visible = TRUE;
	tmp_layer->background = FALSE;
	tmp_layer->name = g_string_new("Image");
	tmp_layer->external_link = g_string_new(NULL);
	tmp_layer->external_link_window = g_string_new("_self");
	tmp_layer->transition_in_type = TRANS_LAYER_NONE;
	tmp_layer->transition_in_duration = 0.0;
	tmp_layer->transition_out_type = TRANS_LAYER_NONE;
	tmp_layer->transition_out_duration = 0.0;

	// Open a dialog asking the user to select an image
	path_widget = gtk_file_chooser_dialog_new("Please choose an image file", GTK_WINDOW(main_window), GTK_FILE_CHOOSER_ACTION_OPEN,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,  // Cancel button
				      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,  // Open button
				      NULL);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(path_widget), last_folder->str);
	gtk_widget_show_all(GTK_WIDGET(path_widget));

	// Loop around until we have all valid values, or the user cancels out
	validated_string = NULL;
	do
	{
		// Display the dialog
		if (GTK_RESPONSE_ACCEPT != gtk_dialog_run(GTK_DIALOG(path_widget)))
		{
			// The dialog was cancelled, so destroy it and return to the caller
			gtk_widget_destroy(GTK_WIDGET(path_widget));
			g_string_free(path_gstring, TRUE);
			g_string_free(valid_image_path, TRUE);
			return;
		}

		// Grab the folder the user may have changed too  
		last_folder = g_string_assign(last_folder, gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(path_widget)));

		// Reset the useable input flag
		useable_input = TRUE;

		// Retrieve the new image path
		validated_string = validate_value(FILE_PATH, V_CHAR, (gchar *) gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(path_widget)));
		if (NULL == validated_string)
		{
			display_warning("Error ED330: There was something wrong with the image path given.  Please try again.");
			useable_input = FALSE;
		} else
		{
			// Load the image
			new_image_data = gdk_pixbuf_new_from_file(validated_string->str, NULL);
			if (NULL == new_image_data)
			{
				display_warning("Error ED331: There was something wrong with the image file selected.  Please try again.");
				useable_input = FALSE;					
			} else
			{
				// Retrieve the image dimensions
				new_image_height = gdk_pixbuf_get_height(new_image_data);
				new_image_width = gdk_pixbuf_get_width(new_image_data);
				g_string_free(validated_string, TRUE);
				validated_string = NULL;

				// Create the rest of the image layer data
				tmp_image_ob->image_data = new_image_data;
				tmp_image_ob->width = gdk_pixbuf_get_width(new_image_data);
				tmp_image_ob->height = gdk_pixbuf_get_height(new_image_data);
				g_string_free(validated_string, TRUE);
				validated_string = NULL;
			}
		}

	} while (FALSE == useable_input);

	// Destroy the file selection dialog box
	gtk_widget_destroy(GTK_WIDGET(path_widget));

	// * To get here, the user must have chosen a valid image file, so we process the results *

	// Add the new layer to the slide
	layer_pointer = slide_data->layers;
	layer_pointer = g_list_first(layer_pointer);
	layer_pointer = g_list_prepend(layer_pointer, tmp_layer);

	// If the new layer end time is longer than the slide duration, then extend the slide duration
	if ((tmp_layer->start_time + tmp_layer->duration + tmp_layer->transition_in_duration + tmp_layer->transition_out_duration) > slide_data->duration)
	{
		slide_data->duration = tmp_layer->start_time + tmp_layer->duration + tmp_layer->transition_in_duration + tmp_layer->transition_out_duration;
	}

	// Add the new layer to slide list store
	tmp_iter = g_new(GtkTreeIter, 1);
	tmp_layer->row_iter = tmp_iter;
	gtk_list_store_prepend(slide_data->layer_store, tmp_iter);
	gtk_list_store_set(slide_data->layer_store, tmp_iter,
						TIMELINE_NAME, tmp_layer->name->str,
						TIMELINE_VISIBILITY, TRUE,
						TIMELINE_DURATION, NULL,
						TIMELINE_X_OFF_START, tmp_layer->x_offset_start,
						TIMELINE_Y_OFF_START, tmp_layer->y_offset_start,
						TIMELINE_X_OFF_FINISH, tmp_layer->x_offset_finish,
						TIMELINE_Y_OFF_FINISH, tmp_layer->y_offset_finish,
						-1);

	// Regenerate the timeline
	gtk_widget_destroy(GTK_WIDGET(slide_data->timeline_widget));
	slide_data->timeline_widget = NULL;
	draw_timeline();

	// Redraw the workspace
	draw_workspace();

	// Recreate the slide thumbnail
	film_strip_create_thumbnail(slide_data);

	// Select the new layer in the timeline widget
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(slide_data->timeline_widget), &tmp_path, NULL);
	if (NULL != tmp_path)
		old_path = tmp_path;  // Make a backup of the old path, so we can free it
	tmp_path = gtk_tree_path_new_first();
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(slide_data->timeline_widget), tmp_path, NULL, FALSE);
	if (NULL != old_path)
		gtk_tree_path_free(old_path);  // Free the old path

	// Set the changes made variable
	changes_made = TRUE;

	// Update the status bar
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, " Image layer added");
	gdk_flush();
}
