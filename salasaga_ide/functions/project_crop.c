/*
 * $Id$
 *
 * Flame Project: Function called when the user clicks the Crop timeline toolbar button 
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


// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"
#include "draw_timeline.h"
#include "draw_workspace.h"
#include "regenerate_film_strip_thumbnails.h"


void project_crop(void)
{
	// Local variables
	GtkDialog			*crop_dialog;				// Widget for the dialog
	GtkWidget			*crop_table;				// Table used for neat layout of the dialog box
	guint				row_counter = 0;			// Used to count which row things are up to
	gint				dialog_result;				// Catches the return code from the dialog box

	gint				bottom_value;
	layer				*last_layer;				// Temporary layer
	GList				*layer_pointer;				// Points to the layers in the selected slide
	gint				left_value;
	gint				new_height;					// Hold the height of the cropped area
	GdkPixbuf			*new_pixbuf;				// Holds the cropped image data
	gint				new_width;					// Hold the width of the cropped area
	gint				num_slides;					// Total number of layers
	gint				right_value;
	gint				slide_counter;
	slide				*slide_data;
	gint				top_value;

	GtkWidget			*left_label;				// Label widget
	GtkWidget			*left_button;				//

	GtkWidget			*right_label;				// Label widget
	GtkWidget			*right_button;				//

	GtkWidget			*top_label;					// Label widget
	GtkWidget			*top_button;				//

	GtkWidget			*bottom_label;				// Label widget
	GtkWidget			*bottom_button;				//

	GdkPixbuf			*tmp_pixbuf;				// Temporary pixbuf


	// If no project is loaded then don't run this function
	if (NULL == current_slide)
	{
		// Make a beep, then return
		gdk_beep();
		return;
	}

	// * Pop open a dialog box asking the user how much to crop off each side of the image *

	// Create the dialog window, and table to hold its children
	crop_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons("Crop image layer", GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL));
	crop_table = gtk_table_new(3, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(crop_dialog->vbox), GTK_WIDGET(crop_table), FALSE, FALSE, 10);

	// Create the label asking for the left side crop amount
	left_label = gtk_label_new("Left crop: ");
	gtk_misc_set_alignment(GTK_MISC(left_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(crop_table), GTK_WIDGET(left_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the left side crop amount
	left_button = gtk_spin_button_new_with_range(0, project_width, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(left_button), 0);
	gtk_table_attach(GTK_TABLE(crop_table), GTK_WIDGET(left_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the right side crop amount
	right_label = gtk_label_new("Right crop: ");
	gtk_misc_set_alignment(GTK_MISC(right_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(crop_table), GTK_WIDGET(right_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the right side crop amount
	right_button = gtk_spin_button_new_with_range(0, project_width, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(right_button), 0);
	gtk_table_attach(GTK_TABLE(crop_table), GTK_WIDGET(right_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the top crop amount
	top_label = gtk_label_new("Top crop: ");
	gtk_misc_set_alignment(GTK_MISC(top_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(crop_table), GTK_WIDGET(top_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the left side crop amount
	top_button = gtk_spin_button_new_with_range(0, project_height, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(top_button), 0);
	gtk_table_attach(GTK_TABLE(crop_table), GTK_WIDGET(top_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the right side crop amount
	bottom_label = gtk_label_new("Bottom crop: ");
	gtk_misc_set_alignment(GTK_MISC(bottom_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(crop_table), GTK_WIDGET(bottom_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the right side crop amount
	bottom_button = gtk_spin_button_new_with_range(0, project_height, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(bottom_button), 0);
	gtk_table_attach(GTK_TABLE(crop_table), GTK_WIDGET(bottom_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Run the dialog
	gtk_widget_show_all(GTK_WIDGET(crop_dialog));
	dialog_result = gtk_dialog_run(GTK_DIALOG(crop_dialog));

	// Was the OK button pressed?
	if (GTK_RESPONSE_ACCEPT != dialog_result)
	{
		// The dialog was cancelled, so destroy the dialog box and return
		gtk_widget_destroy(GTK_WIDGET(crop_dialog));
		return;
	}

	// Get the values from the dialog
	left_value = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(left_button));
	right_value = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(right_button));
	top_value = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(top_button));
	bottom_value = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(bottom_button));

	// Destroy the dialog box
	gtk_widget_destroy(GTK_WIDGET(crop_dialog));

	// Loop through the slide structure, cropping the backgrounds
	slides = g_list_first(slides);
	num_slides = g_list_length(slides);
	for (slide_counter = 0; slide_counter < num_slides; slide_counter++)
	{
		slide_data = g_list_nth_data(slides, slide_counter);
		layer_pointer = slide_data->layers;
		layer_pointer = g_list_last(layer_pointer);

		// * Check if this slide has a background image *
		last_layer = layer_pointer->data;

		// Is this layer an image?
		if (TYPE_GDK_PIXBUF != last_layer->object_type)
		{
			// No it's not, so skip this slide
			continue;
		}

		// Is this layer the background?
		if (TRUE != last_layer->background)
		{
			// No it's not, so skip this slide
			continue;
		}

		// Create a new pixbuf, for storing the cropped image in
		new_height = gdk_pixbuf_get_height(((layer_image *) last_layer->object_data)->image_data)
			- top_value
			- bottom_value;
		new_width = gdk_pixbuf_get_width(((layer_image *) last_layer->object_data)->image_data)
			- left_value
			- right_value;
		new_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, new_width, new_height);

		// Create a new pixbuf, having just the cropped image data in it
		gdk_pixbuf_copy_area(((layer_image *) last_layer->object_data)->image_data, // Source pixbuf
			left_value,  // Left crop
			top_value,  // Top crop
			new_width,  // Width
			new_height,  // Height
			new_pixbuf,  // Destination
			0, 0);

		// Update the layer with the new cropped data
		tmp_pixbuf = ((layer_image *) last_layer->object_data)->image_data;
		((layer_image *) last_layer->object_data)->image_data = new_pixbuf;
		((layer_image *) last_layer->object_data)->width = new_width;
		((layer_image *) last_layer->object_data)->height = new_height;
		g_string_assign(((layer_image *) last_layer->object_data)->image_path, "");

		// Free the memory used by the old pixbuf
		g_object_unref(tmp_pixbuf);
	}

	// Update project width and height global variables
	project_height = project_height - top_value - bottom_value;
	project_width = project_width - left_value - right_value;

	// Redraw the timeline
	draw_timeline();

	// Redraw the workspace
	draw_workspace();

	// Recreate the film strip thumbnails
	regenerate_film_strip_thumbnails();
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.6  2008/02/16 11:17:56  vapour
 * Removed un-needed include file.
 *
 * Revision 1.5  2008/02/12 14:19:39  vapour
 * Updated to use the new background field in the layer structure.
 *
 * Revision 1.4  2008/02/04 14:40:52  vapour
 *  + Removed unnecessary includes.
 *  + Improved spacing between table cells.
 *
 * Revision 1.3  2008/01/15 16:18:59  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.2  2007/10/06 11:38:28  vapour
 * Continued adjusting function include definitions.
 *
 * Revision 1.1  2007/09/29 04:22:12  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */
