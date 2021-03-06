/*
 * $Id$
 *
 * Salasaga: Displays a dialog box asking for the values required to make a new image layer
 *
 * Copyright (C) 2005-2010 Digital Distribution Global Training Solutions Pty. Ltd.
 * <justin@salasaga.org>
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


// GTK includes
#include <gtk/gtk.h>

// Salasaga includes
#include "../../salasaga_types.h"
#include "../global_functions.h"
#include "../cairo/create_cairo_pixbuf_pattern.h"
#include "../dialog/display_warning.h"
#include "../film_strip/film_strip_create_thumbnail.h"
#include "../layer/layer_duplicate.h"
#include "../other/validate_value.h"
#include "../preference/application_preferences.h"
#include "../time_line/draw_timeline.h"
#include "../time_line/time_line_set_selected_layer_num.h"
#include "../undo_redo/undo_functions.h"
#include "../working_area/draw_workspace.h"


void layer_new_image_inner(guint release_x, guint release_y)
{
	// Local variables
	GList				*layer_pointer;				// Points to the layers in the selected slide
	GString				*message;					// Used to construct message strings
	GdkPixbuf			*new_image_data;			// Receives the new image data
	gint				new_image_height;			// Receives the height of the new image
	gint				new_image_width;			// Receives the width of the new image
	GString				*path_gstring;				// Holds the file selection path
	GtkWidget			*path_widget;				// File selection widget
	slide				*slide_data;				// Pointer to the data for the current slide
	layer_image			*tmp_image_ob;				// Temporary image layer object
	layer				*tmp_layer;					// Temporary layer
	gboolean			usable_input;				// Used as a flag to indicate if all validation was successful
	GString				*valid_image_path;			// Receives the new image path once validated
	GString				*validated_string;			// Receives known good strings from the validation function
	undo_history_data	*undo_item_data = NULL;		// Memory structure undo history items are created in


	// If no project is loaded then don't run this function
	if (NULL == get_current_slide())
	{
		// Make a beep, then return
		gdk_beep();
		return;
	}

	// Initialise some things
	message = g_string_new(NULL);
	path_gstring = g_string_new(NULL);
	valid_image_path = g_string_new(NULL);

	// Change the cursor back to normal
	gdk_window_set_cursor(get_main_drawing_area()->window, NULL);

	// * Create a new image layer in memory using reasonable defaults *

	// Simplify pointing to the current slide structure in memory
	slide_data = get_current_slide_data();

	// Create the image layer data
	tmp_image_ob = g_new(layer_image, 1);
	tmp_image_ob->modified = FALSE;

	// Constuct the new image layer with some defaults
	tmp_layer = g_new(layer, 1);
	tmp_layer->object_type = TYPE_GDK_PIXBUF;
	tmp_layer->object_data = (GObject *) tmp_image_ob;
	tmp_layer->start_time = 0.0;
	tmp_layer->duration = get_default_layer_duration();
	tmp_layer->x_offset_start = release_x;
	tmp_layer->y_offset_start = release_y;
	tmp_layer->x_offset_finish = release_x;
	tmp_layer->y_offset_finish = release_y;
	tmp_layer->visible = TRUE;
	tmp_layer->background = FALSE;
	tmp_layer->name = g_string_new(_("Image"));
	tmp_layer->external_link = g_string_new(NULL);
	tmp_layer->external_link_window = g_string_new(_("_self"));
	tmp_layer->transition_in_type = TRANS_LAYER_NONE;
	tmp_layer->transition_in_duration = 0.0;
	tmp_layer->transition_out_type = TRANS_LAYER_NONE;
	tmp_layer->transition_out_duration = 0.0;

	// Open a dialog asking the user to select an image
	path_widget = gtk_file_chooser_dialog_new(_("Please choose an image file"), GTK_WINDOW(get_main_window()), GTK_FILE_CHOOSER_ACTION_OPEN,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,  // Cancel button
				      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,  // Open button
				      NULL);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(path_widget), get_last_folder());
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
		set_last_folder(gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(path_widget)));

		// Reset the usable input flag
		usable_input = TRUE;

		// Retrieve the new image path
		validated_string = validate_value(FILE_PATH, V_CHAR, (gchar *) gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(path_widget)));
		if (NULL == validated_string)
		{
			g_string_printf(message, "%s ED330: %s", _("Error"), _("There was something wrong with the image path given.  Please try again."));
			display_warning(message->str);
			usable_input = FALSE;
		} else
		{
			// Load the image
			new_image_data = gdk_pixbuf_new_from_file(validated_string->str, NULL);
			if (NULL == new_image_data)
			{
				g_string_printf(message, "%s ED331: %s", _("Error"), _("There was something wrong with the image file selected.  Please try again."));
				display_warning(message->str);
				usable_input = FALSE;
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

				// Create a cairo pattern from the image data
				tmp_image_ob->cairo_pattern = create_cairo_pixbuf_pattern(tmp_image_ob->image_data);
				if (NULL == tmp_image_ob->cairo_pattern)
				{
					// Something went wrong when creating the image pattern
					g_string_printf(message, "%s ED372: %s", _("Error"), _("Couldn't create an image pattern."));
					display_warning(message->str);
					g_string_free(message, TRUE);
					return;
				}
			}
		}

	} while (FALSE == usable_input);

	// Destroy the file selection dialog box
	gtk_widget_destroy(GTK_WIDGET(path_widget));

	// * To get here, the user must have chosen a valid image file, so we process the results *

	// Create and store the undo history item for this layer
	undo_item_data = g_new0(undo_history_data, 1);
	undo_item_data->layer_data_new = layer_duplicate(tmp_layer);
	undo_item_data->layer_data_old = NULL;  // NULL means not set
	undo_item_data->position_new = 0;
	undo_item_data->position_old = -1;  // -1 means not set
	undo_item_data->slide_data = get_current_slide_data();
	undo_history_add_item(UNDO_INSERT_LAYER, undo_item_data, TRUE);

	// Add the new layer to the slide
	layer_pointer = slide_data->layers;
	layer_pointer = g_list_first(layer_pointer);
	layer_pointer = g_list_prepend(layer_pointer, tmp_layer);
	slide_data->num_layers++;

	// If the new layer end time is longer than the slide duration, then extend the slide duration
	if (tmp_layer->duration > slide_data->duration)
	{
		// Change the slide duration
		slide_data->duration = tmp_layer->duration;

		// Change the background layer duration
		tmp_layer = g_list_nth_data(layer_pointer, slide_data->num_layers - 1);
		tmp_layer->duration = slide_data->duration;
	}

	// Free the memory used in this function
	g_string_free(message, TRUE);

	// Regenerate the timeline
	draw_timeline();

	// Redraw the workspace
	draw_workspace();

	// Recreate the slide thumbnail
	film_strip_create_thumbnail(slide_data);

	// Select the new layer in the timeline widget
	time_line_set_selected_layer_num(slide_data->timeline_widget, 0);

	// Set the changes made variable
	set_changes_made(TRUE);

	// Update the status bar
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), _(" Image layer added"));
	gdk_flush();
}
