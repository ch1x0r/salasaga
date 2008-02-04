/*
 * $Id$
 *
 * Flame Project: Function called when the user selects Project -> Properties from the top menu 
 * 
 * Copyright (C) 2008 Justin Clift <justin@postgresql.org>
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


void menu_project_properties(void)
{
	// Local variables
	gint				dialog_result;				// Catches the return code from the dialog box
	GtkDialog			*main_dialog;				// Widget for the main dialog
	GtkWidget			*proj_dialog_table;			// Table used for neat layout of the labels and fields in project preferences
	gint				proj_row_counter;			// Used when building the project preferences dialog box

	GtkWidget			*label_project_name;		// Project Name
	GtkWidget			*entry_project_name;		//

	GtkWidget			*label_project_folder;		// Project Folder
	GtkWidget			*button_project_folder;		//

	GtkWidget			*label_output_folder;		// Output Folder
	GtkWidget			*button_output_folder;		//

	GtkWidget			*label_output_quality;		// Output Quality
	GtkWidget			*button_output_quality;		//

	GtkWidget			*label_slide_length;		// Slide Length
	GtkWidget			*button_slide_length;		//

	GtkWidget			*label_frames_per_second;	// Slide Length
	GtkWidget			*button_frames_per_second;	//

	GtkWidget			*label_project_width;		// Project width
	GtkWidget			*entry_project_width;		//

	GtkWidget			*label_project_height;		// Project height
	GtkWidget			*entry_project_height;		//

	GString				*tmp_gstring;				// Temporary GString used for constructing text


	// Initialise various things
	proj_row_counter = 0;
	tmp_gstring = g_string_new(NULL);

	// Create the main dialog window
	main_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons("Project Properties", GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL));
	proj_dialog_table = gtk_table_new(10, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(main_dialog->vbox), GTK_WIDGET(proj_dialog_table), FALSE, FALSE, 5);

	// Project Name
	label_project_name = gtk_label_new("Project Name: ");
	gtk_misc_set_alignment(GTK_MISC(label_project_name), 0, 0.5);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(label_project_name), 0, 1, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	entry_project_name = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry_project_name), 50);
	gtk_entry_set_text(GTK_ENTRY(entry_project_name), (gchar *) project_name->str);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(entry_project_name), 2, 3, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	proj_row_counter = proj_row_counter + 1;

	// Project Folder
	label_project_folder = gtk_label_new("Project Folder: ");
	gtk_misc_set_alignment(GTK_MISC(label_project_folder), 0, 0.5);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(label_project_folder), 0, 1, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	button_project_folder = gtk_file_chooser_button_new("Select the Project Folder", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(button_project_folder), project_folder->str);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(button_project_folder), 2, 3, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	proj_row_counter = proj_row_counter + 1;

	// Output Folder
	label_output_folder = gtk_label_new("Output Folder: ");
	gtk_misc_set_alignment(GTK_MISC(label_output_folder), 0, 0.5);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(label_output_folder), 0, 1, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	button_output_folder = gtk_file_chooser_button_new("Select the Output Folder", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(button_output_folder), output_folder->str);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(button_output_folder), 2, 3, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	proj_row_counter = proj_row_counter + 1;

	// Output Quality
	label_output_quality = gtk_label_new("Output Quality: ");
	gtk_misc_set_alignment(GTK_MISC(label_output_quality), 0, 0.5);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(label_output_quality), 0, 1, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	button_output_quality = gtk_spin_button_new_with_range(0, 9, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_output_quality), output_quality);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(button_output_quality), 2, 3, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	proj_row_counter = proj_row_counter + 1;

	// Slide Length
	label_slide_length = gtk_label_new("Default Slide Length: ");
	gtk_misc_set_alignment(GTK_MISC(label_slide_length), 0, 0.5);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(label_slide_length), 0, 1, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	button_slide_length = gtk_spin_button_new_with_range(0, 1000, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_slide_length), slide_length);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(button_slide_length), 2, 3, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	proj_row_counter = proj_row_counter + 1;

	// Frames per second
	label_frames_per_second = gtk_label_new("Frames per second: ");
	gtk_misc_set_alignment(GTK_MISC(label_frames_per_second), 0, 0.5);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(label_frames_per_second), 0, 1, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	button_frames_per_second = gtk_spin_button_new_with_range(0, 48, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_frames_per_second), frames_per_second);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(button_frames_per_second), 2, 3, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	proj_row_counter = proj_row_counter + 1;

	// Project Width
	label_project_width = gtk_label_new("Project Width: ");
	gtk_misc_set_alignment(GTK_MISC(label_project_width), 0, 0.5);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(label_project_width), 0, 1, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	entry_project_width = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry_project_width), 12);
	g_string_printf(tmp_gstring, "%d pixels", project_width);
	gtk_entry_set_text(GTK_ENTRY(entry_project_width), tmp_gstring->str);
	gtk_editable_set_editable(GTK_EDITABLE(entry_project_width), FALSE);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(entry_project_width), 2, 3, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	proj_row_counter = proj_row_counter + 1;

	// Project Height
	label_project_height = gtk_label_new("Project Height: ");
	gtk_misc_set_alignment(GTK_MISC(label_project_height), 0, 0.5);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(label_project_height), 0, 1, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	entry_project_height = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry_project_height), 12);
	g_string_printf(tmp_gstring, "%d pixels", project_height);
	gtk_entry_set_text(GTK_ENTRY(entry_project_height), tmp_gstring->str);
	gtk_editable_set_editable(GTK_EDITABLE(entry_project_height), FALSE);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(entry_project_height), 2, 3, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Set the dialog going
	gtk_widget_show_all(GTK_WIDGET(main_dialog));
	dialog_result = gtk_dialog_run(GTK_DIALOG(main_dialog));

	// Was the OK button pressed?
	if (GTK_RESPONSE_ACCEPT == dialog_result)
	{
		// * Yes, so update the project variables with their new values *

		// Project Name
		project_name = g_string_assign(project_name, gtk_entry_get_text(GTK_ENTRY(entry_project_name)));

		// Project Folder
		project_folder = g_string_assign(project_folder, gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(button_project_folder)));

		// Output Folder
		output_folder = g_string_assign(output_folder, gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(button_output_folder)));

		// Output Quality
		output_quality = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_output_quality));

		// Slide Length
		slide_length = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_slide_length));

		// Frames per second
		frames_per_second = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_frames_per_second));
	}

	// Free up the memory allocated in this function
	// fixme2: Once this function works properly, I should revisit this code to ensure everything is freed properly
	gtk_widget_destroy(GTK_WIDGET(main_dialog));
	g_string_free(tmp_gstring, TRUE);
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.3  2008/02/04 14:38:17  vapour
 *  + Removed unnecessary includes.
 *  + Improved spacing between table cells.
 *
 * Revision 1.2  2008/02/04 13:03:32  vapour
 * Added (non-editable) display of the project width and height.
 *
 * Revision 1.1  2008/02/04 10:30:35  vapour
 * Added new function for Project Properties.
 *
 */
