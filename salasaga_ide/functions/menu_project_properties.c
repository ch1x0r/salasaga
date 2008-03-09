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
#include "display_warning.h"
#include "validate_value.h"


void menu_project_properties(void)
{
	// Local variables
	guint				guint_val;					// Temporary guint value used for validation
	gint				gint_val;					// Temporary guint value
	GtkDialog			*main_dialog;				// Widget for the main dialog
	GtkWidget			*proj_dialog_table;			// Table used for neat layout of the labels and fields in project preferences
	gint				proj_row_counter;			// Used when building the project preferences dialog box
	gboolean			useable_input;				// Used as a flag to indicate if all validation was successful
	guint				valid_end_behaviour;		// Receives the new end behaviour once validated
	guint				valid_fps;					// Receives the new project fps once validated
	GString				*valid_output_folder;		// Receives the new output folder once validated
	GString				*valid_proj_name;			// Receives the new project name once validated
	GString				*valid_project_folder;		// Receives the new project folder once validated
	guint				valid_slide_length;			// Receives the new project default slide length once validated
	guint				valid_start_behaviour;		// Receives the new start behaviour once validated
	guint				*validated_guint;			// Receives known good guint values from the validation function
	GString				*validated_string;			// Receives known good strings from the validation function

	GtkWidget			*label_project_name;		// Project Name
	GtkWidget			*entry_project_name;		//

	GtkWidget			*label_project_folder;		// Project Folder
	GtkWidget			*button_project_folder;		//

	GtkWidget			*label_output_folder;		// Output Folder
	GtkWidget			*button_output_folder;		//

	GtkWidget			*label_slide_length;		// Slide Length
	GtkWidget			*button_slide_length;		//

	GtkWidget			*label_frames_per_second;	// Slide Length
	GtkWidget			*button_frames_per_second;	//

	GtkWidget			*label_project_width;		// Project width
	GtkWidget			*entry_project_width;		//

	GtkWidget			*label_project_height;		// Project height
	GtkWidget			*entry_project_height;		//

	GtkWidget			*label_start_behaviour;		// Start behaviour
	GtkWidget			*selector_start_behaviour;	//

	GtkWidget			*label_end_behaviour;		// End behaviour
	GtkWidget			*selector_end_behaviour;	//

	GString				*tmp_gstring;				// Temporary GString used for constructing text


	// Initialise various things
	proj_row_counter = 0;
	valid_output_folder = g_string_new(NULL);
	valid_proj_name = g_string_new(NULL);
	valid_project_folder = g_string_new(NULL);
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
	gtk_entry_set_max_length(GTK_ENTRY(entry_project_name), valid_fields[PROJECT_NAME].max_value);
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

	// Slide Length
	label_slide_length = gtk_label_new("Default Slide Length: ");
	gtk_misc_set_alignment(GTK_MISC(label_slide_length), 0, 0.5);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(label_slide_length), 0, 1, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	button_slide_length = gtk_spin_button_new_with_range(0, valid_fields[SLIDE_LENGTH].max_value, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_slide_length), slide_length);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(button_slide_length), 2, 3, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	proj_row_counter = proj_row_counter + 1;

	// Frames per second
	label_frames_per_second = gtk_label_new("Frames per second: ");
	gtk_misc_set_alignment(GTK_MISC(label_frames_per_second), 0, 0.5);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(label_frames_per_second), 0, 1, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	button_frames_per_second = gtk_spin_button_new_with_range(0, valid_fields[PROJECT_FPS].max_value, 1);
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
	proj_row_counter = proj_row_counter + 1;

	// Start behaviour
	label_start_behaviour = gtk_label_new("Start behaviour: ");
	gtk_misc_set_alignment(GTK_MISC(label_start_behaviour), 0, 0.5);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(label_start_behaviour), 0, 1, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	selector_start_behaviour = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(selector_start_behaviour), "Paused");
	gtk_combo_box_append_text(GTK_COMBO_BOX(selector_start_behaviour), "Play");
	switch (start_behaviour)
	{
		case START_BEHAVIOUR_PLAY:
			gtk_combo_box_set_active(GTK_COMBO_BOX(selector_start_behaviour), START_BEHAVIOUR_PLAY);
			break;

		default:
			gtk_combo_box_set_active(GTK_COMBO_BOX(selector_start_behaviour), START_BEHAVIOUR_PAUSED);
	}
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(selector_start_behaviour), 2, 3, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	proj_row_counter = proj_row_counter + 1;

	// End behaviour
	label_end_behaviour = gtk_label_new("End behaviour: ");
	gtk_misc_set_alignment(GTK_MISC(label_end_behaviour), 0, 0.5);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(label_end_behaviour), 0, 1, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	selector_end_behaviour = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(selector_end_behaviour), "Stop");
	gtk_combo_box_append_text(GTK_COMBO_BOX(selector_end_behaviour), "Loop and play");
	gtk_combo_box_append_text(GTK_COMBO_BOX(selector_end_behaviour), "Loop and stop");
	switch (end_behaviour)
	{
		case END_BEHAVIOUR_LOOP_PLAY:
			gtk_combo_box_set_active(GTK_COMBO_BOX(selector_end_behaviour), END_BEHAVIOUR_LOOP_PLAY);
			break;

		case END_BEHAVIOUR_LOOP_STOP:
			gtk_combo_box_set_active(GTK_COMBO_BOX(selector_end_behaviour), END_BEHAVIOUR_LOOP_STOP);
			break;

		default:
			gtk_combo_box_set_active(GTK_COMBO_BOX(selector_end_behaviour), END_BEHAVIOUR_STOP);
	}
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(selector_end_behaviour), 2, 3, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Ensure everything will show
	gtk_widget_show_all(GTK_WIDGET(main_dialog));

	// Loop around until we have all valid values, or the user cancels out
	validated_string = NULL;
	do
	{
		// Display the dialog
		if (GTK_RESPONSE_ACCEPT != gtk_dialog_run(GTK_DIALOG(main_dialog)))
		{
			// The dialog was cancelled, so destroy it and return to the caller
			gtk_widget_destroy(GTK_WIDGET(main_dialog));
			g_string_free(valid_proj_name, TRUE);
			g_string_free(valid_project_folder, TRUE);
			g_string_free(valid_output_folder, TRUE);
			return;
		}

		// Reset the useable input flag
		useable_input = TRUE;

		// Validate the project name input
		validated_string = validate_value(PROJECT_NAME, V_CHAR, (gchar *) gtk_entry_get_text(GTK_ENTRY(entry_project_name)));
		if (NULL == validated_string)
		{
			display_warning("Error ED135: There was something wrong with the project name value.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_proj_name = g_string_assign(valid_proj_name, validated_string->str);
			g_string_free(validated_string, TRUE);
			validated_string = NULL;
		}

		// Retrieve the new project folder input
		validated_string = validate_value(FOLDER_PATH, V_CHAR, gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(button_project_folder)));
		if (NULL == validated_string)
		{
			display_warning("Error ED136: There was something wrong with the project folder given.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_project_folder = g_string_assign(valid_project_folder, validated_string->str);
			g_string_free(validated_string, TRUE);
			validated_string = NULL;
		}

		// Retrieve the new output folder input
		validated_string = validate_value(FOLDER_PATH, V_CHAR, gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(button_output_folder)));
		if (NULL == validated_string)
		{
			display_warning("Error ED137: There was something wrong with the output folder given.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_output_folder = g_string_assign(valid_output_folder, validated_string->str);
			g_string_free(validated_string, TRUE);
			validated_string = NULL;
		}

		// Retrieve the new project default slide length input
		guint_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_slide_length));
		validated_guint = validate_value(SLIDE_LENGTH, V_INT_UNSIGNED, &guint_val);
		if (NULL == validated_guint)
		{
			display_warning("Error ED138: There was something wrong with the project default slide length value.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_slide_length = *validated_guint;
			g_free(validated_guint);
		}

		// Retrieve the new frames per second input
		guint_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_frames_per_second));
		validated_guint = validate_value(PROJECT_FPS, V_INT_UNSIGNED, &guint_val);
		if (NULL == validated_guint)
		{
			display_warning("Error ED139: There was something wrong with the default frames per second value.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_fps = *validated_guint;
			g_free(validated_guint);
		}

		// Retrieve the new start behaviour input
		gint_val = gtk_combo_box_get_active(GTK_COMBO_BOX(selector_start_behaviour));
		if (-1 == gint_val)
		{
			// A -1 return means no value was selected
			display_warning("Error ED280: There was something wrong with the start behaviour value selected.  Please try again.");
			useable_input = FALSE;
		} else
		{
			// A value was selected
			valid_start_behaviour = gint_val;
		}

		// Retrieve the new end behaviour input
		gint_val = gtk_combo_box_get_active(GTK_COMBO_BOX(selector_end_behaviour));
		if (-1 == gint_val)
		{
			// A -1 return means no value was selected
			display_warning("Error ED277: There was something wrong with the end behaviour value selected.  Please try again.");
			useable_input = FALSE;
		} else
		{
			// A value was selected
			valid_end_behaviour = gint_val;
		}

	} while (FALSE == useable_input);

	// * We only get here after all input is considered valid *

	// Destroy the dialog box
	gtk_widget_destroy(GTK_WIDGET(main_dialog));

	// Project Name
	project_name = g_string_assign(project_name, valid_proj_name->str);
	g_string_free(valid_proj_name, TRUE);

	// Project Folder
	project_folder = g_string_assign(project_folder, valid_project_folder->str);
	g_string_free(valid_project_folder, TRUE);

	// Output Folder
	output_folder = g_string_assign(output_folder, valid_output_folder->str);
	g_string_free(valid_output_folder, TRUE);

	// Slide Length
	slide_length = valid_slide_length;

	// Frames per second
	frames_per_second = valid_fps;

	// Start behaviour
	start_behaviour = valid_start_behaviour;

	// End behaviour
	end_behaviour = valid_end_behaviour;

	// Update the status bar
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, " Project properties updated");
	gdk_flush();

	// Free the memory allocated in this function
	g_string_free(tmp_gstring, TRUE);
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.9  2008/03/09 14:57:09  vapour
 * Renamed the enums for end behaviour, added a Start Behaviour option to the project preferences dialog.
 *
 * Revision 1.8  2008/03/06 00:15:40  vapour
 * Added an end behaviour project preference.
 *
 * Revision 1.7  2008/03/03 02:57:47  vapour
 * Added status bar feedback message.
 *
 * Revision 1.6  2008/02/20 06:03:01  vapour
 * Updated to validate all input.
 *
 * Revision 1.5  2008/02/14 16:56:01  vapour
 * Updated dialog box to use the limits defined for each field in the valid fields array, for frames per second and project name.
 *
 * Revision 1.4  2008/02/05 09:17:49  vapour
 * Removed support of output quality variable, as the concept is no longer relevant.
 *
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
