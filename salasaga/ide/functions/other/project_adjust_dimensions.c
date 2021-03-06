/*
 * $Id$
 *
 * Salasaga: Function called when the user chooses Project -> Adjust Dimensions from the top menu
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


// Turn on C99 compatibility - needed for roundf() to work
#define _ISOC99_SOURCE

// Math include
#include <math.h>

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
#include "../preference/project_preferences.h"
#include "../working_area/draw_workspace.h"
#include "../zoom_selector/zoom_selector_changed.h"


void project_adjust_dimensions(void)
{
	// Local variables
	gboolean			acceptable_result = FALSE;	// Toggle to determine if we've received valid adjustment values from the user
	GtkDialog			*adjustment_dialog;			// Widget for the dialog
	GtkWidget			*adjustment_table;			// Table used for neat layout of the dialog box
	gint				dialog_height;				// Height of the dialog in pixels
	gint				dialog_result;				// Catches the return code from the dialog box
	gint				dialog_width;				// Width of the dialog in pixels
	layer				*last_layer = NULL;			// Temporary layer
	GList				*layer_pointer;				// Points to the layers in the selected slide
	GString				*message;					// Used to construct message strings
	gint				new_height;					// Hold the height of the adjusted area
	GdkPixbuf			*new_pixbuf;				// Holds the adjusted image data
	gint				new_width;					// Hold the width of the adjusted area
	gint				num_slides;					// Total number of layers
	guint				present_slide_num;			// Holds the number of the currently selected slide in the user interface
	guint				row_counter = 0;			// Used to count which row things are up to
	gdouble				scale_mark_counter;			// Simple counter used when constructing scale marks for sliders
	gint				slide_counter;
	slide				*slide_data;
	gint				table_padding_x;			// Amount of padding to use in the table
	gint				table_padding_y;			// Amount of padding to use in the table
	layer_image			*tmp_image_ob;				// Points to the image data in the selected layer

	GtkWidget			*new_width_label;			// Label widget
	GtkWidget			*new_width_slider;			//
	GtkWidget			*new_width_pixels_label;	// Label widget

	GtkWidget			*new_height_label;			// Label widget
	GtkWidget			*new_height_slider;			//
	GtkWidget			*new_height_pixels_label;	// Label widget


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

	// * Pop open a dialog box asking the user how much to adjust the dimensions of the project by *

	// Create the dialog window, and table to hold its children
	adjustment_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(_("Adjust project dimensions"), GTK_WINDOW(get_main_window()), GTK_DIALOG_MODAL, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL));
	adjustment_table = gtk_table_new(4, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(adjustment_dialog->vbox), GTK_WIDGET(adjustment_table), FALSE, FALSE, 10);

	// Create the label asking for the new project width
	new_width_label = gtk_label_new(_("Project width:"));
	gtk_misc_set_alignment(GTK_MISC(new_width_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(adjustment_table), GTK_WIDGET(new_width_label), 0, 1, row_counter, row_counter + 1, 0, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);

	// Create the slider that accepts the new project width
	new_width_slider = gtk_hscale_new_with_range(get_valid_fields_min_value(PROJECT_WIDTH), get_valid_fields_max_value(PROJECT_WIDTH), 1);
	gtk_range_set_value(GTK_RANGE(new_width_slider), get_project_width());
	gtk_scale_add_mark(GTK_SCALE(new_width_slider), get_project_width(), GTK_POS_TOP, NULL);
	for (scale_mark_counter = 1024; scale_mark_counter <= get_valid_fields_max_value(PROJECT_WIDTH); scale_mark_counter += 1024)
	{
		// Add scale marks
		gtk_scale_add_mark(GTK_SCALE(new_width_slider), scale_mark_counter, GTK_POS_BOTTOM, NULL);
	}
	gtk_table_attach(GTK_TABLE(adjustment_table), GTK_WIDGET(new_width_slider), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);

	// Create the new project width "pixels" string
	new_width_pixels_label = gtk_label_new(_("pixels"));
	gtk_misc_set_alignment(GTK_MISC(new_width_pixels_label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(adjustment_table), GTK_WIDGET(new_width_pixels_label), 2, 3, row_counter, row_counter + 1, 0, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	row_counter = row_counter + 1;

	// Create the label asking for the new project height
	new_height_label = gtk_label_new(_("Project height:"));
	gtk_misc_set_alignment(GTK_MISC(new_height_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(adjustment_table), GTK_WIDGET(new_height_label), 0, 1, row_counter, row_counter + 1, 0, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);

	// Create the slider that accepts the new project height
	new_height_slider = gtk_hscale_new_with_range(get_valid_fields_min_value(PROJECT_HEIGHT), get_valid_fields_max_value(PROJECT_HEIGHT), 1);
	gtk_range_set_value(GTK_RANGE(new_height_slider), get_project_height());
	gtk_scale_add_mark(GTK_SCALE(new_height_slider), get_project_height(), GTK_POS_TOP, NULL);
	for (scale_mark_counter = 1024; scale_mark_counter <= get_valid_fields_max_value(PROJECT_HEIGHT); scale_mark_counter += 1024)
	{
		// Add scale marks
		gtk_scale_add_mark(GTK_SCALE(new_height_slider), scale_mark_counter, GTK_POS_BOTTOM, NULL);
	}
	gtk_table_attach(GTK_TABLE(adjustment_table), GTK_WIDGET(new_height_slider), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);

	// Create the new project height "pixels" string
	new_height_pixels_label = gtk_label_new(_("pixels"));
	gtk_misc_set_alignment(GTK_MISC(new_height_pixels_label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(adjustment_table), GTK_WIDGET(new_height_pixels_label), 2, 3, row_counter, row_counter + 1, 0, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	row_counter = row_counter + 1;

	// Make the dialog wider than it's defaults, so the slider marks are useful rather than annoying
	gtk_window_get_size(GTK_WINDOW(adjustment_dialog), &dialog_width, &dialog_height);
	gtk_window_resize(GTK_WINDOW(adjustment_dialog), (gint) roundf(dialog_width * 2), dialog_height);

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
		new_width = (gint) gtk_range_get_value(GTK_RANGE(new_width_slider));
		new_height = (gint) gtk_range_get_value(GTK_RANGE(new_height_slider));

		// If the user is adjusting the size out of acceptable limits, warn them and loop again
		if ((get_valid_fields_min_value(PROJECT_HEIGHT) <= new_height) && (get_valid_fields_max_value(PROJECT_HEIGHT) >= new_height) && (get_valid_fields_min_value(PROJECT_WIDTH) <= new_width) && (get_valid_fields_max_value(PROJECT_WIDTH) >= new_width))
		{
			acceptable_result = TRUE;
		} else
		{
			if ((get_valid_fields_min_value(PROJECT_HEIGHT) > new_height) || (get_valid_fields_min_value(PROJECT_WIDTH) > new_width))
			{
				g_string_printf(message, "%s ED448: %s\n\n%s", _("Error"), _("Those adjustment values would make the project smaller than the minimum allowed."), _("Please try again."));
				display_warning(message->str);
			} else
			{
				g_string_printf(message, "%s ED449: %s\n\n%s", _("Error"), _("Those adjustment values would make the project larger than the maximum allowed."), _("Please try again."));
				display_warning(message->str);
			}
		}
	}

	// Destroy the dialog box
	gtk_widget_destroy(GTK_WIDGET(adjustment_dialog));

	// Get the present slide, so we can select it again later
	present_slide_num = g_list_position(get_slides(), get_current_slide());

	// Loop through the slide structure, adjusting the backgrounds
	set_slides(g_list_first(get_slides()));
	num_slides = g_list_length(get_slides());
	for (slide_counter = 0; slide_counter < num_slides; slide_counter++)
	{
		set_slides(g_list_first(get_slides()));
		set_current_slide(g_list_nth(get_slides(), slide_counter));
		slide_data = get_current_slide_data();
		layer_pointer = slide_data->layers;
		layer_pointer = g_list_last(layer_pointer);

		// * Check if this slide has a background image or if it's an empty layer *
		last_layer = layer_pointer->data;

		// Is this layer an empty layer?
		if (TYPE_EMPTY != last_layer->object_type)
		{
			// Yes, it's an empty layer, so we don't need to change anything
			continue;
		}

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

		// * This layer is a background image layer *

		// Scale the existing background image layer to the new dimensions
		tmp_image_ob = (layer_image *) last_layer->object_data;
		new_pixbuf = gdk_pixbuf_scale_simple(GDK_PIXBUF(tmp_image_ob->image_data), new_width, new_height, GDK_INTERP_TILES);

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
			g_string_printf(message, "%s ED453: %s", _("Error"), _("Couldn't create an image pattern."));
			display_warning(message->str);
			return;
		}

		// Redraw the workspace
		draw_workspace();

		// Show movement on the progress bar
		gtk_progress_bar_pulse(GTK_PROGRESS_BAR(get_status_bar()));
		gdk_flush();
	}

	// Free the memory used in this function
	g_string_free(message, TRUE);

	// Update project width and height global variables
	set_project_height(new_height);
	set_project_width(new_width);

	// Select the appropriate slide again
	set_slides(g_list_first(get_slides()));
	set_current_slide(g_list_nth(get_slides(), present_slide_num));

	// Recalculate the size of the working area
	zoom_selector_changed(GTK_WIDGET(get_zoom_selector()), NULL, (gpointer) NULL);

	// Recreate the film strip thumbnails
	regenerate_film_strip_thumbnails();

	// Set the changes made variable
	set_changes_made(TRUE);

	// Update the status bar
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), _("Project dimensions adjusted"));
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(get_status_bar()), 0.0);
	gdk_flush();
}
