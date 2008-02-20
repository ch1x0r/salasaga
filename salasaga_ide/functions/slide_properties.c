/*
 * $Id$
 *
 * Flame Project: Function called when the user selects Slide -> Properties from the top menu 
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
#include "display_warning.h"
#include "regenerate_timeline_duration_images.h"
#include "validate_value.h"


void slide_properties(void)
{
	// Local variables
	GtkWidget			*dialog_table;				// Table used for neat layout of the dialog box
	guint				guint_val;					// Temporary guint value used for validation
	guint				layer_counter;				// Counter used in loops
	guint				num_layers;					// Receives the number of layers present in the slide
	guint				old_slide_duration;			// Holds the old slide duration
	guint				row_counter = 0;			// Used to count which row things are up to
	GtkDialog			*slide_dialog;				// Widget for the dialog
	layer				*this_layer_data;			// Pointer to individual layer data
	slide				*this_slide;				// Points to the slide we're working with
	gboolean			useable_input;				// Used as a flag to indicate if all validation was successful
	guint				valid_slide_duration;		// Receives the new slide duration once validated
	GString				*valid_slide_name;			// Receives the new slide name once validated
	guint				*validated_guint;			// Receives known good guint values from the validation function
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
	duration_entry = gtk_spin_button_new_with_range(0, valid_fields[SLIDE_LENGTH].max_value, 10);
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
		guint_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(duration_entry));
		validated_guint = validate_value(SLIDE_LENGTH, V_INT_UNSIGNED, &guint_val);
		if (NULL == validated_guint)
		{
			display_warning("Error ED141: There was something wrong with the slide duration value.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_slide_duration = *validated_guint;
			g_free(validated_guint);
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
		num_layers = g_list_length(this_slide->layers);
		for (layer_counter = 0; layer_counter < num_layers; layer_counter++)
		{
			// Does this layer last too long?
			this_layer_data = g_list_nth_data(this_slide->layers, layer_counter);
			if (this_layer_data->finish_frame > valid_slide_duration)
			{
				// This layer now lasts too long, so we shorten it's finish frame to match the new slide duration
				this_layer_data->finish_frame = valid_slide_duration;
			}
		}
	}

	// Regenerate the timeline duration images for all layers in this slide
	regenerate_timeline_duration_images(this_slide);

	// Destroy the dialog box
	gtk_widget_destroy(GTK_WIDGET(slide_dialog));

	// Free the resources allocated in this function
	g_string_free(valid_slide_name, TRUE);
	g_string_free(tmp_gstring, TRUE);
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.3  2008/02/20 09:06:15  vapour
 *  + Now validates input.
 *  + Simplified pointers.
 *
 * Revision 1.2  2008/02/04 14:41:46  vapour
 *  + Removed unnecessary includes.
 *  + Improved spacing between table cells.
 *
 * Revision 1.1  2008/02/03 02:07:28  vapour
 * Added initial working code for a Slide Properties dialog.
 *
 */
