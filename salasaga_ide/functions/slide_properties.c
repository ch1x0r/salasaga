/*
 * $Id$
 *
 * Salasaga: Function called when the user selects Slide -> Properties from the top menu 
 * 
 * Copyright (C) 2007-2008 Justin Clift <justin@salasaga.org>
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

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"
#include "draw_timeline.h"
#include "display_warning.h"
#include "validate_value.h"


void slide_properties(void)
{
	// Local variables
	GtkWidget			*dialog_table;				// Table used for neat layout of the dialog box
	gfloat				gfloat_val;					// Temporary gfloat value used for validation
	guint				layer_counter;				// Counter used in loops
	guint				num_layers;					// Receives the number of layers present in the slide
	gfloat				old_slide_duration;			// Holds the old slide duration
	guint				row_counter = 0;			// Used to count which row things are up to
	GtkDialog			*slide_dialog;				// Widget for the dialog
	layer				*this_layer_data;			// Pointer to individual layer data
	slide				*this_slide;				// Points to the slide we're working with
	gboolean			useable_input;				// Used as a flag to indicate if all validation was successful
	gfloat				valid_slide_duration;		// Receives the new slide duration once validated
	GString				*valid_slide_name;			// Receives the new slide name once validated
	gfloat				*validated_gfloat;			// Receives known good gfloat values from the validation function
	GString				*validated_string;			// Receives known good strings from the validation function

	GtkWidget			*name_label;				// Label widget
	GtkWidget			*name_entry;				// Widget for accepting the name of the slide

	GtkWidget			*duration_label;			// Label widget
	GtkWidget			*duration_entry;			// Widget for accepting the duration of the slide

	GString				*tmp_gstring;				// Temporary GString


	// Initialise various things
	this_slide = current_slide->data;
	tmp_gstring = g_string_new(NULL);
	valid_slide_name = g_string_new(NULL);
	old_slide_duration = this_slide->duration;

	// * Display a dialog box asking for the new name of the slide *

	// Create the dialog window, and table to hold its children
	slide_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons("Edit Slide properties", GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL));
	dialog_table = gtk_table_new(8, 2, FALSE);
	gtk_box_pack_start(GTK_BOX(slide_dialog->vbox), GTK_WIDGET(dialog_table), FALSE, FALSE, 0);

	// Create the label for the slide name
	name_label = gtk_label_new("Slide name: ");
	gtk_misc_set_alignment(GTK_MISC(name_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(dialog_table), GTK_WIDGET(name_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry holding the slide name
	name_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(name_entry), valid_fields[SLIDE_NAME].max_value);
	if (NULL != this_slide->name)
	{
		// Display the present name in the text field
		gtk_entry_set_text(GTK_ENTRY(name_entry), this_slide->name->str);
	} else
	{
		// No present name, so just use the slide's position
		g_string_printf(tmp_gstring, "Slide %u", g_list_position(slides, current_slide) + 1);
		gtk_entry_set_text(GTK_ENTRY(name_entry), tmp_gstring->str);
	}
	gtk_table_attach(GTK_TABLE(dialog_table), GTK_WIDGET(name_entry), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label for the slide duration
	duration_label = gtk_label_new("Slide duration: ");
	gtk_misc_set_alignment(GTK_MISC(duration_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(dialog_table), GTK_WIDGET(duration_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry holding the slide duration
	duration_entry = gtk_spin_button_new_with_range(valid_fields[SLIDE_DURATION].min_value, valid_fields[SLIDE_DURATION].max_value, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(duration_entry), old_slide_duration);
	gtk_table_attach(GTK_TABLE(dialog_table), GTK_WIDGET(duration_entry), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Ensure everything will show
	gtk_widget_show_all(GTK_WIDGET(slide_dialog));

	// Loop around until we have all valid values, or the user cancels out
	validated_string = NULL;
	do
	{
		// Display the dialog
		if (GTK_RESPONSE_ACCEPT != gtk_dialog_run(GTK_DIALOG(slide_dialog)))
		{
			// The dialog was cancelled, so destroy it and return to the caller
			gtk_widget_destroy(GTK_WIDGET(slide_dialog));
			g_string_free(valid_slide_name, TRUE);
			g_string_free(tmp_gstring, TRUE);
			return;
		}

		// Reset the useable input flag
		useable_input = TRUE;

		// Validate the slide name input
		validated_string = validate_value(SLIDE_NAME, V_CHAR, (gchar *) gtk_entry_get_text(GTK_ENTRY(name_entry)));
		if (NULL == validated_string)
		{
			display_warning("Error ED140: There was something wrong with the slide name value.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_slide_name = g_string_assign(valid_slide_name, validated_string->str);
			g_string_free(validated_string, TRUE);
			validated_string = NULL;
		}

		// Retrieve the new slide duration input
		gfloat_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(duration_entry));
		validated_gfloat = validate_value(SLIDE_DURATION, V_FLOAT_UNSIGNED, &gfloat_val);
		if (NULL == validated_gfloat)
		{
			display_warning("Error ED141: There was something wrong with the slide duration value.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_slide_duration = *validated_gfloat;
			g_free(validated_gfloat);
		}
	} while (FALSE == useable_input);

	// * We only get here after all input is considered valid *

	// Set the new slide name
	if (NULL == this_slide->name)
	{
		this_slide->name = g_string_new(NULL);
	}
	g_string_printf(this_slide->name, "%s", valid_slide_name->str);

	// Set the new slide duration
	this_slide->duration = valid_slide_duration;
	if (valid_slide_duration < old_slide_duration)
	{
		// New slide duration is shorter than it was, so we adjust any layers that now go past the end
		this_slide->layers = g_list_first(this_slide->layers);
		num_layers = this_slide->num_layers;
		for (layer_counter = 0; layer_counter < num_layers; layer_counter++)
		{
			// Does this layer last too long?
			this_layer_data = g_list_nth_data(this_slide->layers, layer_counter);

			// If the layer end time is longer than the new slide duration, so we need to shorten the layer somehow
			if ((this_layer_data->start_time + this_layer_data->duration + this_layer_data->transition_in_duration + this_layer_data->transition_out_duration) > valid_slide_duration)
			{
				if (this_layer_data->start_time >= valid_slide_duration)
				{
					// The layer is completely outside of the slide duration, so reset it to be at the end of the slide
					this_layer_data->start_time = valid_slide_duration - (1 / frames_per_second);
					this_layer_data->duration = 1 / frames_per_second;
				} else
				{
					// The layer start time is in bounds, so we just shorted the duration
					this_layer_data->duration = valid_slide_duration - this_layer_data->start_time;
				}
			}
		}
	}

	// Regenerate the timeline
	draw_timeline();

	// Destroy the dialog box
	gtk_widget_destroy(GTK_WIDGET(slide_dialog));

	// Set the changes made variable
	changes_made = TRUE;

	// Update the status bar
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, " Slide properties updated");
	gdk_flush();

	// Free the resources allocated in this function
	g_string_free(valid_slide_name, TRUE);
	g_string_free(tmp_gstring, TRUE);
}
