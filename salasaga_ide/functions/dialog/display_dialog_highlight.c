/*
 * $Id$
 *
 * Salasaga: Display a dialog box asking for highlight layer settings 
 * 
 * Copyright (C) 2005-2008 Justin Clift <justin@salasaga.org>
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


gboolean display_dialog_highlight(layer *tmp_layer, gchar *dialog_title)
{
	// Local variables
	gfloat				gfloat_val;					// Temporary gfloat value
	gint				gint_val;					// Temporary gint value
	guint				guint_val;					// Temporary guint value used for validation
	GtkDialog			*highlight_dialog;			// Widget for the dialog
	GtkWidget			*highlight_table;			// Table used for neat layout of the dialog box
	guint				row_counter = 0;			// Used to count which row things are up to
	gboolean			useable_input;				// Used as a flag to indicate if all validation was successful
	gfloat				valid_border_width = 0;		// Receives the new border width once validated
	gfloat				valid_duration = 0;			// Receives the new finish frame once validated
	GString				*valid_ext_link;			// Receives the new external link once validated
	GString				*valid_ext_link_win;		// Receives the new external link window once validated
	guint				valid_height = 0;			// Receives the new height value once validated
	GString				*valid_name;				// Receives the new layer name once validated
	gfloat				valid_opacity = 0;			// Receives the new opacity once validated
	gfloat				valid_start_time = 0;		// Receives the new start time once validated
	gfloat				valid_trans_in_duration = 0;// Receives the new appearance transition duration once validated
	guint				valid_trans_in_type = 0;	// Receives the new appearance transition type once validated
	gfloat				valid_trans_out_duration;	// Receives the new exit transition duration once validated
	guint				valid_trans_out_type = 0;	// Receives the new exit transition type once validated
	guint				valid_width = 0;			// Receives the new width value once validated
	guint				valid_x_offset_finish = 0;	// Receives the new finish frame x offset once validated
	guint				valid_x_offset_start = 0;	// Receives the new start frame x offset once validated
	guint				valid_y_offset_finish = 0;	// Receives the new finish frame y offset once validated
	guint				valid_y_offset_start = 0;	// Receives the new start frame y offset once validated
	gfloat				*validated_gfloat;			// Receives known good gfloat values from the validation function
	guint				*validated_guint;			// Receives known good guint values from the validation function
	GString				*validated_string;			// Receives known good strings from the validation function

	GtkWidget			*name_label;				// Label widget
	GtkWidget			*name_entry;				//

	GtkWidget			*fill_colour_label;			// Label widget
	GtkWidget			*fill_colour_button;		// Colour selection button

	GtkWidget			*border_colour_label;		// Label widget
	GtkWidget			*border_colour_button;		// Colour selection button

	GtkWidget			*border_width_label;		// Label widget
	GtkWidget			*border_width_button;		//

	GtkWidget			*opacity_label;				// Label widget
	GtkWidget			*opacity_button;			//

	GtkWidget			*visibility_checkbox;		// Visibility widget

	GtkWidget			*x_off_label_start;			// Label widget
	GtkWidget			*x_off_button_start;		//

	GtkWidget			*y_off_label_start;			// Label widget
	GtkWidget			*y_off_button_start;		//

	GtkWidget			*x_off_label_finish;		// Label widget
	GtkWidget			*x_off_button_finish;		//

	GtkWidget			*y_off_label_finish;		// Label widget
	GtkWidget			*y_off_button_finish;		//

	GtkWidget			*width_label;				// Label widget
	GtkWidget			*width_button;				//

	GtkWidget			*height_label;				// Label widget
	GtkWidget			*height_button;				//

	GtkWidget			*start_label;				// Label widget
	GtkWidget			*start_button;				//

	GtkWidget			*duration_label;			// Label widget
	GtkWidget			*duration_button;			//

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

	layer_highlight		*tmp_highlight_ob;			// Temporary highlight layer object


	// Initialise some things
	tmp_highlight_ob = (layer_highlight *) tmp_layer->object_data;
	valid_ext_link = g_string_new(NULL);
	valid_ext_link_win = g_string_new(NULL);
	valid_name = g_string_new(NULL);

	// * Pop open a dialog box asking the user for the details of the layer *

	// Create the dialog window, and table to hold its children
	highlight_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(dialog_title, GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL));
	highlight_table = gtk_table_new(3, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(highlight_dialog->vbox), GTK_WIDGET(highlight_table), FALSE, FALSE, 10);

	// Create the label asking for the layer name
	name_label = gtk_label_new("Layer Name: ");
	gtk_misc_set_alignment(GTK_MISC(name_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(name_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the new layer name
	name_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(name_entry), valid_fields[LAYER_NAME].max_value);
	gtk_entry_set_text(GTK_ENTRY(name_entry), tmp_layer->name->str);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(name_entry), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the background fill colour selection label
	fill_colour_label = gtk_label_new("Background fill colour: ");
	gtk_misc_set_alignment(GTK_MISC(fill_colour_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(fill_colour_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the background fill colour selection button
    fill_colour_button = gtk_color_button_new_with_color(&(tmp_highlight_ob->fill_colour));
    gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(fill_colour_button), TRUE);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(fill_colour_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the background line colour selection label
	border_colour_label = gtk_label_new("Background border colour: ");
	gtk_misc_set_alignment(GTK_MISC(border_colour_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(border_colour_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the background line colour selection button
    border_colour_button = gtk_color_button_new_with_color(&(tmp_highlight_ob->border_colour));
    gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(border_colour_button), TRUE);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(border_colour_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the background border width
	border_width_label = gtk_label_new("Background border width: ");
	gtk_misc_set_alignment(GTK_MISC(border_width_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(border_width_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the background border width input
	border_width_button = gtk_spin_button_new_with_range(valid_fields[LINE_WIDTH].min_value, valid_fields[LINE_WIDTH].max_value, 0.1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(border_width_button), tmp_highlight_ob->border_width);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(border_width_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the opacity
	opacity_label = gtk_label_new("Opacity: ");
	gtk_misc_set_alignment(GTK_MISC(opacity_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(opacity_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the opacity input
	opacity_button = gtk_spin_button_new_with_range(valid_fields[OPACITY].min_value, valid_fields[OPACITY].max_value, 1.0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(opacity_button), tmp_highlight_ob->opacity);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(opacity_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the starting X Offset
	x_off_label_start = gtk_label_new("Starting X Offset:");
	gtk_misc_set_alignment(GTK_MISC(x_off_label_start), 0, 0.5);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(x_off_label_start), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the starting X Offset input
	x_off_button_start = gtk_spin_button_new_with_range(0, project_width, 10);
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(x_off_button_start), TRUE);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(x_off_button_start), tmp_layer->x_offset_start);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(x_off_button_start), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the starting Y Offset
	y_off_label_start = gtk_label_new("Starting Y Offset:");
	gtk_misc_set_alignment(GTK_MISC(y_off_label_start), 0, 0.5);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(y_off_label_start), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the starting Y Offset input
	y_off_button_start = gtk_spin_button_new_with_range(0, project_height, 10);
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(y_off_button_start), TRUE);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(y_off_button_start), tmp_layer->y_offset_start);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(y_off_button_start), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the finishing X Offset
	x_off_label_finish = gtk_label_new("Finishing X Offset:");
	gtk_misc_set_alignment(GTK_MISC(x_off_label_finish), 0, 0.5);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(x_off_label_finish), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the finishing X Offset input
	x_off_button_finish = gtk_spin_button_new_with_range(0, project_width, 10);
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(x_off_button_finish), TRUE);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(x_off_button_finish), tmp_layer->x_offset_finish);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(x_off_button_finish), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the finishing Y Offset
	y_off_label_finish = gtk_label_new("Finishing Y Offset:");
	gtk_misc_set_alignment(GTK_MISC(y_off_label_finish), 0, 0.5);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(y_off_label_finish), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the finishing Y Offset input
	y_off_button_finish = gtk_spin_button_new_with_range(0, project_height, 10);
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(y_off_button_finish), TRUE);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(y_off_button_finish), tmp_layer->y_offset_finish);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(y_off_button_finish), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the width
	width_label = gtk_label_new("Width:");
	gtk_misc_set_alignment(GTK_MISC(width_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(width_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the width input
	width_button = gtk_spin_button_new_with_range(valid_fields[HIGHLIGHT_WIDTH].min_value, valid_fields[HIGHLIGHT_WIDTH].max_value, 10);
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(width_button), TRUE);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(width_button), tmp_highlight_ob->width);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(width_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the height
	height_label = gtk_label_new("Height:");
	gtk_misc_set_alignment(GTK_MISC(height_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(height_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the height input
	height_button = gtk_spin_button_new_with_range(valid_fields[HIGHLIGHT_HEIGHT].min_value, valid_fields[HIGHLIGHT_HEIGHT].max_value, 10);
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(y_off_button_finish), TRUE);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(height_button), tmp_highlight_ob->height);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(height_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the check box asking if the layer should be visible
	visibility_checkbox = gtk_check_button_new_with_label("Is this layer visible?");
	if (FALSE == tmp_layer->visible)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(visibility_checkbox), FALSE);
	} else
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(visibility_checkbox), TRUE);
	}
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(visibility_checkbox), 0, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the starting time
	start_label = gtk_label_new("Starting time (seconds): ");
	gtk_misc_set_alignment(GTK_MISC(start_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(start_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the starting time input
	start_button = gtk_spin_button_new_with_range(valid_fields[FRAME_NUMBER].min_value, valid_fields[FRAME_NUMBER].max_value, 0.1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(start_button), tmp_layer->start_time);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(start_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Appearance transition type
	label_trans_in_type = gtk_label_new("Start how: ");
	gtk_misc_set_alignment(GTK_MISC(label_trans_in_type), 0, 0.5);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(label_trans_in_type), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	selector_trans_in_type = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(selector_trans_in_type), "Immediate");
	gtk_combo_box_append_text(GTK_COMBO_BOX(selector_trans_in_type), "Fade in");
	switch (tmp_layer->transition_in_type)
	{
		case TRANS_LAYER_FADE:
			gtk_combo_box_set_active(GTK_COMBO_BOX(selector_trans_in_type), TRANS_LAYER_FADE);
			break;

		default:
			gtk_combo_box_set_active(GTK_COMBO_BOX(selector_trans_in_type), TRANS_LAYER_NONE);
	}
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(selector_trans_in_type), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Appearance transition duration label
	label_trans_in_duration = gtk_label_new("Start duration (seconds):");
	gtk_misc_set_alignment(GTK_MISC(label_trans_in_duration), 0, 0.5);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(label_trans_in_duration), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Appearance transition duration entry
	button_trans_in_duration = gtk_spin_button_new_with_range(valid_fields[TRANSITION_DURATION].min_value, valid_fields[TRANSITION_DURATION].max_value, 0.1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_trans_in_duration), tmp_layer->transition_in_duration);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(button_trans_in_duration), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the layer duration
	duration_label = gtk_label_new("Display for (seconds): ");
	gtk_misc_set_alignment(GTK_MISC(duration_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(duration_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the duration input
	duration_button = gtk_spin_button_new_with_range(valid_fields[FRAME_NUMBER].min_value, valid_fields[FRAME_NUMBER].max_value, 0.1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(duration_button), tmp_layer->duration);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(duration_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Exit Transition type
	label_trans_out_type = gtk_label_new("Exit how: ");
	gtk_misc_set_alignment(GTK_MISC(label_trans_out_type), 0, 0.5);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(label_trans_out_type), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	selector_trans_out_type = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(selector_trans_out_type), "Immediate");
	gtk_combo_box_append_text(GTK_COMBO_BOX(selector_trans_out_type), "Fade out");
	switch (tmp_layer->transition_out_type)
	{
		case TRANS_LAYER_FADE:
			gtk_combo_box_set_active(GTK_COMBO_BOX(selector_trans_out_type), TRANS_LAYER_FADE);
			break;

		default:
			gtk_combo_box_set_active(GTK_COMBO_BOX(selector_trans_out_type), TRANS_LAYER_NONE);
	}
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(selector_trans_out_type), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Exit transition duration label
	label_trans_out_duration = gtk_label_new("Exit duration (seconds):");
	gtk_misc_set_alignment(GTK_MISC(label_trans_out_duration), 0, 0.5);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(label_trans_out_duration), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Exit transition duration entry
	button_trans_out_duration = gtk_spin_button_new_with_range(valid_fields[TRANSITION_DURATION].min_value, valid_fields[TRANSITION_DURATION].max_value, 0.1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_trans_out_duration), tmp_layer->transition_out_duration);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(button_trans_out_duration), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for an external link
	external_link_label = gtk_label_new("External link: ");
	gtk_misc_set_alignment(GTK_MISC(external_link_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(external_link_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts an external link
	external_link_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(external_link_entry), valid_fields[EXTERNAL_LINK].max_value);
	gtk_entry_set_text(GTK_ENTRY(external_link_entry), tmp_layer->external_link->str);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(external_link_entry), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the window to open the external link in
	external_link_win_label = gtk_label_new("External link window: ");
	gtk_misc_set_alignment(GTK_MISC(external_link_win_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(external_link_win_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts a text string for the window to open the external link in
	external_link_win_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(external_link_win_entry), valid_fields[EXTERNAL_LINK_WINDOW].max_value);
	gtk_entry_set_text(GTK_ENTRY(external_link_win_entry), tmp_layer->external_link_window->str);
	gtk_table_attach(GTK_TABLE(highlight_table), GTK_WIDGET(external_link_win_entry), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Ensure everything will show
	gtk_widget_show_all(GTK_WIDGET(highlight_dialog));

	// Loop around until we have all valid values, or the user cancels out
	validated_string = NULL;
	do
	{
		// Display the dialog
		if (GTK_RESPONSE_ACCEPT != gtk_dialog_run(GTK_DIALOG(highlight_dialog)))
		{
			// The dialog was cancelled, so destroy it and return to the caller
			gtk_widget_destroy(GTK_WIDGET(highlight_dialog));
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
			display_warning("Error ED382: There was something wrong with the new layer name.  Please try again.");
			useable_input = FALSE;
		} else
		{
			g_string_assign(valid_name, validated_string->str);
			g_string_free(validated_string, TRUE);
			validated_string = NULL;
		}

		// Retrieve the new border width
		gfloat_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(border_width_button));
		validated_gfloat = validate_value(LINE_WIDTH, V_FLOAT_UNSIGNED, &gfloat_val);
		if (NULL == validated_gfloat)
		{
			display_warning("Error ED396: There was something wrong with the border width value.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_border_width = *validated_gfloat;
			g_free(validated_gfloat);
		}

		// Retrieve the new opacity value
		gfloat_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(opacity_button));
		validated_gfloat = validate_value(OPACITY, V_FLOAT_UNSIGNED, &gfloat_val);
		if (NULL == validated_gfloat)
		{
			display_warning("Error ED405: There was something wrong with the opacity value.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_opacity = *validated_gfloat;
			g_free(validated_gfloat);
		}

		// Retrieve the new starting frame x offset
		guint_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(x_off_button_start));
		validated_guint = validate_value(X_OFFSET, V_INT_UNSIGNED, &guint_val);
		if (NULL == validated_guint)
		{
			display_warning("Error ED144: There was something wrong with the starting frame X offset value.  Please try again.");
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
			display_warning("Error ED145: There was something wrong with the starting frame Y offset value.  Please try again.");
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
			display_warning("Error ED146: There was something wrong with the finish frame X offset value.  Please try again.");
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
			display_warning("Error ED147: There was something wrong with the finish frame X offset value.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_y_offset_finish = *validated_guint;
			g_free(validated_guint);
		}

		// Retrieve the new highlight width
		guint_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(width_button));
		validated_guint = validate_value(HIGHLIGHT_WIDTH, V_INT_UNSIGNED, &guint_val);
		if (NULL == validated_guint)
		{
			display_warning("Error ED148: There was something wrong with the highlight width value.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_width = *validated_guint;
			g_free(validated_guint);
		}

		// Retrieve the new highlight height
		guint_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(height_button));
		validated_guint = validate_value(HIGHLIGHT_HEIGHT, V_INT_UNSIGNED, &guint_val);
		if (NULL == validated_guint)
		{
			display_warning("Error ED149: There was something wrong with the highlight height value.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_height = *validated_guint;
			g_free(validated_guint);
		}

		// Retrieve the new starting time
		gfloat_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(start_button));
		validated_gfloat = validate_value(LAYER_DURATION, V_FLOAT_UNSIGNED, &gfloat_val);
		if (NULL == validated_gfloat)
		{
			display_warning("Error ED150: There was something wrong with the starting time value.  Please try again.");
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
			display_warning("Error ED151: There was something wrong with the duration value.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_duration = *validated_gfloat;
			g_free(validated_gfloat);
		}

		// Retrieve the external link input
		validated_string = validate_value(EXTERNAL_LINK, V_CHAR, (gchar *) gtk_entry_get_text(GTK_ENTRY(external_link_entry)));
		if (NULL == validated_string)
		{
			display_warning("Error ED152: There was something wrong with the external link value.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_ext_link = g_string_assign(valid_ext_link, validated_string->str);
			g_string_free(validated_string, TRUE);
			validated_string = NULL;
		}

		// Retrieve the external link window input
		validated_string = validate_value(EXTERNAL_LINK_WINDOW, V_CHAR, (gchar *) gtk_entry_get_text(GTK_ENTRY(external_link_win_entry)));
		if (NULL == validated_string)
		{
			display_warning("Error ED153: There was something wrong with the external link target window value.  Please try again.");
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
			display_warning("Error ED297: There was something wrong with the appearance transition type selected.  Please try again.");
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
			display_warning("Error ED298: There was something wrong with the appearance transition duration value.  Please try again.");
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
			display_warning("Error ED299: There was something wrong with the exit transition type selected.  Please try again.");
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
			display_warning("Error ED300: There was something wrong with the exit transition duration value.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_trans_out_duration = *validated_gfloat;
			g_free(validated_gfloat);
		}
	} while (FALSE == useable_input);

	// * We only get here after all input is considered valid *

	// Fill out the temporary layer with the requested details
	g_string_printf(tmp_layer->name, "%s", valid_name->str);
	tmp_layer->x_offset_start = valid_x_offset_start;
	tmp_layer->y_offset_start = valid_y_offset_start;
	tmp_layer->x_offset_finish = valid_x_offset_finish;
	tmp_layer->y_offset_finish = valid_y_offset_finish;
	tmp_highlight_ob->width = valid_width;
	tmp_highlight_ob->height = valid_height;
	if (TRUE == gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(visibility_checkbox)))
	{
		tmp_layer->visible = TRUE;
	} else
	{
		tmp_layer->visible = FALSE;
	}
	tmp_layer->start_time = valid_start_time;
	tmp_layer->duration = valid_duration;
	g_string_printf(tmp_layer->external_link, "%s", valid_ext_link->str);
	g_string_printf(tmp_layer->external_link_window, "%s", valid_ext_link_win->str);
	tmp_layer->transition_in_type = valid_trans_in_type;
	tmp_layer->transition_in_duration = valid_trans_in_duration;
	tmp_layer->transition_out_type = valid_trans_out_type;
	tmp_layer->transition_out_duration = valid_trans_out_duration;
	gtk_color_button_get_color(GTK_COLOR_BUTTON(border_colour_button), &(tmp_highlight_ob->border_colour));
	gtk_color_button_get_color(GTK_COLOR_BUTTON(fill_colour_button), &(tmp_highlight_ob->fill_colour));
	tmp_highlight_ob->border_width = valid_border_width;
	tmp_highlight_ob->opacity = valid_opacity;

	// Destroy the dialog box
	gtk_widget_destroy(GTK_WIDGET(highlight_dialog));

	// Free the memory allocated in this function
	g_string_free(valid_name, TRUE);
	g_string_free(valid_ext_link, TRUE);
	g_string_free(valid_ext_link_win, TRUE);
	return TRUE;
}
