/*
 * $Id$
 *
 * Salasaga: Function called when the user clicks the Crop Image toolbar button
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


// Turn on C99 compatibility - needed for roundf() to work
#define _ISOC99_SOURCE

// Math include
#include <math.h>

// Standard includes
#include <stdlib.h>

// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../cairo/create_cairo_pixbuf_pattern.h"
#include "../dialog/display_warning.h"
#include "../film_strip/film_strip_create_thumbnail.h"
#include "../other/widget_focus.h"
#include "../time_line/draw_timeline.h"
#include "../time_line/time_line_get_selected_layer_num.h"
#include "../working_area/draw_workspace.h"


void image_crop(void)
{
	// Local variables
	gboolean			acceptable_result = FALSE;	// Toggle to determine if we've received valid crop values from the user
	GtkDialog			*crop_dialog;				// Widget for the dialog
	GtkWidget			*crop_table;				// Table used for neat layout of the dialog box
	gint				dialog_height;				// Height of the dialog in pixels
	gint				dialog_result;				// Catches the return code from the dialog box
	gint				dialog_width;				// Width of the dialog in pixels
	GList				*layer_pointer;				// Points to the layers in the selected slide
	GString				*message;					// Used to construct message strings
	GdkPixbuf			*old_pixbuf;				// Pointer to the full size (old) pixbuf
	gint				new_height;					// Hold the height of the cropped area
	GdkPixbuf			*new_pixbuf;				// Holds the cropped image data
	gint				new_width;					// Hold the width of the cropped area
	guint				row_counter = 0;			// Used to count which row things are up to
	gdouble				scale_mark_counter;			// Simple counter used when constructing scale marks for sliders
	guint				selected_row;				// Holds the number of the row that is selected
	gint				table_padding_x;			// Amount of padding to use in the table
	gint				table_padding_y;			// Amount of padding to use in the table
	layer				*this_layer;				// Temporary layer
	layer_image			*tmp_image_ob;				// Points to the image data in the selected layer

	GtkWidget			*left_label;				// Label widget
	GtkWidget			*left_slider;				//
	GtkWidget			*left_pixels_label;			// Label widget

	GtkWidget			*right_label;				// Label widget
	GtkWidget			*right_slider;				//
	GtkWidget			*right_pixels_label;		// Label widget

	GtkWidget			*top_label;					// Label widget
	GtkWidget			*top_slider;				//
	GtkWidget			*top_pixels_label;			// Label widget

	GtkWidget			*bottom_label;				// Label widget
	GtkWidget			*bottom_slider;				//
	GtkWidget			*bottom_pixels_label;		// Label widget


	// If no project is loaded then don't run this function
	if (NULL == get_current_slide())
	{
		// Make a beep, then return
		gdk_beep();
		return;
	}

	// Initialise some variables
	layer_pointer = get_current_slide_layers_pointer();
	message = g_string_new(NULL);
	table_padding_x = get_table_x_padding();
	table_padding_y = get_table_y_padding();

	// Change the focus of the window to be this widget
	set_delete_focus(FOCUS_LAYER);

	// * Check if the selected layer is an image *

	// Determine which layer the user has selected in the timeline
	selected_row = time_line_get_selected_layer_num(get_current_slide_timeline_widget());
	layer_pointer = g_list_first(layer_pointer);
	this_layer = g_list_nth_data(layer_pointer, selected_row);
	tmp_image_ob = (layer_image *) this_layer->object_data;

	// Is this layer an image?
	if (TYPE_GDK_PIXBUF != this_layer->object_type)
	{
		// Give the user feedback
		gdk_beep();
		g_string_printf(message, "%s ED36: %s", _("Error"), _("Only Image layers can be cropped."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		return;
	}

	// Is this layer the background?
	if (TRUE == this_layer->background)
	{
		// Give the user feedback
		gdk_beep();
		g_string_printf(message, "%s ED37: %s", _("Error"), _("Background layers can not be cropped."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		return;
	}

	// * Pop open a dialog box asking the user how much to crop off each side of the image *

	// Create the dialog window, and table to hold its children
	crop_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(_("Crop image layer"), GTK_WINDOW(get_main_window()), GTK_DIALOG_MODAL, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL));
	crop_table = gtk_table_new(4, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(crop_dialog->vbox), GTK_WIDGET(crop_table), FALSE, FALSE, 10);

	// Create the label asking for the left side crop amount
	left_label = gtk_label_new(_("Left crop: "));
	gtk_misc_set_alignment(GTK_MISC(left_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(crop_table), GTK_WIDGET(left_label), 0, 1, row_counter, row_counter + 1, 0, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);

	// Create the slider that accepts the left side crop amount
	left_slider = gtk_hscale_new_with_range(0, valid_fields[LAYER_WIDTH].max_value, 1);
	gtk_range_set_value(GTK_RANGE(left_slider), 0);
	for (scale_mark_counter = 1024; scale_mark_counter <= valid_fields[LAYER_WIDTH].max_value; scale_mark_counter += 1024)
	{
		// Add scale marks
		gtk_scale_add_mark(GTK_SCALE(left_slider), scale_mark_counter, GTK_POS_BOTTOM, NULL);
	}
	gtk_table_attach(GTK_TABLE(crop_table), GTK_WIDGET(left_slider), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);

	// Create the left crop "pixels" string
	left_pixels_label = gtk_label_new(_("pixels"));
	gtk_misc_set_alignment(GTK_MISC(left_pixels_label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(crop_table), GTK_WIDGET(left_pixels_label), 2, 3, row_counter, row_counter + 1, 0, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	row_counter = row_counter + 1;

	// Create the label asking for the right side crop amount
	right_label = gtk_label_new(_("Right crop: "));
	gtk_misc_set_alignment(GTK_MISC(right_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(crop_table), GTK_WIDGET(right_label), 0, 1, row_counter, row_counter + 1, 0, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);

	// Create the slider that accepts the right side crop amount
	right_slider = gtk_hscale_new_with_range(0, valid_fields[LAYER_WIDTH].max_value, 1);
	gtk_range_set_value(GTK_RANGE(right_slider), 0);
	for (scale_mark_counter = 1024; scale_mark_counter <= valid_fields[LAYER_WIDTH].max_value; scale_mark_counter += 1024)
	{
		// Add scale marks
		gtk_scale_add_mark(GTK_SCALE(right_slider), scale_mark_counter, GTK_POS_BOTTOM, NULL);
	}
	gtk_table_attach(GTK_TABLE(crop_table), GTK_WIDGET(right_slider), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);

	// Create the right crop "pixels" string
	right_pixels_label = gtk_label_new(_("pixels"));
	gtk_misc_set_alignment(GTK_MISC(right_pixels_label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(crop_table), GTK_WIDGET(right_pixels_label), 2, 3, row_counter, row_counter + 1, 0, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	row_counter = row_counter + 1;

	// Create the label asking for the top crop amount
	top_label = gtk_label_new(_("Top crop: "));
	gtk_misc_set_alignment(GTK_MISC(top_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(crop_table), GTK_WIDGET(top_label), 0, 1, row_counter, row_counter + 1, 0, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);

	// Create the slider that accepts the top crop amount
	top_slider = gtk_hscale_new_with_range(0, valid_fields[LAYER_HEIGHT].max_value, 1);
	gtk_range_set_value(GTK_RANGE(top_slider), 0);
	for (scale_mark_counter = 1024; scale_mark_counter <= valid_fields[LAYER_HEIGHT].max_value; scale_mark_counter += 1024)
	{
		// Add scale marks
		gtk_scale_add_mark(GTK_SCALE(top_slider), scale_mark_counter, GTK_POS_BOTTOM, NULL);
	}
	gtk_table_attach(GTK_TABLE(crop_table), GTK_WIDGET(top_slider), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);

	// Create the top crop "pixels" string
	top_pixels_label = gtk_label_new(_("pixels"));
	gtk_misc_set_alignment(GTK_MISC(top_pixels_label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(crop_table), GTK_WIDGET(top_pixels_label), 2, 3, row_counter, row_counter + 1, 0, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	row_counter = row_counter + 1;

	// Create the label asking for the right side crop amount
	bottom_label = gtk_label_new(_("Bottom crop: "));
	gtk_misc_set_alignment(GTK_MISC(bottom_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(crop_table), GTK_WIDGET(bottom_label), 0, 1, row_counter, row_counter + 1, 0, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);

	// Create the slider that accepts the bottom crop amount
	bottom_slider = gtk_hscale_new_with_range(0, valid_fields[LAYER_HEIGHT].max_value, 1);
	gtk_range_set_value(GTK_RANGE(bottom_slider), 0);
	for (scale_mark_counter = 1024; scale_mark_counter <= valid_fields[LAYER_HEIGHT].max_value; scale_mark_counter += 1024)
	{
		// Add scale marks
		gtk_scale_add_mark(GTK_SCALE(bottom_slider), scale_mark_counter, GTK_POS_BOTTOM, NULL);
	}
	gtk_table_attach(GTK_TABLE(crop_table), GTK_WIDGET(bottom_slider), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);

	// Create the bottom crop "pixels" string
	bottom_pixels_label = gtk_label_new(_("pixels"));
	gtk_misc_set_alignment(GTK_MISC(bottom_pixels_label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(crop_table), GTK_WIDGET(bottom_pixels_label), 2, 3, row_counter, row_counter + 1, 0, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	row_counter = row_counter + 1;

	// Make the dialog wider than it's defaults, so the slider marks are useful rather than annoying
	gtk_window_get_size(GTK_WINDOW(crop_dialog), &dialog_width, &dialog_height);
	gtk_window_resize(GTK_WINDOW(crop_dialog), (gint) roundf(dialog_width * 2.0), dialog_height);

	// Run the dialog
	gtk_widget_show_all(GTK_WIDGET(crop_dialog));
	while (FALSE == acceptable_result)
	{
		dialog_result = gtk_dialog_run(GTK_DIALOG(crop_dialog));

		// Was the OK button pressed?
		if (GTK_RESPONSE_ACCEPT != dialog_result)
		{
			// The dialog was cancelled, so destroy the dialog box and return
			gtk_widget_destroy(GTK_WIDGET(crop_dialog));
			return;
		}

		// Determine the new dimensions of the image
		new_height = gdk_pixbuf_get_height(tmp_image_ob->image_data)
			- (gint) gtk_range_get_value(GTK_RANGE(top_slider))
			- (gint) gtk_range_get_value(GTK_RANGE(bottom_slider));
		new_width = gdk_pixbuf_get_width(tmp_image_ob->image_data)
			- (gint) gtk_range_get_value(GTK_RANGE(left_slider))
			- (gint) gtk_range_get_value(GTK_RANGE(right_slider));

		// If the user is cropping too much, warn them and loop again
		if ((0 < new_height) && (0 < new_width))
		{
			acceptable_result = TRUE;
		} else
		{
			g_string_printf(message, "%s ED447: %s\n\n%s", _("Error"), _("Those crop values will remove the entire image."), _("Please try again."));
			display_warning(message->str);
		}
	}

	// Create a new pixbuf, for storing the cropped image in
	new_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, new_width, new_height);

	// Copy the cropped image data to it
	old_pixbuf = tmp_image_ob->image_data;
	gdk_pixbuf_copy_area(old_pixbuf, // Source pixbuf
		(gint) gtk_range_get_value(GTK_RANGE(left_slider)),  // Left crop
		(gint) gtk_range_get_value(GTK_RANGE(top_slider)),  // Top crop
		new_width,  // Width
		new_height,  // Height
		new_pixbuf,  // The newly created pixbuf as the destination
		0, 0);

	// Update the layer with the new cropped data
	tmp_image_ob->image_data = new_pixbuf;
	tmp_image_ob->width = new_width;
	tmp_image_ob->height = new_height;
	tmp_image_ob->modified = TRUE;

	// Create a cairo pattern from the image data
	tmp_image_ob->cairo_pattern = create_cairo_pixbuf_pattern(tmp_image_ob->image_data);
	if (NULL == tmp_image_ob->cairo_pattern)
	{
		// Something went wrong when creating the image pattern
		g_string_printf(message, "%s ED374: %s", _("Error"), _("Couldn't create an image pattern."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		return;
	}

	// Free the memory used by the old pixbuf
	g_object_unref(GDK_PIXBUF(old_pixbuf));

	// Destroy the dialog box
	gtk_widget_destroy(GTK_WIDGET(crop_dialog));

	// Redraw the timeline
	draw_timeline();

	// Redraw the workspace
	draw_workspace();

	// Recreate the slide thumbnail
	film_strip_create_thumbnail(get_current_slide_data());

	// Set the changes made variable
	set_changes_made(TRUE);

	// Update the status bar
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), _(" Image cropped"));
	gdk_flush();

	// Free the memory used in this function
	g_string_free(message, TRUE);
}
