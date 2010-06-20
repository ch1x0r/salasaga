/*
 * $Id$
 *
 * Salasaga: Function called when the user chooses Layer -> Image resize from the top menu
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

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../../salasaga_types.h"
#include "../global_functions.h"
#include "../cairo/create_cairo_pixbuf_pattern.h"
#include "../dialog/display_warning.h"
#include "../film_strip/regenerate_film_strip_thumbnails.h"
#include "../time_line/time_line_get_selected_layer_num.h"
#include "../working_area/draw_workspace.h"


void image_resize(void)
{
	// Local variables
	gboolean			acceptable_result = FALSE;	// Toggle to determine if we've received valid adjustment values from the user
	GtkDialog			*adjustment_dialog;			// Widget for the dialog
	GtkWidget			*adjustment_table;			// Table used for neat layout of the dialog box
	gint				dialog_result;				// Catches the return code from the dialog box
	gint				image_height;				// Width of the image being resized
	gint				image_width;				// Height of the image being resized
	GList				*layer_pointer;				// Points to the layers in the selected slide
	GString				*message;					// Used to construct message strings
	gint				new_height;					// Hold the height of the adjusted area
	GdkPixbuf			*new_pixbuf;				// Holds the adjusted image data
	gint				new_width;					// Hold the width of the adjusted area
	gint				resize_type;				// Receives the type of scaling algorithm to be applied to the image
	guint				row_counter = 0;			// Used to count which row things are up to
	gdouble				scale_mark_counter;			// Simple counter used when constructing scale marks for sliders
	guint				selected_row;				// Holds the row that is selected
	slide				*slide_data;				// Pointer to current slide data
	gint				table_padding_x;			// Amount of padding to use in the table
	gint				table_padding_y;			// Amount of padding to use in the table
	layer				*this_layer = NULL;			// Temporary layer
	layer_image			*tmp_image_ob;				// Points to the image data in the selected layer

	GtkWidget			*new_width_label;			// Label widget
	GtkWidget			*new_width_scale;			//
	GtkWidget			*new_width_pixels_label;	// Label widget

	GtkWidget			*new_height_label;			// Label widget
	GtkWidget			*new_height_scale;			//
	GtkWidget			*new_height_pixels_label;	// Label widget

	GtkWidget			*resize_type_label;			// Label widget
	GtkWidget			*selector_resize_type;		// Label widget


	// If no project is loaded then don't run this function
	if (NULL == get_current_slide())
	{
		// Make a beep, then return
		gdk_beep();
		return;
	}

	// Initialisation
	message = g_string_new(NULL);
	table_padding_x = get_table_x_padding();
	table_padding_y = get_table_y_padding();

	// Get a pointer to the layer we're resizing
	slide_data = get_current_slide_data();
	selected_row = time_line_get_selected_layer_num(slide_data->timeline_widget);
	layer_pointer = slide_data->layers;
	layer_pointer = g_list_first(layer_pointer);
	this_layer = g_list_nth_data(layer_pointer, selected_row);

	// Is this layer an image?
	if (TYPE_GDK_PIXBUF != this_layer->object_type)
	{
		// No it's not, so this function doesn't apply to this layer
		return;
	}

	// Is this layer the background?
	if (TRUE == this_layer->background)
	{
		// Yes it is, so this function doesn't apply to this layer
		return;
	}

	// Point to the image layer data
	tmp_image_ob = (layer_image *) this_layer->object_data;
	image_width = tmp_image_ob->width;
	image_height = tmp_image_ob->height;

	// * Pop open a dialog box asking the user how much to adjust the dimensions of the image by *

	// Create the dialog window, and table to hold its children
	adjustment_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(_("Resize image layer"), GTK_WINDOW(get_main_window()), GTK_DIALOG_MODAL, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL));
	adjustment_table = gtk_table_new(3, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(adjustment_dialog->vbox), GTK_WIDGET(adjustment_table), FALSE, FALSE, 10);

	// Create the label asking for the new image width
	new_width_label = gtk_label_new(_("Image width:"));
	gtk_misc_set_alignment(GTK_MISC(new_width_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(adjustment_table), GTK_WIDGET(new_width_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);

	// Create the slider asking for the new image width
	new_width_scale = gtk_hscale_new_with_range(get_valid_fields_min_value(LAYER_WIDTH), get_valid_fields_max_value(LAYER_WIDTH), 1);
	gtk_range_set_value(GTK_RANGE(new_width_scale), image_width);
	gtk_scale_add_mark(GTK_SCALE(new_width_scale), image_width, GTK_POS_TOP, NULL);
	for (scale_mark_counter = 512; scale_mark_counter <= get_valid_fields_max_value(LAYER_WIDTH); scale_mark_counter += 512)
	{
		// Add scale marks
		gtk_scale_add_mark(GTK_SCALE(new_width_scale), scale_mark_counter, GTK_POS_BOTTOM, NULL);
	}
	gtk_table_attach(GTK_TABLE(adjustment_table), GTK_WIDGET(new_width_scale), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);

	// Create the new image width "pixels" string
	new_width_pixels_label = gtk_label_new(_("pixels"));
	gtk_misc_set_alignment(GTK_MISC(new_width_pixels_label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(adjustment_table), GTK_WIDGET(new_width_pixels_label), 2, 3, row_counter, row_counter + 1, 0, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	row_counter = row_counter + 1;

	// Create the label asking for the new image height
	new_height_label = gtk_label_new(_("Image height:"));
	gtk_misc_set_alignment(GTK_MISC(new_height_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(adjustment_table), GTK_WIDGET(new_height_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);

	// Create the slider asking for the new image height
	new_height_scale = gtk_hscale_new_with_range(get_valid_fields_min_value(LAYER_HEIGHT), get_valid_fields_max_value(LAYER_HEIGHT), 1);
	gtk_range_set_value(GTK_RANGE(new_height_scale), image_height);
	gtk_scale_add_mark(GTK_SCALE(new_height_scale), image_height, GTK_POS_TOP, NULL);
	for (scale_mark_counter = 512; scale_mark_counter <= get_valid_fields_max_value(LAYER_HEIGHT); scale_mark_counter += 512)
	{
		// Add scale marks
		gtk_scale_add_mark(GTK_SCALE(new_height_scale), scale_mark_counter, GTK_POS_BOTTOM, NULL);
	}
	gtk_table_attach(GTK_TABLE(adjustment_table), GTK_WIDGET(new_height_scale), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);

	// Create the new image height "pixels" string
	new_height_pixels_label = gtk_label_new(_("pixels"));
	gtk_misc_set_alignment(GTK_MISC(new_height_pixels_label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(adjustment_table), GTK_WIDGET(new_height_pixels_label), 2, 3, row_counter, row_counter + 1, 0, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	row_counter = row_counter + 1;

	// Create the label asking for the image resize type
	resize_type_label = gtk_label_new(_("Resize image using:"));
	gtk_misc_set_alignment(GTK_MISC(resize_type_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(adjustment_table), GTK_WIDGET(resize_type_label), 0, 1, row_counter, row_counter + 1, 0, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);

	// Create the selector for the image resize type
	selector_resize_type = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(selector_resize_type), _("Nearest neighbor (lowest quality)"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(selector_resize_type), _("Tiled"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(selector_resize_type), _("Bilinear (default)"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(selector_resize_type), _("Hyperbolic (best quality for photos)"));
	gtk_combo_box_set_active(GTK_COMBO_BOX(selector_resize_type), 2);
	gtk_table_attach(GTK_TABLE(adjustment_table), GTK_WIDGET(selector_resize_type), 1, 3, row_counter, row_counter + 1, 0, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	row_counter = row_counter + 1;

	// Run the dialog
	gtk_widget_show_all(GTK_WIDGET(adjustment_dialog));
	while (FALSE == acceptable_result)
	{
		dialog_result = gtk_dialog_run(GTK_DIALOG(adjustment_dialog));

		// Was the OK button pressed?
		if (GTK_RESPONSE_ACCEPT != dialog_result)
		{
			// The dialog was cancelled, so destroy the dialog box and return
			gtk_widget_destroy(GTK_WIDGET(adjustment_dialog));
			return;
		}

		// Get the values from the dialog
		new_width = (gint) gtk_range_get_value(GTK_RANGE(new_width_scale));
		new_height = (gint) gtk_range_get_value(GTK_RANGE(new_height_scale));

		// If the user is adjusting the size out of acceptable limits, warn them and loop again
		if ((get_valid_fields_min_value(PROJECT_HEIGHT) <= new_height) && (get_valid_fields_max_value(PROJECT_HEIGHT) >= new_height) && (get_valid_fields_min_value(PROJECT_WIDTH) <= new_width) && (get_valid_fields_max_value(PROJECT_WIDTH) >= new_width))
		{
			acceptable_result = TRUE;
		} else
		{
			if ((get_valid_fields_min_value(PROJECT_HEIGHT) > new_height) || (get_valid_fields_min_value(PROJECT_WIDTH) > new_width))
			{
				g_string_printf(message, "%s ED450: %s\n\n%s", _("Error"), _("Those adjustment values would make the image smaller than the minimum allowed."), _("Please try again."));
				display_warning(message->str);
			} else
			{
				g_string_printf(message, "%s ED451: %s\n\n%s", _("Error"), _("Those adjustment values would make the image larger than the maximum allowed."), _("Please try again."));
				display_warning(message->str);
			}
		}
	}

	// Get resize type
	resize_type = gtk_combo_box_get_active(GTK_COMBO_BOX(selector_resize_type));

	// Destroy the dialog box
	gtk_widget_destroy(GTK_WIDGET(adjustment_dialog));

	// If the new image size is the same as the old image size, we skip the rest of the function
	if ((new_height == image_height) && (new_width == image_width))
	{
		// Free the memory used in this function
		g_string_free(message, TRUE);

		// Update the status bar
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), _("Image layer resize skipped"));
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(get_status_bar()), 0.0);
		gdk_flush();

		return;
	}

	// Scale the existing image layer to the new dimensions
	new_pixbuf = gdk_pixbuf_scale_simple(GDK_PIXBUF(tmp_image_ob->image_data), new_width, new_height, resize_type);

	// Update the layer with the new adjusted data
	g_object_unref(GDK_PIXBUF(tmp_image_ob->image_data));  // Free the memory used by the old pixbuf
	tmp_image_ob->image_data = new_pixbuf;
	tmp_image_ob->width = new_width;
	tmp_image_ob->height = new_height;

	// Create a cairo pattern from the image data
	tmp_image_ob->cairo_pattern = create_cairo_pixbuf_pattern(tmp_image_ob->image_data);
	if (NULL == tmp_image_ob->cairo_pattern)
	{
		// Something went wrong when creating the image pattern
		g_string_printf(message, "%s ED452: %s", _("Error"), _("Couldn't create an image pattern."));
		display_warning(message->str);
		return;
	}

	// Recreate the film strip thumbnails
	regenerate_film_strip_thumbnails();

	// Redraw the workspace
	//draw_workspace();

	// Set the changes made variable
	set_changes_made(TRUE);

	// Show movement on the progress bar
	gtk_progress_bar_pulse(GTK_PROGRESS_BAR(get_status_bar()));
	gdk_flush();

	// Update the status bar
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), _("Image layer resized"));
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(get_status_bar()), 0.0);
	gdk_flush();

	// Free the memory used in this function
	g_string_free(message, TRUE);
}
