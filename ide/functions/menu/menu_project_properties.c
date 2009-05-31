/*
 * $Id$
 *
 * Salasaga: Function called when the user selects Project -> Properties from the top menu
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
#include "../validate_value.h"
#include "../dialog/display_warning.h"


void menu_project_properties(void)
{
	// Local variables
	gint				gint_val;					// Temporary gint value
	guint				guint_val;					// Temporary guint value used for validation
	GtkDialog			*main_dialog;				// Widget for the main dialog
	GString				*message;					// Used to construct message strings
	GtkWidget			*proj_dialog_table;			// Table used for neat layout of the labels and fields in project preferences
	gint				proj_row_counter;			// Used when building the project preferences dialog box
	gboolean			usable_input;				// Used as a flag to indicate if all validation was successful
	gboolean			valid_control_bar_behaviour;  // Receives the new control bar display behaviour
	guint				valid_end_behaviour = 0;	// Receives the new end behaviour once validated
	GString				*valid_ext_link;			// Receives the new external link once validated
	GString				*valid_ext_link_win;		// Receives the new external link window once validated
	guint				valid_fps = 0;				// Receives the new project fps once validated
	gboolean			valid_info_display;			// Receives the new informatio button display behaviour
	GString				*valid_output_folder;		// Receives the new output folder once validated
	GString				*valid_proj_name;			// Receives the new project name once validated
	GString				*valid_project_folder;		// Receives the new project folder once validated
	guint				valid_start_behaviour = 0;	// Receives the new start behaviour once validated
	guint				*validated_guint;			// Receives known good guint values from the validation function
	GString				*validated_string;			// Receives known good strings from the validation function

	GtkWidget			*label_project_name;		// Project name
	GtkWidget			*entry_project_name;		//

	GtkWidget			*label_project_folder;		// Project folder
	GtkWidget			*button_project_folder;		//

	GtkWidget			*label_output_folder;		// Output folder
	GtkWidget			*button_output_folder;		//

	GtkWidget			*label_frames_per_second;	// Frames per second
	GtkWidget			*button_frames_per_second;	//

	GtkWidget			*label_project_width;		// Project width
	GtkWidget			*entry_project_width;		//

	GtkWidget			*label_project_height;		// Project height
	GtkWidget			*entry_project_height;		//

	GtkWidget			*label_start_behaviour;		// Start behaviour
	GtkWidget			*selector_start_behaviour;	//

	GtkWidget			*label_end_behaviour;		// End behaviour
	GtkWidget			*selector_end_behaviour;	//

	GtkWidget			*label_control_bar;			// Display control bar
	GtkWidget			*check_control_bar;			//

	GtkWidget			*label_display_info;		// Display information button
	GtkWidget			*check_display_info;		//

	// * Information button variables *

	GtkWidget			*label_info_text;			// Label for the information text field
	GtkTextBuffer		*text_buffer;				// Temporary text buffer the user words with
	GtkTextIter			text_end;					// End position of text buffer
	GtkWidget			*text_frame;				// Frame to go around the text widget
	GString				*text_gstring;				// Temporary text buffer
	GtkTextIter			text_start;					// Start position of text buffer
	GtkWidget			*text_view;					// Widget for accepting the new text data

	GtkWidget			*external_link_label;		// Label widget
	GtkWidget			*external_link_entry;		//

	GtkWidget			*external_link_win_label;	// Label widget
	GtkWidget			*external_link_win_entry;	//

	GString				*tmp_gstring;				// Temporary GString used for constructing text


	// Initialise various things
	message = g_string_new(NULL);
	proj_row_counter = 0;
	valid_ext_link = g_string_new(NULL);
	valid_ext_link_win = g_string_new(NULL);
	valid_output_folder = g_string_new(NULL);
	valid_proj_name = g_string_new(NULL);
	valid_project_folder = g_string_new(NULL);
	tmp_gstring = g_string_new(NULL);

	// Create the main dialog window
	main_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(_("Project Properties"), GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL));
	proj_dialog_table = gtk_table_new(10, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(main_dialog->vbox), GTK_WIDGET(proj_dialog_table), FALSE, FALSE, 5);

	// Project Name
	label_project_name = gtk_label_new(_("Project Name: "));
	gtk_misc_set_alignment(GTK_MISC(label_project_name), 0, 0.5);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(label_project_name), 0, 1, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	entry_project_name = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry_project_name), valid_fields[PROJECT_NAME].max_value);
	gtk_entry_set_text(GTK_ENTRY(entry_project_name), (gchar *) project_name->str);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(entry_project_name), 2, 3, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	proj_row_counter = proj_row_counter + 1;

	// Project Folder
	label_project_folder = gtk_label_new(_("Project Folder: "));
	gtk_misc_set_alignment(GTK_MISC(label_project_folder), 0, 0.5);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(label_project_folder), 0, 1, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	button_project_folder = gtk_file_chooser_button_new(_("Select the Project Folder"), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(button_project_folder), project_folder->str);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(button_project_folder), 2, 3, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	proj_row_counter = proj_row_counter + 1;

	// Output Folder
	label_output_folder = gtk_label_new(_("Output Folder: "));
	gtk_misc_set_alignment(GTK_MISC(label_output_folder), 0, 0.5);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(label_output_folder), 0, 1, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	button_output_folder = gtk_file_chooser_button_new(_("Select the Output Folder"), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(button_output_folder), output_folder->str);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(button_output_folder), 2, 3, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	proj_row_counter = proj_row_counter + 1;

	// Frames per second
	label_frames_per_second = gtk_label_new(_("Frames per second: "));
	gtk_misc_set_alignment(GTK_MISC(label_frames_per_second), 0, 0.5);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(label_frames_per_second), 0, 1, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	button_frames_per_second = gtk_spin_button_new_with_range(valid_fields[PROJECT_FPS].min_value, valid_fields[PROJECT_FPS].max_value, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_frames_per_second), frames_per_second);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(button_frames_per_second), 2, 3, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	proj_row_counter = proj_row_counter + 1;

	// Project Width
	label_project_width = gtk_label_new(_("Project Width: "));
	gtk_misc_set_alignment(GTK_MISC(label_project_width), 0, 0.5);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(label_project_width), 0, 1, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	entry_project_width = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry_project_width), 12);
	g_string_printf(tmp_gstring, "%d %s", project_width, _("pixels"));
	gtk_entry_set_text(GTK_ENTRY(entry_project_width), tmp_gstring->str);
	gtk_editable_set_editable(GTK_EDITABLE(entry_project_width), FALSE);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(entry_project_width), 2, 3, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	proj_row_counter = proj_row_counter + 1;

	// Project Height
	label_project_height = gtk_label_new(_("Project Height: "));
	gtk_misc_set_alignment(GTK_MISC(label_project_height), 0, 0.5);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(label_project_height), 0, 1, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	entry_project_height = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry_project_height), 12);
	g_string_printf(tmp_gstring, "%d %s", project_height, _("pixels"));
	gtk_entry_set_text(GTK_ENTRY(entry_project_height), tmp_gstring->str);
	gtk_editable_set_editable(GTK_EDITABLE(entry_project_height), FALSE);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(entry_project_height), 2, 3, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	proj_row_counter = proj_row_counter + 1;

	// Start behaviour
	label_start_behaviour = gtk_label_new(_("SWF start behavior: "));
	gtk_misc_set_alignment(GTK_MISC(label_start_behaviour), 0, 0.5);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(label_start_behaviour), 0, 1, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	selector_start_behaviour = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(selector_start_behaviour), _("Paused"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(selector_start_behaviour), _("Play"));
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
	label_end_behaviour = gtk_label_new(_("SWF end behavior: "));
	gtk_misc_set_alignment(GTK_MISC(label_end_behaviour), 0, 0.5);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(label_end_behaviour), 0, 1, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	selector_end_behaviour = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(selector_end_behaviour), _("Stop"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(selector_end_behaviour), _("Loop and play"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(selector_end_behaviour), _("Loop and stop"));
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
	proj_row_counter = proj_row_counter + 1;

	// Display control bar
	label_control_bar = gtk_label_new(_("Display SWF control bar: "));
	gtk_misc_set_alignment(GTK_MISC(label_control_bar), 0, 0.5);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(label_control_bar), 0, 1, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	check_control_bar = gtk_check_button_new();
	if (TRUE == show_control_bar)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_control_bar), TRUE);
	} else
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_control_bar), FALSE);
	}
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(check_control_bar), 2, 3, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	proj_row_counter = proj_row_counter + 1;

	// Display information button
	label_display_info = gtk_label_new(_("Display SWF information button: "));
	gtk_misc_set_alignment(GTK_MISC(label_display_info), 0, 0.5);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(label_display_info), 0, 1, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	check_display_info = gtk_check_button_new();
	if (TRUE == info_display)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_display_info), TRUE);
	} else
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_display_info), FALSE);
	}
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(check_display_info), 2, 3, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	proj_row_counter = proj_row_counter + 1;

	// Create a label for the information button text view
	label_info_text = gtk_label_new(_("Information button text"));
	gtk_misc_set_alignment(GTK_MISC(label_info_text), 0.5, 0.5);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(label_info_text), 0, 3, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	proj_row_counter = proj_row_counter + 1;

	// Create the text view that accepts the new information button text
	text_frame = gtk_frame_new(NULL);
	gtk_container_set_border_width(GTK_CONTAINER(text_frame), 2);
	gtk_frame_set_shadow_type(GTK_FRAME(text_frame), GTK_SHADOW_OUT);
	text_view = gtk_text_view_new();
	gtk_widget_set_size_request(GTK_WIDGET(text_view), 0, 100);
	gtk_container_add(GTK_CONTAINER(text_frame), text_view);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(text_frame), 0, 3, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	proj_row_counter = proj_row_counter + 1;

	// Copy the text string from the existing text buffer to a new, temporary one
	// (Note - this is so we don't work directly with the text buffer, which would keep edits even if the user hits the Cancel button)
	gtk_text_buffer_get_bounds(info_text, &text_start, &text_end);
	text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
	text_gstring = g_string_new(gtk_text_buffer_get_slice(info_text, &text_start, &text_end, TRUE));
	gtk_text_buffer_set_text(GTK_TEXT_BUFFER(text_buffer), text_gstring->str, text_gstring->len);

	// Create the label asking for an external link
	external_link_label = gtk_label_new(_("Information button link: "));
	gtk_misc_set_alignment(GTK_MISC(external_link_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(external_link_label), 0, 1, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts an external link
	external_link_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(external_link_entry), valid_fields[EXTERNAL_LINK].max_value);
	gtk_entry_set_text(GTK_ENTRY(external_link_entry), info_link->str);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(external_link_entry), 2, 3, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	proj_row_counter = proj_row_counter + 1;

	// Create the label asking for the window to open the external link in
	external_link_win_label = gtk_label_new(_("Information button link window: "));
	gtk_misc_set_alignment(GTK_MISC(external_link_win_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(external_link_win_label), 0, 1, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts a text string for the window to open the external link in
	external_link_win_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(external_link_win_entry), valid_fields[EXTERNAL_LINK_WINDOW].max_value);
	gtk_entry_set_text(GTK_ENTRY(external_link_win_entry), info_link_target->str);
	gtk_table_attach(GTK_TABLE(proj_dialog_table), GTK_WIDGET(external_link_win_entry), 2, 3, proj_row_counter, proj_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Ensure everything will be shown
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

		// Reset the usable input flag
		usable_input = TRUE;

		// Validate the project name input
		validated_string = validate_value(PROJECT_NAME, V_CHAR, (gchar *) gtk_entry_get_text(GTK_ENTRY(entry_project_name)));
		if (NULL == validated_string)
		{
			g_string_printf(message, "%s ED135: %s", _("Error"), _("There was something wrong with the project name value.  Please try again."));
			display_warning(message->str);
			usable_input = FALSE;
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
			g_string_printf(message, "%s ED136: %s", _("Error"), _("There was something wrong with the project folder given.  Please try again."));
			display_warning(message->str);
			usable_input = FALSE;
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
			g_string_printf(message, "%s ED137: %s", _("Error"), _("There was something wrong with the output folder given.  Please try again."));
			display_warning(message->str);
			usable_input = FALSE;
		} else
		{
			valid_output_folder = g_string_assign(valid_output_folder, validated_string->str);
			g_string_free(validated_string, TRUE);
			validated_string = NULL;
		}

		// Retrieve the new frames per second input
		guint_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_frames_per_second));
		validated_guint = validate_value(PROJECT_FPS, V_INT_UNSIGNED, &guint_val);
		if (NULL == validated_guint)
		{
			g_string_printf(message, "%s ED139: %s", _("Error"), _("There was something wrong with the default frames per second value.  Please try again."));
			display_warning(message->str);
			usable_input = FALSE;
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
			g_string_printf(message, "%s ED280: %s", _("Error"), _("There was something wrong with the start behavior value selected.  Please try again."));
			display_warning(message->str);
			usable_input = FALSE;
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
			g_string_printf(message, "%s ED277: %s", _("Error"), _("There was something wrong with the end behavior value selected.  Please try again."));
			display_warning(message->str);
			usable_input = FALSE;
		} else
		{
			// A value was selected
			valid_end_behaviour = gint_val;
		}

		// Retrieve the new control bar display behaviour
		valid_control_bar_behaviour = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_control_bar));

		// Retrieve the new information button display behaviour
		valid_info_display = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_display_info));

		// Validate the link input
		validated_string = validate_value(EXTERNAL_LINK, V_CHAR, (gchar *) gtk_entry_get_text(GTK_ENTRY(external_link_entry)));
		if (NULL == validated_string)
		{
			g_string_printf(message, "%s ED407: %s", _("Error"), _("There was something wrong with the information button link value.  Please try again."));
			display_warning(message->str);
			usable_input = FALSE;
		} else
		{
			valid_ext_link = g_string_assign(valid_ext_link, validated_string->str);
			g_string_free(validated_string, TRUE);
			validated_string = NULL;
		}

		// Validate the link window input
		validated_string = validate_value(EXTERNAL_LINK_WINDOW, V_CHAR, (gchar *) gtk_entry_get_text(GTK_ENTRY(external_link_win_entry)));
		if (NULL == validated_string)
		{
			g_string_printf(message, "%s ED408: %s", _("Error"), _("There was something wrong with the information button target window value.  Please try again."));
			display_warning(message->str);
			usable_input = FALSE;
		} else
		{
			valid_ext_link_win = g_string_assign(valid_ext_link_win, validated_string->str);
			g_string_free(validated_string, TRUE);
			validated_string = NULL;
		}
	} while (FALSE == usable_input);

	// * We only get here after all input is considered valid *

	// Copy the text buffer from the onscreen widget to our existing text buffer
	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(text_buffer), &text_start, &text_end);
	text_gstring = g_string_assign(text_gstring, gtk_text_buffer_get_slice(GTK_TEXT_BUFFER(text_buffer), &text_start, &text_end, TRUE));
	gtk_text_buffer_set_text(GTK_TEXT_BUFFER(info_text), text_gstring->str, text_gstring->len);

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

	// Frames per second
	frames_per_second = valid_fps;

	// Start behaviour
	start_behaviour = valid_start_behaviour;

	// End behaviour
	end_behaviour = valid_end_behaviour;

	// Control bar display
	show_control_bar = valid_control_bar_behaviour;

	// Information button display
	info_display = valid_info_display;
	g_string_printf(info_link, "%s", valid_ext_link->str);
	g_string_printf(info_link_target, "%s", valid_ext_link_win->str);

	// Set the changes made variable
	changes_made = TRUE;

	// Update the status bar
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(status_bar), _(" Project properties updated"));
	gdk_flush();

	// Free the memory allocated in this function
	g_string_free(message, TRUE);
	g_string_free(valid_ext_link, TRUE);
	g_string_free(valid_ext_link_win, TRUE);
	g_string_free(text_gstring, TRUE);
	g_string_free(tmp_gstring, TRUE);
}
