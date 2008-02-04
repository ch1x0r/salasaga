/*
 * $Id$
 *
 * Flame Project: Display a dialog box asking for empty layer settings 
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


gboolean display_dialog_empty(layer *tmp_layer, gchar *dialog_title)
{
	// Local variables
	GtkDialog			*empty_dialog;				// Widget for the dialog
	GtkWidget			*empty_table;				// Table used for neat layout of the dialog box
	gint				dialog_result;				// Catches the return code from the dialog box
	guint				row_counter = 0;			// Used to count which row things are up to

	GtkWidget			*label_bg_colour;			// Background colour
	GtkWidget			*button_bg_colour;			// Color button

	GtkWidget			*external_link_label;		// Label widget
	GtkWidget			*external_link_entry;		// Widget for accepting an external link for clicking on

	GtkWidget			*external_link_win_label;	// Label widget
	GtkWidget			*external_link_win_entry;	//

	layer_empty			*tmp_empty_ob;				// Temporary empty layer object


	// Initialise some things
	tmp_empty_ob = (layer_empty *) tmp_layer->object_data;

	// * Pop open a dialog box asking the user for the details of the layer *

	// Create the dialog window, and table to hold its children
	empty_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(dialog_title, GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL));
	empty_table = gtk_table_new(3, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(empty_dialog->vbox), GTK_WIDGET(empty_table), FALSE, FALSE, 10);

	// Background Colour
	label_bg_colour = gtk_label_new("Background Colour: ");
	gtk_misc_set_alignment(GTK_MISC(label_bg_colour), 0, 0.5);
	gtk_table_attach(GTK_TABLE(empty_table), GTK_WIDGET(label_bg_colour), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	button_bg_colour = gtk_color_button_new_with_color(&tmp_empty_ob->bg_color);
	gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(button_bg_colour), TRUE);
	gtk_table_attach(GTK_TABLE(empty_table), GTK_WIDGET(button_bg_colour), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for an external link
	external_link_label = gtk_label_new("External link: ");
	gtk_misc_set_alignment(GTK_MISC(external_link_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(empty_table), GTK_WIDGET(external_link_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts an external link
	external_link_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(external_link_entry), 50);
	gtk_entry_set_text(GTK_ENTRY(external_link_entry), tmp_layer->external_link->str);
	gtk_table_attach(GTK_TABLE(empty_table), GTK_WIDGET(external_link_entry), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the window to open the external link in
	external_link_win_label = gtk_label_new("External link window: ");
	gtk_misc_set_alignment(GTK_MISC(external_link_win_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(empty_table), GTK_WIDGET(external_link_win_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts a text string for the window to open the external link in
	external_link_win_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(external_link_win_entry), 50);
	gtk_entry_set_text(GTK_ENTRY(external_link_win_entry), tmp_layer->external_link_window->str);
	gtk_table_attach(GTK_TABLE(empty_table), GTK_WIDGET(external_link_win_entry), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Run the dialog
	gtk_widget_show_all(GTK_WIDGET(empty_dialog));
	dialog_result = gtk_dialog_run(GTK_DIALOG(empty_dialog));

	// Was the OK button pressed?
	if (GTK_RESPONSE_ACCEPT != dialog_result)
	{
		// * The user cancelled the dialog *

		// Destroy the dialog box and return
		gtk_widget_destroy(GTK_WIDGET(empty_dialog));
		return FALSE;	
	}

	// Retrieve the updated values
	gtk_color_button_get_color(GTK_COLOR_BUTTON(button_bg_colour), &tmp_empty_ob->bg_color);
	g_string_printf(tmp_layer->external_link, "%s", gtk_entry_get_text(GTK_ENTRY(external_link_entry)));
	g_string_printf(tmp_layer->external_link_window, "%s", gtk_entry_get_text(GTK_ENTRY(external_link_win_entry)));

	// Destroy the dialog box
	gtk_widget_destroy(GTK_WIDGET(empty_dialog));

	return TRUE;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.4  2008/02/04 14:24:38  vapour
 *  + Removed unnecessary includes.
 *  + Improved spacing between table cells.
 *
 * Revision 1.3  2008/02/01 10:32:50  vapour
 * Added code to the dialog to display and accept values for the external link and its target window.
 *
 * Revision 1.2  2008/01/15 16:19:00  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.1  2007/09/29 04:22:15  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */
