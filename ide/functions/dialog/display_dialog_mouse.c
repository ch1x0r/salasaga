/*
 * $Id$
 *
 * Salasaga: Display a dialog box asking for mouse pointer settings 
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
#include "display_warning.h"


gboolean display_dialog_mouse(layer *tmp_layer, gchar *dialog_title, gboolean request_file)
{
	// Local variables
	gfloat				gfloat_val;					// Temporary gfloat value
	gint				gint_val;					// Temporary gint value
	guint				guint_val;					// Temporary guint value used for validation
	GString				*message;					// Used to construct message strings
	GtkDialog			*mouse_dialog;				// Widget for the dialog
	GtkWidget			*mouse_table;				// Table used for neat layout of the dialog box
	guint				row_counter = 0;			// Used to count which row things are up to
	gboolean			useable_input;				// Used as a flag to indicate if all validation was successful
	guint				valid_click_type = 0;		// Receives the new mouse click type once validated
	gfloat				valid_duration = 0;			// Receives the new finish frame once validated
	GString				*valid_ext_link;			// Receives the new external link once validated
	GString				*valid_ext_link_win;		// Receives the new external link window once validated
	GString				*valid_name;				// Receives the new layer name once validated
	gfloat				valid_start_time = 0;		// Receives the new start time once validated
	gfloat				valid_trans_in_duration = 0;// Receives the new appearance transition duration once validated
	guint				valid_trans_in_type = 0;	// Receives the new appearance transition type once validated
	gfloat				valid_trans_out_duration = 0;  // Receives the new exit transition duration once validated
	guint				valid_trans_out_type = 0;	// Receives the new exit transition type once validated
	guint				valid_x_offset_finish = 0;	// Receives the new finish frame x offset once validated
	guint				valid_x_offset_start = 0;	// Receives the new start frame x offset once validated
	guint				valid_y_offset_finish = 0;	// Receives the new finish frame y offset once validated
	guint				valid_y_offset_start = 0;	// Receives the new start frame y offset once validated
	gfloat				*validated_gfloat;			// Receives known good gfloat values from the validation function
	guint				*validated_guint;			// Receives known good guint values from the validation function
	GString				*validated_string;			// Receives known good strings from the validation function

	GtkWidget			*name_label;				// Label widget
	GtkWidget			*name_entry;				//

	GtkWidget			*visibility_checkbox;		// Visibility widget

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

	GtkWidget			*duration_label;			// Label widget
	GtkWidget			*duration_button;			//

	GtkWidget			*label_click;				// Mouse click type
	GtkWidget			*selector_click;			//

	GtkWidget			*external_link_label;		// Label widget
	GtkWidget			*external_link_entry;		// Widget for accepting an external link for clicking on

	GtkWidget			*external_link_win_label;	// Label widget
	GtkWidget			*external_link_win_entry;	//

	GtkWidget			*label_trans_in_type;		// Transition in type
	GtkWidget			*selector_trans_in_type;	//

	GtkWidget			*label_trans_in_duration;	// Transition in duration (seconds)
	GtkWidget			*button_trans_in_duration;	//

	GtkWidget			*label_trans_out_type;		// Transition out type
	GtkWidget			*selector_trans_out_type;	//

	GtkWidget			*label_trans_out_duration;	// Transition out duration (seconds)
	GtkWidget			*button_trans_out_duration;	//

	layer_mouse			*tmp_mouse_ob;				// Temporary layer object


	// Initialise some things
	tmp_mouse_ob = (layer_mouse *) tmp_layer->object_data;
	message = g_string_new(NULL); 
	valid_ext_link = g_string_new(NULL);
	valid_ext_link_win = g_string_new(NULL);
	valid_name = g_string_new(NULL);

	// * Open a dialog box asking the user for the details of the layer *

	// Create the dialog window, and table to hold its children
	mouse_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(dialog_title, GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL));
	mouse_table = gtk_table_new(3, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(mouse_dialog->vbox), GTK_WIDGET(mouse_table), FALSE, FALSE, 10);

	// Create the label asking for the layer name
	name_label = gtk_label_new(_("Layer Name: "));
	gtk_misc_set_alignment(GTK_MISC(name_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(name_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the new layer name
	name_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(name_entry), valid_fields[LAYER_NAME].max_value);
	gtk_entry_set_text(GTK_ENTRY(name_entry), tmp_layer->name->str);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(name_entry), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the starting X Offset
	x_off_label_start = gtk_label_new(_("Start X Offset: "));
	gtk_misc_set_alignment(GTK_MISC(x_off_label_start), 0, 0.5);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(x_off_label_start), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the starting X Offset input
	x_off_button_start = gtk_spin_button_new_with_range(0, project_width, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(x_off_button_start), tmp_layer->x_offset_start);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(x_off_button_start), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the starting Y Offset
	y_off_label_start = gtk_label_new(_("Start Y Offset: "));
	gtk_misc_set_alignment(GTK_MISC(y_off_label_start), 0, 0.5);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(y_off_label_start), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the starting Y Offset input
	y_off_button_start = gtk_spin_button_new_with_range(0, project_height, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(y_off_button_start), tmp_layer->y_offset_start);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(y_off_button_start), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the finishing X Offset
	x_off_label_finish = gtk_label_new(_("Finish X Offset: "));
	gtk_misc_set_alignment(GTK_MISC(x_off_label_finish), 0, 0.5);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(x_off_label_finish), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the finishing X Offset input
	x_off_button_finish = gtk_spin_button_new_with_range(0, project_width, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(x_off_button_finish), tmp_layer->x_offset_finish);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(x_off_button_finish), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the finishing Y Offset
	y_off_label_finish = gtk_label_new(_("Finish Y Offset: "));
	gtk_misc_set_alignment(GTK_MISC(y_off_label_finish), 0, 0.5);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(y_off_label_finish), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the finishing Y Offset input
	y_off_button_finish = gtk_spin_button_new_with_range(0, project_height, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(y_off_button_finish), tmp_layer->y_offset_finish);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(y_off_button_finish), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the check box asking if the layer should be visible
	visibility_checkbox = gtk_check_button_new_with_label(_("Is this layer visible?"));
	if (FALSE == tmp_layer->visible)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(visibility_checkbox), FALSE);
	} else
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(visibility_checkbox), TRUE);
	}
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(visibility_checkbox), 0, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the starting time
	start_label = gtk_label_new(_("Starting time (seconds): "));
	gtk_misc_set_alignment(GTK_MISC(start_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(start_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the starting time input
	start_button = gtk_spin_button_new_with_range(valid_fields[FRAME_NUMBER].min_value, valid_fields[FRAME_NUMBER].max_value, 0.1);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(start_button), 2);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(start_button), tmp_layer->start_time);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(start_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Appearance transition type
	label_trans_in_type = gtk_label_new(_("Start how: "));
	gtk_misc_set_alignment(GTK_MISC(label_trans_in_type), 0, 0.5);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(label_trans_in_type), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	selector_trans_in_type = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(selector_trans_in_type), _("Immediate"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(selector_trans_in_type), _("Fade in"));
	switch (tmp_layer->transition_in_type)
	{
		case TRANS_LAYER_FADE:
			gtk_combo_box_set_active(GTK_COMBO_BOX(selector_trans_in_type), TRANS_LAYER_FADE);
			break;

		default:
			gtk_combo_box_set_active(GTK_COMBO_BOX(selector_trans_in_type), TRANS_LAYER_NONE);
	}
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(selector_trans_in_type), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Appearance transition duration label
	label_trans_in_duration = gtk_label_new(_("Start duration (seconds):"));
	gtk_misc_set_alignment(GTK_MISC(label_trans_in_duration), 0, 0.5);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(label_trans_in_duration), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Appearance transition duration entry
	button_trans_in_duration = gtk_spin_button_new_with_range(valid_fields[TRANSITION_DURATION].min_value, valid_fields[TRANSITION_DURATION].max_value, 0.1);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(button_trans_in_duration), 2);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_trans_in_duration), tmp_layer->transition_in_duration);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(button_trans_in_duration), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the layer duration
	duration_label = gtk_label_new(_("Display for (seconds): "));
	gtk_misc_set_alignment(GTK_MISC(duration_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(duration_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the duration input
	duration_button = gtk_spin_button_new_with_range(valid_fields[FRAME_NUMBER].min_value, valid_fields[FRAME_NUMBER].max_value, 0.1);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(duration_button), 2);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(duration_button), tmp_layer->duration);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(duration_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Exit Transition type
	label_trans_out_type = gtk_label_new(_("Exit how: "));
	gtk_misc_set_alignment(GTK_MISC(label_trans_out_type), 0, 0.5);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(label_trans_out_type), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	selector_trans_out_type = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(selector_trans_out_type), _("Immediate"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(selector_trans_out_type), _("Fade out"));
	switch (tmp_layer->transition_out_type)
	{
		case TRANS_LAYER_FADE:
			gtk_combo_box_set_active(GTK_COMBO_BOX(selector_trans_out_type), TRANS_LAYER_FADE);
			break;

		default:
			gtk_combo_box_set_active(GTK_COMBO_BOX(selector_trans_out_type), TRANS_LAYER_NONE);
	}
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(selector_trans_out_type), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Exit transition duration label
	label_trans_out_duration = gtk_label_new(_("Exit duration (seconds):"));
	gtk_misc_set_alignment(GTK_MISC(label_trans_out_duration), 0, 0.5);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(label_trans_out_duration), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Exit transition duration entry
	button_trans_out_duration = gtk_spin_button_new_with_range(valid_fields[TRANSITION_DURATION].min_value, valid_fields[TRANSITION_DURATION].max_value, 0.1);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(button_trans_out_duration), 2);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_trans_out_duration), tmp_layer->transition_out_duration);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(button_trans_out_duration), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Mouse click type
	label_click = gtk_label_new(_("Mouse click type: "));
	gtk_misc_set_alignment(GTK_MISC(label_click), 0, 0.5);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(label_click), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	selector_click = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(selector_click), _("No mouse click"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(selector_click), _("Left single click"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(selector_click), _("Left double click"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(selector_click), _("Left triple click"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(selector_click), _("Right single click"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(selector_click), _("Right double click"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(selector_click), _("Right triple click"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(selector_click), _("Middle single click"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(selector_click), _("Middle double click"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(selector_click), _("Middle triple click"));
	switch (tmp_mouse_ob->click)
	{
		case MOUSE_NONE:
			gtk_combo_box_set_active(GTK_COMBO_BOX(selector_click), MOUSE_NONE);
			break;

		case MOUSE_LEFT_ONE:
			gtk_combo_box_set_active(GTK_COMBO_BOX(selector_click), MOUSE_LEFT_ONE);
			break;

		case MOUSE_LEFT_DOUBLE:
			gtk_combo_box_set_active(GTK_COMBO_BOX(selector_click), MOUSE_LEFT_DOUBLE);
			break;

		case MOUSE_LEFT_TRIPLE:
			gtk_combo_box_set_active(GTK_COMBO_BOX(selector_click), MOUSE_LEFT_TRIPLE);
			break;

		case MOUSE_RIGHT_ONE:
			gtk_combo_box_set_active(GTK_COMBO_BOX(selector_click), MOUSE_RIGHT_ONE);
			break;

		case MOUSE_RIGHT_DOUBLE:
			gtk_combo_box_set_active(GTK_COMBO_BOX(selector_click), MOUSE_RIGHT_DOUBLE);
			break;

		case MOUSE_RIGHT_TRIPLE:
			gtk_combo_box_set_active(GTK_COMBO_BOX(selector_click), MOUSE_RIGHT_TRIPLE);
			break;

		case MOUSE_MIDDLE_ONE:
			gtk_combo_box_set_active(GTK_COMBO_BOX(selector_click), MOUSE_MIDDLE_ONE);
			break;

		case MOUSE_MIDDLE_DOUBLE:
			gtk_combo_box_set_active(GTK_COMBO_BOX(selector_click), MOUSE_MIDDLE_DOUBLE);
			break;

		case MOUSE_MIDDLE_TRIPLE:
			gtk_combo_box_set_active(GTK_COMBO_BOX(selector_click), MOUSE_MIDDLE_TRIPLE);
			break;

		default:
			gtk_combo_box_set_active(GTK_COMBO_BOX(selector_click), MOUSE_NONE);
	}
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(selector_click), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for an external link
	external_link_label = gtk_label_new(_("External link: "));
	gtk_misc_set_alignment(GTK_MISC(external_link_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(external_link_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts an external link
	external_link_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(external_link_entry), valid_fields[EXTERNAL_LINK].max_value);
	gtk_entry_set_text(GTK_ENTRY(external_link_entry), tmp_layer->external_link->str);
	gtk_table_attach(GTK_TABLE(mouse_table), GTK_WIDGET(external_link_entry), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the window to open the external link in
	external_link_win_label = gtk_label_new(_("External link window: "));
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

		// Validate the layer name input
		validated_string = validate_value(LAYER_NAME, V_CHAR, (gchar *) gtk_entry_get_text(GTK_ENTRY(name_entry)));
		if (NULL == validated_string)
		{
			g_string_printf(message, "%s ED383: %s", _("Error"), _("There was something wrong with the new layer name.  Please try again."));
			display_warning(message->str);
			useable_input = FALSE;
		} else
		{
			g_string_assign(valid_name, validated_string->str);
			g_string_free(validated_string, TRUE);
			validated_string = NULL;
		}

		// Retrieve the new starting frame x offset
		guint_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(x_off_button_start));
		validated_guint = validate_value(X_OFFSET, V_INT_UNSIGNED, &guint_val);
		if (NULL == validated_guint)
		{
			g_string_printf(message, "%s ED164: %s", _("Error"), _("There was something wrong with the starting frame X offset value.  Please try again."));
			display_warning(message->str);
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
			g_string_printf(message, "%s ED165: %s", _("Error"), _("There was something wrong with the starting frame Y offset value.  Please try again."));
			display_warning(message->str);
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
			g_string_printf(message, "%s ED166: %s", _("Error"), _("There was something wrong with the finish frame X offset value.  Please try again."));
			display_warning(message->str);
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
			g_string_printf(message, "%s ED167: %s", _("Error"), _("There was something wrong with the finish frame X offset value.  Please try again."));
			display_warning(message->str);
			useable_input = FALSE;
		} else
		{
			valid_y_offset_finish = *validated_guint;
			g_free(validated_guint);
		}

		// Retrieve the new starting time
		gfloat_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(start_button));
		validated_gfloat = validate_value(LAYER_DURATION, V_FLOAT_UNSIGNED, &gfloat_val);
		if (NULL == validated_gfloat)
		{
			g_string_printf(message, "%s ED168: %s", _("Error"), _("There was something wrong with the starting time value.  Please try again."));
			display_warning(message->str);
			useable_input = FALSE;
		} else
		{
			valid_start_time = *validated_gfloat;
			g_free(validated_gfloat);
		}

		// Retrieve the new duration
		gfloat_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(duration_button));
		validated_gfloat = validate_value(LAYER_DURATION, V_FLOAT_UNSIGNED, &gfloat_val);
		if (NULL == validated_gfloat)
		{
			g_string_printf(message, "%s ED169: %s", _("Error"), _("There was something wrong with the duration value.  Please try again."));
			display_warning(message->str);
			useable_input = FALSE;
		} else
		{
			valid_duration = *validated_gfloat;
			g_free(validated_gfloat);
		}

		// Validate the external link input
		validated_string = validate_value(EXTERNAL_LINK, V_CHAR, (gchar *) gtk_entry_get_text(GTK_ENTRY(external_link_entry)));
		if (NULL == validated_string)
		{
			g_string_printf(message, "%s ED170: %s", _("Error"), _("There was something wrong with the external link value.  Please try again."));
			display_warning(message->str);
			useable_input = FALSE;
		} else
		{
			valid_ext_link = g_string_assign(valid_ext_link, validated_string->str);
			g_string_free(validated_string, TRUE);
			validated_string = NULL;
		}

		// Validate the external link window input
		validated_string = validate_value(EXTERNAL_LINK_WINDOW, V_CHAR, (gchar *) gtk_entry_get_text(GTK_ENTRY(external_link_win_entry)));
		if (NULL == validated_string)
		{
			g_string_printf(message, "%s ED171: %s", _("Error"), _("There was something wrong with the external link target window value.  Please try again."));
			display_warning(message->str);
			useable_input = FALSE;
		} else
		{
			valid_ext_link_win = g_string_assign(valid_ext_link_win, validated_string->str);
			g_string_free(validated_string, TRUE);
			validated_string = NULL;
		}

		// Retrieve the transition in type
		gint_val = gtk_combo_box_get_active(GTK_COMBO_BOX(selector_trans_in_type));
		if (-1 == gint_val)
		{
			// A -1 return means no value was selected
			g_string_printf(message, "%s ED305: %s", _("Error"), _("There was something wrong with the appearance transition type selected.  Please try again."));
			display_warning(message->str);
			useable_input = FALSE;
		} else
		{
			// A value was selected
			valid_trans_in_type = gint_val;
		}

		// Retrieve the transition in duration
		gfloat_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_trans_in_duration));
		validated_gfloat = validate_value(TRANSITION_DURATION, V_FLOAT_UNSIGNED, &gfloat_val);
		if (NULL == validated_gfloat)
		{
			g_string_printf(message, "%s ED306: %s", _("Error"), _("There was something wrong with the appearance transition duration value.  Please try again."));
			display_warning(message->str);
			useable_input = FALSE;
		} else
		{
			valid_trans_in_duration = *validated_gfloat;
			g_free(validated_gfloat);
		}

		// Retrieve the transition out type
		gint_val = gtk_combo_box_get_active(GTK_COMBO_BOX(selector_trans_out_type));
		if (-1 == gint_val)
		{
			// A -1 return means no value was selected
			g_string_printf(message, "%s ED307: %s", _("Error"), _("There was something wrong with the exit transition type selected.  Please try again."));
			display_warning(message->str);
			useable_input = FALSE;
		} else
		{
			// A value was selected
			valid_trans_out_type = gint_val;
		}

		// Retrieve the transition out duration
		gfloat_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_trans_out_duration));
		validated_gfloat = validate_value(TRANSITION_DURATION, V_FLOAT_UNSIGNED, &gfloat_val);
		if (NULL == validated_gfloat)
		{
			g_string_printf(message, "%s ED308: %s", _("Error"), _("There was something wrong with the exit transition duration value.  Please try again."));
			display_warning(message->str);
			useable_input = FALSE;
		} else
		{
			valid_trans_out_duration = *validated_gfloat;
			g_free(validated_gfloat);
		}

		// Retrieve the mouse click type
		gint_val = gtk_combo_box_get_active(GTK_COMBO_BOX(selector_click));
		if (-1 == gint_val)
		{
			// A -1 return means no value was selected
			g_string_printf(message, "%s ED413: %s", _("Error"), _("There was something wrong with the mouse click type selected.  Please try again."));
			display_warning(message->str);
			useable_input = FALSE;
		} else
		{
			// A value was selected
			valid_click_type = gint_val;
		}
	} while (FALSE == useable_input);

	// * We only get here after all input is considered valid *

	// Fill out the temporary layer with the requested details
	g_string_printf(tmp_layer->name, "%s", valid_name->str);
	tmp_layer->x_offset_start = valid_x_offset_start;
	tmp_layer->y_offset_start = valid_y_offset_start;
	tmp_layer->x_offset_finish = valid_x_offset_finish;
	tmp_layer->y_offset_finish = valid_y_offset_finish;
	tmp_layer->start_time = valid_start_time;
	tmp_layer->duration = valid_duration;
	tmp_mouse_ob->click = valid_click_type;
	if (TRUE == gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(visibility_checkbox)))
	{
		tmp_layer->visible = TRUE;
	} else
	{
		tmp_layer->visible = FALSE;
	}
	g_string_printf(tmp_layer->external_link, "%s", valid_ext_link->str);
	g_string_printf(tmp_layer->external_link_window, "%s", valid_ext_link_win->str);
	tmp_layer->transition_in_type = valid_trans_in_type;
	tmp_layer->transition_in_duration = valid_trans_in_duration;
	tmp_layer->transition_out_type = valid_trans_out_type;
	tmp_layer->transition_out_duration = valid_trans_out_duration;

	// Destroy the dialog box
	gtk_widget_destroy(GTK_WIDGET(mouse_dialog));

	// Free the memory used in this function
	g_string_free(message, TRUE);
	g_string_free(valid_ext_link, TRUE);
	g_string_free(valid_ext_link_win, TRUE);
	g_string_free(valid_name, TRUE);

	return TRUE;
}