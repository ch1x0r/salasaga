/*
 * $Id$
 *
 * Flame Project: Display a dialog box asking for highlight layer settings 
 * 
 * Copyright (C) 2007 Justin Clift <justin@postgresql.org>
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
	#include "flame-keycapture.h"
#endif

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"


gboolean display_dialog_highlight(layer *tmp_layer, gchar *dialog_title)
{
	// Local variables
	GtkDialog			*highlight_dialog;			// Widget for the dialog
	GtkWidget			*highlight_table;			// Table used for neat layout of the dialog box
	gint				dialog_result;				// Catches the return code from the dialog box
	guint				row_counter = 0;			// Used to count which row things are up to

	GtkWidget			*x_off_label_start;			// Label widget
	GtkWidget			*x_off_button_start;			//

	GtkWidget			*y_off_label_start;			// Label widget
	GtkWidget			*y_off_button_start;			//

	GtkWidget			*x_off_label_finish;			// Label widget
	GtkWidget			*x_off_button_finish;			//

	GtkWidget			*y_off_label_finish;			// Label widget
	GtkWidget			*y_off_button_finish;			//

	GtkWidget			*width_label;				// Label widget
	GtkWidget			*width_button;				//

	GtkWidget			*height_label;				// Label widget
	GtkWidget			*height_button;				//

	GtkWidget			*start_label;				// Label widget
	GtkWidget			*start_button;				//

	GtkWidget			*finish_label;				// Label widget
	GtkWidget			*finish_button;				//

	GtkWidget			*external_link_label;			// Label widget
	GtkWidget			*external_link_entry;			// Widget for accepting an external link for clicking on

	layer_highlight			*tmp_highlight_ob;			// Temporary highlight layer object


	// Initialise some things
	tmp_highlight_ob = (layer_highlight *) tmp_layer->object_data;

	// * Pop open a dialog box asking the user for the details of the layer *

	// Create the dialog window, and table to hold its children
	highlight_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(dialog_title, GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL));
	highlight_table = gtk_table_new(3, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(highlight_dialog->vbox), GTK_WIDGET(highlight_table), FALSE, FALSE, 10);

	// Create the label asking for the starting X Offset
	x_off_label_start = gtk_label_new("Starting X Offset:");
	gtk_misc_set_alignment(GTK_MISC(x_off_label_start), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(x_off_label_start), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the starting X Offset input
	x_off_button_start = gtk_spin_button_new_with_range(0, project_width, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(x_off_button_start), tmp_highlight_ob->x_offset_start);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(x_off_button_start), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the starting Y Offset
	y_off_label_start = gtk_label_new("Starting Y Offset:");
	gtk_misc_set_alignment(GTK_MISC(y_off_label_start), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(y_off_label_start), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the starting Y Offset input
	y_off_button_start = gtk_spin_button_new_with_range(0, project_height, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(y_off_button_start), tmp_highlight_ob->y_offset_start);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(y_off_button_start), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the finishing X Offset
	x_off_label_finish = gtk_label_new("Finishing X Offset:");
	gtk_misc_set_alignment(GTK_MISC(x_off_label_finish), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(x_off_label_finish), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the finishing X Offset input
	x_off_button_finish = gtk_spin_button_new_with_range(0, project_width, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(x_off_button_finish), tmp_highlight_ob->x_offset_finish);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(x_off_button_finish), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the finishing Y Offset
	y_off_label_finish = gtk_label_new("Finishing Y Offset:");
	gtk_misc_set_alignment(GTK_MISC(y_off_label_finish), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(y_off_label_finish), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the finishing Y Offset input
	y_off_button_finish = gtk_spin_button_new_with_range(0, project_height, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(y_off_button_finish), tmp_highlight_ob->y_offset_finish);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(y_off_button_finish), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the width
	width_label = gtk_label_new("Width:");
	gtk_misc_set_alignment(GTK_MISC(width_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(width_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the width input
	width_button = gtk_spin_button_new_with_range(0, project_width, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(width_button), tmp_highlight_ob->width);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(width_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the height
	height_label = gtk_label_new("Height:");
	gtk_misc_set_alignment(GTK_MISC(height_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(height_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the height input
	height_button = gtk_spin_button_new_with_range(0, project_height, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(height_button), tmp_highlight_ob->height);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(height_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the starting frame
	start_label = gtk_label_new("Start frame: ");
	gtk_misc_set_alignment(GTK_MISC(start_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(start_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the starting frame input
	start_button = gtk_spin_button_new_with_range(0, 200, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(start_button), tmp_layer->start_frame);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(start_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the finishing frame
	finish_label = gtk_label_new("Finish frame: ");
	gtk_misc_set_alignment(GTK_MISC(finish_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(finish_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the finishing frame input
	finish_button = gtk_spin_button_new_with_range(0, 1000, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(finish_button), tmp_layer->finish_frame);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(finish_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for an external link
	external_link_label = gtk_label_new("External link: ");
	gtk_misc_set_alignment(GTK_MISC(external_link_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(external_link_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts an external link
	external_link_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(external_link_entry), 50);
	gtk_entry_set_text(GTK_ENTRY(external_link_entry), tmp_layer->external_link->str);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(external_link_entry), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Run the dialog
	gtk_widget_show_all(GTK_WIDGET(highlight_dialog));
	dialog_result = gtk_dialog_run(GTK_DIALOG(highlight_dialog));

	// Was the OK button pressed?
	if (GTK_RESPONSE_ACCEPT != dialog_result)
	{
		// * The user cancelled the dialog *

		// Destroy the dialog box and return
		gtk_widget_destroy(GTK_WIDGET(highlight_dialog));
		return FALSE;	
	}

	// Fill out the temporary layer with the requested details
	tmp_highlight_ob->x_offset_start = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(x_off_button_start));
	tmp_highlight_ob->y_offset_start = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(y_off_button_start));
	tmp_highlight_ob->x_offset_finish = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(x_off_button_finish));
	tmp_highlight_ob->y_offset_finish = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(y_off_button_finish));
	tmp_highlight_ob->width = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(width_button));
	tmp_highlight_ob->height = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(height_button));
	tmp_layer->start_frame = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(start_button));
	tmp_layer->finish_frame = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(finish_button));
	g_string_printf(tmp_layer->external_link, "%s", gtk_entry_get_text(GTK_ENTRY(external_link_entry)));

	// Destroy the dialog box
	gtk_widget_destroy(GTK_WIDGET(highlight_dialog));

	return TRUE;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.1  2007/09/29 04:22:13  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */
