/*
 * $Id$
 *
 * Flame Project: Display a dialog box asking for image layer settings 
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


gboolean display_dialog_image(layer *tmp_layer, gchar *dialog_title, gboolean request_file)
{
	// Local variables
	GtkDialog			*image_dialog;				// Widget for the dialog
	GtkWidget			*image_table;				// Table used for neat layout of the dialog box
	gint				dialog_result;				// Catches the return code from the dialog box
	guint				row_counter = 0;			// Used to count which row things are up to

	GtkWidget			*path_widget;				// File selection widget
	GString				*path_gstring;				// Holds the file selection path

	GtkWidget			*image_label;				// Label widget
	GtkWidget			*image_entry;				//

	GtkWidget			*x_off_label_start;			// Label widget
	GtkWidget			*x_off_button_start;		//

	GtkWidget			*y_off_label_start;			// Label widget
	GtkWidget			*y_off_button_start;		//

	GtkWidget			*x_off_label_finish;		// Label widget
	GtkWidget			*x_off_button_finish;		//

	GtkWidget			*y_off_label_finish;		// Label widget
	GtkWidget			*y_off_button_finish;		//

	GtkWidget			*start_label;				// Label widget
	GtkWidget			*start_button;				//

	GtkWidget			*finish_label;				// Label widget
	GtkWidget			*finish_button;				//

	GtkWidget			*external_link_label;		// Label widget
	GtkWidget			*external_link_entry;		// Widget for accepting an external link for clicking on

	GtkWidget			*external_link_win_label;	// Label widget
	GtkWidget			*external_link_win_entry;	//

	layer_image			*tmp_image_ob;				// Temporary layer object


	// Initialise some things
	path_gstring = g_string_new(NULL);
	tmp_image_ob = (layer_image *) tmp_layer->object_data;

	// If we're supposed to ask for a file, then do so
	if (TRUE == request_file)
	{
		// * Pop open a dialog allowing the user to select an image *
		path_widget = gtk_file_selection_new("Choose an image file");
		g_string_printf(path_gstring, "%s/", screenshots_folder->str);  //  Add a trailing slash to the folder name
		gtk_file_selection_set_filename(GTK_FILE_SELECTION(path_widget), path_gstring->str);

		// Run the dialog
		gtk_widget_show_all(GTK_WIDGET(path_widget));
		dialog_result = gtk_dialog_run(GTK_DIALOG(path_widget));

		// Was the OK button pressed?
		if (GTK_RESPONSE_OK != dialog_result)  // Note this is different to GTK_RESPONSE_ACCEPT used normally
		{
			// * The user cancelled the dialog *

			// Destroy the dialog box
			gtk_widget_destroy(GTK_WIDGET(path_widget));

			// Free the memory allocated in this function
			g_string_free(path_gstring, TRUE);
			return FALSE;	
		}

		// Retrieve the selected image file path
		g_string_printf(path_gstring, "%s", gtk_file_selection_get_filename(GTK_FILE_SELECTION(path_widget)));

		// Destroy the file selection dialog box
		gtk_widget_destroy(GTK_WIDGET(path_widget));
	} else
	{
		// We've been instructed to not request a file, which only happens if we've already got one
		path_gstring = tmp_image_ob->image_path;
	}

	// * Pop open a dialog box asking the user for the details of the layer *

	// Create the dialog window, and table to hold its children
	image_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(dialog_title, GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL));
	image_table = gtk_table_new(3, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(image_dialog->vbox), GTK_WIDGET(image_table), FALSE, FALSE, 10);

	// Imported images don't have a filesystem path, so check for this
	if (0 < path_gstring->len)
	{
		// * We have a path, it's a regular image *

		// Create the label for the path to the image
		image_label = gtk_label_new("Image file:");
		gtk_misc_set_alignment(GTK_MISC(image_label), 0, 0.5);
		gtk_table_attach(GTK_TABLE(image_table), GTK_WIDGET(image_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

		// Create the entry confirming the path to the image
		image_entry = gtk_entry_new();
		gtk_entry_set_max_length(GTK_ENTRY(image_entry), 199);
		gtk_entry_set_text(GTK_ENTRY(image_entry), path_gstring->str);
		gtk_table_attach(GTK_TABLE(image_table), GTK_WIDGET(image_entry), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
		row_counter = row_counter + 1;
	}

	// Create the label asking for the starting X Offset
	x_off_label_start = gtk_label_new("Start X Offset: ");
	gtk_misc_set_alignment(GTK_MISC(x_off_label_start), 0, 0.5);
	gtk_table_attach(GTK_TABLE(image_table), GTK_WIDGET(x_off_label_start), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the starting X Offset input
	x_off_button_start = gtk_spin_button_new_with_range(0, project_width, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(x_off_button_start), tmp_image_ob->x_offset_start);
	gtk_table_attach(GTK_TABLE(image_table), GTK_WIDGET(x_off_button_start), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the starting Y Offset
	y_off_label_start = gtk_label_new("Start Y Offset: ");
	gtk_misc_set_alignment(GTK_MISC(y_off_label_start), 0, 0.5);
	gtk_table_attach(GTK_TABLE(image_table), GTK_WIDGET(y_off_label_start), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the starting Y Offset input
	y_off_button_start = gtk_spin_button_new_with_range(0, project_height, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(y_off_button_start), tmp_image_ob->y_offset_start);
	gtk_table_attach(GTK_TABLE(image_table), GTK_WIDGET(y_off_button_start), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the finishing X Offset
	x_off_label_finish = gtk_label_new("Finish X Offset: ");
	gtk_misc_set_alignment(GTK_MISC(x_off_label_finish), 0, 0.5);
	gtk_table_attach(GTK_TABLE(image_table), GTK_WIDGET(x_off_label_finish), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the finishing X Offset input
	x_off_button_finish = gtk_spin_button_new_with_range(0, project_width, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(x_off_button_finish), tmp_image_ob->x_offset_finish);
	gtk_table_attach(GTK_TABLE(image_table), GTK_WIDGET(x_off_button_finish), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the finishing Y Offset
	y_off_label_finish = gtk_label_new("Finish Y Offset: ");
	gtk_misc_set_alignment(GTK_MISC(y_off_label_finish), 0, 0.5);
	gtk_table_attach(GTK_TABLE(image_table), GTK_WIDGET(y_off_label_finish), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the finishing Y Offset input
	y_off_button_finish = gtk_spin_button_new_with_range(0, project_height, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(y_off_button_finish), tmp_image_ob->y_offset_finish);
	gtk_table_attach(GTK_TABLE(image_table), GTK_WIDGET(y_off_button_finish), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the starting frame
	start_label = gtk_label_new("Start frame: ");
	gtk_misc_set_alignment(GTK_MISC(start_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(image_table), GTK_WIDGET(start_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the starting frame input
	start_button = gtk_spin_button_new_with_range(0, 200, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(start_button), tmp_layer->start_frame);
	gtk_table_attach(GTK_TABLE(image_table), GTK_WIDGET(start_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the finishing frame
	finish_label = gtk_label_new("Finish frame: ");
	gtk_misc_set_alignment(GTK_MISC(finish_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(image_table), GTK_WIDGET(finish_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the finishing frame input
	finish_button = gtk_spin_button_new_with_range(0, 1000, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(finish_button), tmp_layer->finish_frame);
	gtk_table_attach(GTK_TABLE(image_table), GTK_WIDGET(finish_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for an external link
	external_link_label = gtk_label_new("External link: ");
	gtk_misc_set_alignment(GTK_MISC(external_link_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(image_table), GTK_WIDGET(external_link_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts an external link
	external_link_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(external_link_entry), 50);
	gtk_entry_set_text(GTK_ENTRY(external_link_entry), tmp_layer->external_link->str);
	gtk_table_attach(GTK_TABLE(image_table), GTK_WIDGET(external_link_entry), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the window to open the external link in
	external_link_win_label = gtk_label_new("External link window: ");
	gtk_misc_set_alignment(GTK_MISC(external_link_win_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(image_table), GTK_WIDGET(external_link_win_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts a text string for the window to open the external link in
	external_link_win_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(external_link_win_entry), 50);
	gtk_entry_set_text(GTK_ENTRY(external_link_win_entry), tmp_layer->external_link_window->str);
	gtk_table_attach(GTK_TABLE(image_table), GTK_WIDGET(external_link_win_entry), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Run the dialog
	gtk_widget_show_all(GTK_WIDGET(image_dialog));
	dialog_result = gtk_dialog_run(GTK_DIALOG(image_dialog));

	// Was the OK button pressed?
	if (GTK_RESPONSE_ACCEPT != dialog_result)
	{
		// * The user cancelled the dialog *

		// Destroy the dialog box and return
		gtk_widget_destroy(GTK_WIDGET(image_dialog));
		return FALSE;	
	}

	// fixme4: We should validate the user input here

	// Fill out the temporary layer with the requested details
	tmp_image_ob->x_offset_start = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(x_off_button_start));
	tmp_image_ob->y_offset_start = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(y_off_button_start));
	tmp_image_ob->x_offset_finish = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(x_off_button_finish));
	tmp_image_ob->y_offset_finish = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(y_off_button_finish));
	tmp_layer->start_frame = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(start_button));
	tmp_layer->finish_frame = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(finish_button));
	g_string_printf(tmp_layer->external_link, "%s", gtk_entry_get_text(GTK_ENTRY(external_link_entry)));
	g_string_printf(tmp_layer->external_link_window, "%s", gtk_entry_get_text(GTK_ENTRY(external_link_win_entry)));

	// Imported images don't have a filesystem path, so check for this
	if (0 < path_gstring->len)
	{
		// * Not an imported image, so get the path information from the widget *
		g_string_printf(path_gstring, "%s", gtk_entry_get_text(GTK_ENTRY(image_entry)));
		tmp_image_ob->image_path = path_gstring;

		// If we already have image data loaded, get rid of it
		if (TRUE != request_file)
		{
			g_object_unref(GDK_PIXBUF(tmp_image_ob->image_data));
		}

		// Load the new image and get its dimensions
		tmp_image_ob->image_data = gdk_pixbuf_new_from_file(tmp_image_ob->image_path->str, NULL);
		tmp_image_ob->width = gdk_pixbuf_get_width(tmp_image_ob->image_data);
		tmp_image_ob->height = gdk_pixbuf_get_height(tmp_image_ob->image_data);
	}

	// Destroy the dialog box
	gtk_widget_destroy(GTK_WIDGET(image_dialog));

	return TRUE;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.5  2008/02/04 14:29:21  vapour
 * Improved spacing between table cells.
 *
 * Revision 1.4  2008/02/01 10:35:10  vapour
 * Added code to the dialog to display and accept values for the external link's target window.
 *
 * Revision 1.3  2008/01/21 11:52:14  vapour
 * Updated to handle images without a path set, like new imported images, or images that were included in a .flame file.
 *
 * Revision 1.2  2008/01/15 16:19:00  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.1  2007/09/29 04:22:14  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */
