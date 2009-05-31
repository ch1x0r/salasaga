/*
 * $Id$
 *
 * Salasaga: Function called when the user selects Slide -> Duration from the top menu
 *
 * Copyright (C) 2005-2009 Justin Clift <justin@salasaga.org>
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
#include "../../externs.h"
#include "../dialog/display_warning.h"
#include "../widgets/time_line/time_line_set_stored_slide_duration.h"
#include "../draw_timeline.h"
#include "../validate_value.h"


void slide_duration(void)
{
	// Local variables
	GtkWidget			*dialog_table;				// Table used for neat layout of the dialog box
	gfloat				end_time;					// Used to calculate the end time in seconds of a layer
	gfloat				gfloat_val;					// Temporary gfloat value used for validation
	guint				layer_counter;				// Counter used in loops
	GString				*message;					// Used to construct message strings
	guint				num_layers;					// Receives the number of layers present in the slide
	gfloat				old_slide_duration;			// Holds the old slide duration
	gfloat				overall_duration;			// Used when working out the visible time for a layer
	guint				row_counter = 0;			// Used to count which row things are up to
	GtkDialog			*slide_dialog;				// Widget for the dialog
	layer				*this_layer_data = NULL;	// Pointer to individual layer data
	slide				*this_slide;				// Points to the slide we're working with
	gboolean			usable_input;				// Used as a flag to indicate if all validation was successful
	gfloat				valid_slide_duration;		// Receives the new slide duration once validated
	gfloat				*validated_gfloat;			// Receives known good gfloat values from the validation function
	GString				*validated_string;			// Receives known good strings from the validation function

	GtkWidget			*duration_label;			// Label widget
	GtkWidget			*duration_entry;			// Widget for accepting the duration of the slide


	// Initialise various things
	this_slide = current_slide->data;
	old_slide_duration = this_slide->duration;

	// * Display a dialog box asking for the new name of the slide *

	// Create the dialog window, and table to hold its children
	slide_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(_("Edit slide duration"), GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL));
	dialog_table = gtk_table_new(8, 2, FALSE);
	gtk_box_pack_start(GTK_BOX(slide_dialog->vbox), GTK_WIDGET(dialog_table), FALSE, FALSE, 0);

	// Create the label for the slide duration
	duration_label = gtk_label_new(_("Slide duration: "));
	gtk_misc_set_alignment(GTK_MISC(duration_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(dialog_table), GTK_WIDGET(duration_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry holding the slide duration
	duration_entry = gtk_spin_button_new_with_range(valid_fields[SLIDE_DURATION].min_value, valid_fields[SLIDE_DURATION].max_value, 0.1);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(duration_entry), 2);
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
			return;
		}

		// Reset the usable input flag
		usable_input = TRUE;

		// Retrieve the new slide duration input
		gfloat_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(duration_entry));
		validated_gfloat = validate_value(SLIDE_DURATION, V_FLOAT_UNSIGNED, &gfloat_val);
		if (NULL == validated_gfloat)
		{
			message = g_string_new(NULL);
			g_string_printf(message, "%s ED141: %s", _("Error"), _("There was something wrong with the slide duration value.  Please try again."));
			display_warning(message->str);
			g_string_free(message, TRUE);
			usable_input = FALSE;
		} else
		{
			valid_slide_duration = *validated_gfloat;
			g_free(validated_gfloat);
		}
	} while (FALSE == usable_input);

	// * We only get here after all input is considered valid *

	// Set the new slide duration
	this_slide->duration = valid_slide_duration;
	if (valid_slide_duration < old_slide_duration)
	{
		// New slide duration is shorter than it was, so we adjust any layers that now go past the end
		this_slide->layers = g_list_first(this_slide->layers);
		num_layers = this_slide->num_layers;
		for (layer_counter = 0; layer_counter < num_layers; layer_counter++)
		{
			// Update the background layer (note that this always comes last, which is useful)
			this_layer_data = g_list_nth_data(this_slide->layers, layer_counter);
			if (TRUE == this_layer_data->background)
			{
				this_layer_data->duration = valid_slide_duration;
				continue;
			}

			// Work out the end time for the layer
			end_time = this_layer_data->start_time + this_layer_data->duration;
			if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
				end_time += this_layer_data->transition_in_duration;
			if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
				end_time += this_layer_data->transition_out_duration;

			// If the layer end time is longer than the new slide duration, so we need to shorten the layer somehow
			if (end_time > valid_slide_duration)
			{
				// If the layer is small enough, we just move it's start time back a bit
				overall_duration = end_time - this_layer_data->start_time;
				if (overall_duration <= valid_slide_duration)
				{
					this_layer_data->start_time = valid_slide_duration - overall_duration;
				} else
				{
					// No, the layer is still longer than the slide duration.  We move the layer back as far as
					// possible, then extend the slide duration to let it fit
					this_layer_data->start_time = 0;
					this_slide->duration = valid_slide_duration = overall_duration;

					// We also beep for good measure, to let the user know they did something wrong
					gdk_beep();
				}
			}
		}
	}
	if (valid_slide_duration > old_slide_duration)
	{
		// New slide duration is longer than it was, so we adjust the background layer to match
		this_slide->layers = g_list_first(this_slide->layers);
		num_layers = this_slide->num_layers;
		this_layer_data = g_list_nth_data(this_slide->layers, num_layers - 1);

		// Double check that it's a background layer, just to be careful
		if (TRUE == this_layer_data->background)
		{
			this_layer_data->duration = valid_slide_duration;
		}
	}

	// Update the timeline widget with the new slide duration
	time_line_set_stored_slide_duration(this_slide->timeline_widget, this_slide->duration);

	// Regenerate the timeline
	draw_timeline();

	// Destroy the dialog box
	gtk_widget_destroy(GTK_WIDGET(slide_dialog));

	// Set the changes made variable
	changes_made = TRUE;

	// Update the status bar
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(status_bar), _(" Slide properties updated"));
	gdk_flush();
}
