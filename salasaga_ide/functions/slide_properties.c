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

// Standard includes
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

// GTK includes
#include <glib/gstdio.h>
#include <gtk/gtk.h>

// Gnome includes
#include <libgnome/gnome-url.h>

// XML includes
#include <libxml/xmlsave.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"
#include "regenerate_timeline_duration_images.h"


void slide_properties(void)
{
	// Local variables
	gint				dialog_result;				// Catches the return code from the dialog box
	GtkWidget			*dialog_table;				// Table used for neat layout of the dialog box
	guint				layer_counter;				// Counter used in loops
	guint				new_slide_duration;			// Receives the new slide duration value
	guint				num_layers;					// Receives the number of layers present in the slide
	guint				old_slide_duration;			// Holds the old slide duration
	guint				row_counter = 0;			// Used to count which row things are up to
	GtkDialog			*slide_dialog;				// Widget for the dialog
	layer				*this_layer_data;			// Pointer to individual layer data

	GtkWidget			*name_label;				// Label widget
	GtkWidget			*name_entry;				// Widget for accepting the name of the slide

	GtkWidget			*duration_label;			// Label widget
	GtkWidget			*duration_entry;			// Widget for accepting the duration of the slide

	GString				*tmp_gstring;				// Temporary GString


	// Initialise various things
	tmp_gstring = g_string_new(NULL);
	old_slide_duration = ((slide *) current_slide->data)->duration;

	// * Display a dialog box asking for the new name of the slide *

	// Create the dialog window, and table to hold its children
	slide_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons("Edit Slide properties", GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL));
	dialog_table = gtk_table_new(8, 2, FALSE);
	gtk_box_pack_start(GTK_BOX(slide_dialog->vbox), GTK_WIDGET(dialog_table), FALSE, FALSE, 0);

	// Create the label for the slide name
	name_label = gtk_label_new("Slide name: ");
	gtk_misc_set_alignment(GTK_MISC(name_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(dialog_table), GTK_WIDGET(name_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry holding the slide name
	name_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(name_entry), 50);
	if (NULL != ((slide *) current_slide->data)->name)
	{
		// Display the present name in the text field
		gtk_entry_set_text(GTK_ENTRY(name_entry), ((slide *) current_slide->data)->name->str);
	} else
	{
		// No present name, so just use the slide's position
		g_string_printf(tmp_gstring, "Slide %u", g_list_position(slides, current_slide) + 1);
		gtk_entry_set_text(GTK_ENTRY(name_entry), tmp_gstring->str);
	}
	gtk_table_attach_defaults(GTK_TABLE(dialog_table), GTK_WIDGET(name_entry), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label for the slide duration
	duration_label = gtk_label_new("Slide duration: ");
	gtk_misc_set_alignment(GTK_MISC(duration_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(dialog_table), GTK_WIDGET(duration_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry holding the slide duration
	duration_entry = gtk_spin_button_new_with_range(0, 1000, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(duration_entry), old_slide_duration);
	gtk_table_attach_defaults(GTK_TABLE(dialog_table), GTK_WIDGET(duration_entry), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Run the dialog
	gtk_widget_show_all(GTK_WIDGET(slide_dialog));
	dialog_result = gtk_dialog_run(GTK_DIALOG(slide_dialog));

	// Was the OK button pressed?
	if (GTK_RESPONSE_ACCEPT != dialog_result)
	{
		// * The user cancelled the dialog *

		// Destroy the dialog box then return
		gtk_widget_destroy(GTK_WIDGET(slide_dialog));

		// Free the resources allocated in this function
		g_string_free(tmp_gstring, TRUE);

		return;
	}

	// fixme4: We should validate the user input (slide name) here

	// Set the new slide name
	if (NULL == ((slide *) current_slide->data)->name)
	{
		((slide *) current_slide->data)->name = g_string_new(NULL);
	}
	g_string_printf(((slide *) current_slide->data)->name, "%s", gtk_entry_get_text(GTK_ENTRY(name_entry)));

	// Set the new slide duration
	((slide *) current_slide->data)->duration = new_slide_duration = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(duration_entry));
	if (new_slide_duration < old_slide_duration)
	{
		// New slide duration is shorter than it was, so we adjust any layers that would go past the new end
		num_layers = g_list_length(((slide *) current_slide->data)->layers);
		for (layer_counter = 0; layer_counter < num_layers; layer_counter++)
		{
			// Does this layer last too long?
			this_layer_data = g_list_nth_data(((slide *) current_slide->data)->layers, layer_counter);
			if (this_layer_data->finish_frame > new_slide_duration)
			{
				// This layer now lasts too long, so we shorten it's finish frame to match the new slide duration
				this_layer_data->finish_frame = new_slide_duration;
			}
		}
	}

	// Regenerate the timeline duration images for all layers in this slide
	regenerate_timeline_duration_images((slide *) current_slide->data);

	// Destroy the dialog box
	gtk_widget_destroy(GTK_WIDGET(slide_dialog));

	// Free the resources allocated in this function
	g_string_free(tmp_gstring, TRUE);
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.1  2008/02/03 02:07:28  vapour
 * Added initial working code for a Slide Properties dialog.
 *
 */
