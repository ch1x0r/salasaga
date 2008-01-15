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


gboolean display_dialog_empty(layer *tmp_layer, gchar *dialog_title)
{
	// Local variables
	GtkDialog			*empty_dialog;				// Widget for the dialog
	GtkWidget			*empty_table;				// Table used for neat layout of the dialog box
	gint				dialog_result;				// Catches the return code from the dialog box
	guint				row_counter = 0;			// Used to count which row things are up to

	GtkWidget			*label_bg_colour;			// Background colour
	GtkWidget			*button_bg_colour;			// Color button

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
	gtk_table_attach_defaults(GTK_TABLE(empty_table), GTK_WIDGET(label_bg_colour), 0, 1, row_counter, row_counter + 1);
	button_bg_colour = gtk_color_button_new_with_color(&tmp_empty_ob->bg_color);
	gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(button_bg_colour), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(empty_table), GTK_WIDGET(button_bg_colour), 2, 3, row_counter, row_counter + 1);
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

	// Get the new background Colour
	gtk_color_button_get_color(GTK_COLOR_BUTTON(button_bg_colour), &tmp_empty_ob->bg_color);

	// Destroy the dialog box
	gtk_widget_destroy(GTK_WIDGET(empty_dialog));

	return TRUE;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.2  2008/01/15 16:19:00  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.1  2007/09/29 04:22:15  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */
