/*
 * $Id$
 *
 * Flame Project: Display a dialog box asking for mouse pointer settings 
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
#include "validate_value.h"


gboolean display_dialog_mouse(layer *tmp_layer, gchar *dialog_title, gboolean request_file)
{
	// Local variables
	guint				guint_val;					// Temporary guint value used for validation
	GtkDialog			*mouse_dialog;				// Widget for the dialog
	GtkWidget			*mouse_table;				// Table used for neat layout of the dialog box
	guint				row_counter = 0;			// Used to count which row things are up to
	gboolean			useable_input;				// Used as a flag to indicate if all validation was successful
	GString				*valid_ext_link;			// Receives the new external link once validated
	GString				*valid_ext_link_win;		// Receives the new external link window once validated
	guint				valid_finish_frame;			// Receives the new finish frame once validated
	guint				valid_start_frame;			// Receives the new start frame once validated
	guint				valid_x_offset_finish;		// Receives the new finish frame x offset once validated
	guint				valid_x_offset_start;		// Receives the new start frame x offset once validated
	guint				valid_y_offset_finish;		// Receives the new finish frame y offset once validated
	guint				valid_y_offset_start;		// Receives the new start frame y offset once validated
	guint				*validated_guint;			// Receives known good guint values from the validation function
	GString				*validated_string;			// Receives known good strings from the validation function

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

	GtkWidget			*click_button;				// Label widget

	GtkWidget			*external_link_label;		// Label widget
	GtkWidget			*external_link_entry;		// Widget for accepting an external link for clicking on

	GtkWidget			*external_link_win_label;	// Label widget
	GtkWidget			*external_link_win_entry;	//

	layer_mouse			*tmp_mouse_ob;				// Temporary layer object


	// Initialise some things
	tmp_mouse_ob = (layer_mouse *) tmp_layer->object_data;
	valid_ext_link = g_string_new(NULL);
	valid_ext_link_win = g_string_new(NULL);

	// * Open a dialog box asking the user for the details of the layer *

	// Create the dialog window, and table to hold its children
	mouse_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(dialog_title, GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL));
	mouse_table = gtk_table_new(3, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(mouse_dialog->vbox), GTK_WIDGET(mouse_table), FALSE, FALSE, 10);

	// Create the label asking for the starting X Offset
	x_off_label_start = gtk_label_new("Start X Offset: ");
	gtk_misc_set_alignment(GTK_MISC(x_off_label_start), 0, 0.5);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(x_off_label_start), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the starting X Offset input
	x_off_button_start = gtk_spin_button_new_with_range(0, project_width, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(x_off_button_start), tmp_layer->x_offset_start);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(x_off_button_start), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the starting Y Offset
	y_off_label_start = gtk_label_new("Start Y Offset: ");
	gtk_misc_set_alignment(GTK_MISC(y_off_label_start), 0, 0.5);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(y_off_label_start), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the starting Y Offset input
	y_off_button_start = gtk_spin_button_new_with_range(0, project_height, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(y_off_button_start), tmp_layer->y_offset_start);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(y_off_button_start), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the finishing X Offset
	x_off_label_finish = gtk_label_new("Finish X Offset: ");
	gtk_misc_set_alignment(GTK_MISC(x_off_label_finish), 0, 0.5);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(x_off_label_finish), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the finishing X Offset input
	x_off_button_finish = gtk_spin_button_new_with_range(0, project_width, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(x_off_button_finish), tmp_layer->x_offset_finish);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(x_off_button_finish), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the finishing Y Offset
	y_off_label_finish = gtk_label_new("Finish Y Offset: ");
	gtk_misc_set_alignment(GTK_MISC(y_off_label_finish), 0, 0.5);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(y_off_label_finish), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the finishing Y Offset input
	y_off_button_finish = gtk_spin_button_new_with_range(0, project_height, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(y_off_button_finish), tmp_layer->y_offset_finish);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(y_off_button_finish), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the starting frame
	start_label = gtk_label_new("Start frame: ");
	gtk_misc_set_alignment(GTK_MISC(start_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(start_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the starting frame input
	start_button = gtk_spin_button_new_with_range(0, valid_fields[FRAME_NUMBER].max_value, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(start_button), tmp_layer->start_frame);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(start_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the finishing frame
	finish_label = gtk_label_new("Finish frame: ");
	gtk_misc_set_alignment(GTK_MISC(finish_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(finish_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the finishing frame input
	finish_button = gtk_spin_button_new_with_range(0, valid_fields[FRAME_NUMBER].max_value, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(finish_button), tmp_layer->finish_frame);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(finish_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the button asking if there should be a mouse click sound
	click_button = gtk_check_button_new_with_label("Include mouse click?");
	if (MOUSE_NONE == tmp_mouse_ob->click)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(click_button), FALSE);
	} else
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(click_button), TRUE);
	}
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(click_button), 0, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for an external link
	external_link_label = gtk_label_new("External link: ");
	gtk_misc_set_alignment(GTK_MISC(external_link_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(external_link_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts an external link
	external_link_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(external_link_entry), valid_fields[EXTERNAL_LINK].max_value);
	gtk_entry_set_text(GTK_ENTRY(external_link_entry), tmp_layer->external_link->str);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(external_link_entry), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the window to open the external link in
	external_link_win_label = gtk_label_new("External link window: ");
	gtk_misc_set_alignment(GTK_MISC(external_link_win_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(external_link_win_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts a text string for the window to open the external link in
	external_link_win_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(external_link_win_entry), valid_fields[EXTERNAL_LINK_WINDOW].max_value);
	gtk_entry_set_text(GTK_ENTRY(external_link_win_entry), tmp_layer->external_link_window->str);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(external_link_win_entry), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Ensure everything will show
	gtk_widget_show_all(GTK_WIDGET(mouse_dialog));

	// Loop around until we have all valid values, or the user cancels out
	validated_string = NULL;
	do
	{
		// Display the dialog
		if (GTK_RESPONSE_ACCEPT != gtk_dialog_run(GTK_DIALOG(mouse_dialog)))
		{
			// The dialog was cancelled, so destroy it and return to the caller
			gtk_widget_destroy(GTK_WIDGET(mouse_dialog));
			g_string_free(valid_ext_link, TRUE);
			g_string_free(valid_ext_link_win, TRUE);
			return FALSE;
		}

		// Reset the useable input flag
		useable_input = TRUE;

		// Retrieve the new starting frame x offset
		guint_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(x_off_button_start));
		validated_guint = validate_value(X_OFFSET, V_INT_UNSIGNED, &guint_val);
		if (NULL == validated_guint)
		{
			display_warning("Error ED164: There was something wrong with the starting frame X offset value.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_x_offset_start = *validated_guint;
			g_free(validated_guint);
		}

		// Retrieve the new starting frame y offset
		guint_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(y_off_button_start));
		validated_guint = validate_value(Y_OFFSET, V_INT_UNSIGNED, &guint_val);
		if (NULL == validated_guint)
		{
			display_warning("Error ED165: There was something wrong with the starting frame Y offset value.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_y_offset_start = *validated_guint;
			g_free(validated_guint);
		}

		// Retrieve the new finish frame x offset
		guint_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(x_off_button_finish));
		validated_guint = validate_value(X_OFFSET, V_INT_UNSIGNED, &guint_val);
		if (NULL == validated_guint)
		{
			display_warning("Error ED166: There was something wrong with the finish frame X offset value.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_x_offset_finish = *validated_guint;
			g_free(validated_guint);
		}

		// Retrieve the new finish frame y offset
		guint_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(y_off_button_finish));
		validated_guint = validate_value(Y_OFFSET, V_INT_UNSIGNED, &guint_val);
		if (NULL == validated_guint)
		{
			display_warning("Error ED167: There was something wrong with the finish frame X offset value.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_y_offset_finish = *validated_guint;
			g_free(validated_guint);
		}

		// Retrieve the new start frame
		guint_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(start_button));
		validated_guint = validate_value(FRAME_NUMBER, V_INT_UNSIGNED, &guint_val);
		if (NULL == validated_guint)
		{
			display_warning("Error ED168: There was something wrong with the start frame value.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_start_frame = *validated_guint;
			g_free(validated_guint);
		}

		// Retrieve the new finish frame
		guint_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(finish_button));
		validated_guint = validate_value(FRAME_NUMBER, V_INT_UNSIGNED, &guint_val);
		if (NULL == validated_guint)
		{
			display_warning("Error ED169: There was something wrong with the finish frame value.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_finish_frame = *validated_guint;
			g_free(validated_guint);
		}

		// Validate the external link input
		validated_string = validate_value(EXTERNAL_LINK, V_CHAR, (gchar *) gtk_entry_get_text(GTK_ENTRY(external_link_entry)));
		if (NULL == validated_string)
		{
			display_warning("Error ED170: There was something wrong with the external link value.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_ext_link = g_string_assign(valid_ext_link, validated_string->str);
			g_string_free(validated_string, TRUE);
			validated_string = NULL;
		}

		// Validate the external link window input
		validated_string = validate_value(EXTERNAL_LINK, V_CHAR, (gchar *) gtk_entry_get_text(GTK_ENTRY(external_link_win_entry)));
		if (NULL == validated_string)
		{
			display_warning("Error ED171: There was something wrong with the external link target window value.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_ext_link_win = g_string_assign(valid_ext_link_win, validated_string->str);
			g_string_free(validated_string, TRUE);
			validated_string = NULL;
		}
	} while (FALSE == useable_input);

	// * We only get here after all input is considered valid *

	// Fill out the temporary layer with the requested details
	tmp_layer->x_offset_start = valid_x_offset_start;
	tmp_layer->y_offset_start = valid_y_offset_start;
	tmp_layer->x_offset_finish = valid_x_offset_finish;
	tmp_layer->y_offset_finish = valid_y_offset_finish;
	tmp_layer->start_frame = valid_start_frame;
	tmp_layer->finish_frame = valid_finish_frame;
	if (TRUE == gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(click_button)))
	{
		tmp_mouse_ob->click = MOUSE_LEFT_ONE;
	} else
	{
		tmp_mouse_ob->click = MOUSE_NONE;
	}
	g_string_printf(tmp_layer->external_link, "%s", valid_ext_link->str);
	g_string_printf(tmp_layer->external_link_window, "%s", valid_ext_link_win->str);

	// Destroy the dialog box
	gtk_widget_destroy(GTK_WIDGET(mouse_dialog));

	// Free the memory used in this function
	g_string_free(valid_ext_link, TRUE);
	g_string_free(valid_ext_link_win, TRUE);

	return TRUE;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.6  2008/02/20 20:57:01  vapour
 * Updated to validate all incoming input.
 *
 * Revision 1.5  2008/02/12 05:18:50  vapour
 * Adjusted to work with the new, slightly simplified layer structure.
 *
 * Revision 1.4  2008/02/04 14:30:34  vapour
 *  + Removed unnecessary includes.
 *  + Improved spacing between table cells.
 *
 * Revision 1.3  2008/02/01 10:36:15  vapour
 * Added code to the dialog to display and accept values for the external link's target window.
 *
 * Revision 1.2  2008/01/15 16:19:06  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.1  2007/09/29 04:22:17  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */
