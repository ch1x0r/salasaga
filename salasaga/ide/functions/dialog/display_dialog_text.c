/*
 * $Id$
 *
 * Salasaga: Display a dialog box asking for text layer settings 
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


gboolean display_dialog_text(layer *tmp_layer, gchar *dialog_title)
{
	// Local variables
	gfloat				gfloat_val;					// Temporary gfloat value used for validation
	gint				gint_val;					// Temporary gint value
	guint				guint_val;					// Temporary guint value used for validation
	GString				*message;					// Used to construct message strings
	layer_text			*tmp_text_ob;				// Temporary text layer object
	gboolean			useable_input;				// Used as a flag to indicate if all validation was successful
	gfloat				valid_border_width = 0;		// Receives the new border width once validated
	gfloat				valid_duration = 0;			// Receives the new finish frame once validated
	GString				*valid_ext_link;			// Receives the new external link once validated
	GString				*valid_ext_link_win;		// Receives the new external link window once validated
	gfloat				valid_font_size = 0;		// Receives the new font size once validated
	GString				*valid_name;				// Receives the new layer name once validated
	gfloat				valid_start_time = 0;		// Receives the new start time once validated
	gfloat				valid_trans_in_duration = 0;// Receives the new appearance transition duration once validated
	guint				valid_trans_in_type = 0;	// Receives the new appearance transition type once validated
	gfloat				valid_trans_out_duration;	// Receives the new exit transition duration once validated
	guint				valid_trans_out_type = 0;	// Receives the new exit transition type once validated
	guint				valid_x_offset_finish = 0;	// Receives the new finish frame x offset once validated
	guint				valid_x_offset_start = 0;	// Receives the new start frame x offset once validated
	guint				valid_y_offset_finish = 0;	// Receives the new finish frame y offset once validated
	guint				valid_y_offset_start = 0;	// Receives the new start frame y offset once validated
	gfloat				*validated_gfloat;			// Receives known good gfloat values from the validation function
	guint				*validated_guint;			// Receives known good guint values from the validation function
	GString				*validated_string;			// Receives known good strings from the validation function

	// * Dialog widgets *
	guint				row_counter = 0;			// Used to count which row things are up to
	GtkWidget			*appearance_tab_label;		//
	GtkWidget			*appearance_table;			//
	GtkWidget			*duration_tab_label;		//
	GtkWidget			*duration_table;			// 
	GtkWidget			*notebook_widget;			//
	GtkDialog			*text_dialog;				// Widget for the text dialog

	// * Appearance & Links tab fields *

	GtkTextBuffer		*text_buffer;				// Temporary text buffer the user words with 
	GtkTextIter			text_end;					// End position of text buffer
	GtkWidget			*text_frame;				// Frame to go around the text widget
	GString				*text_gstring;				// Temporary text buffer
	GtkTextIter			text_start;					// Start position of text buffer
	GtkWidget			*text_view;					// Widget for accepting the new text data

	GtkWidget			*name_label;				// Label widget
	GtkWidget			*name_entry;				//

	GtkWidget			*font_label;				// Label widget
	GtkWidget			*font_button;				//

	GtkWidget			*fg_colour_label;			// Label widget
	GtkWidget			*fg_colour_button;			// Forground colour selection button

	GtkWidget			*fill_colour_label;			// Label widget
	GtkWidget			*fill_colour_button;		// Colour selection button

	GtkWidget			*border_colour_label;		// Label widget
	GtkWidget			*border_colour_button;		// Colour selection button

	GtkWidget			*border_width_label;		// Label widget
	GtkWidget			*border_width_button;		//

	GtkWidget			*external_link_label;		// Label widget
	GtkWidget			*external_link_entry;		// Widget for accepting an external link for clicking on

	GtkWidget			*external_link_win_label;	// Label widget
	GtkWidget			*external_link_win_entry;	//

	GtkWidget			*display_bg_checkbox;		// Background visibility check box widget

	GtkWidget			*visibility_checkbox;		// Layer visibility check box widget

	// * Duration tab fields *

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

	GtkWidget			*label_trans_in_type;		// Transition in type
	GtkWidget			*selector_trans_in_type;	//

	GtkWidget			*label_trans_in_duration;	// Transition in duration (seconds)
	GtkWidget			*button_trans_in_duration;	//

	GtkWidget			*label_trans_out_type;		// Transition out type
	GtkWidget			*selector_trans_out_type;	//

	GtkWidget			*label_trans_out_duration;	// Transition out duration (seconds)
	GtkWidget			*button_trans_out_duration;	//


	// Initialise some things
	tmp_text_ob = (layer_text *) tmp_layer->object_data;
	message = g_string_new(NULL);
	valid_ext_link = g_string_new(NULL);
	valid_ext_link_win = g_string_new(NULL);
	valid_name = g_string_new(NULL);

	// * Open a dialog box asking the user for the details of the new text layer *

	// Create the dialog
	text_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(dialog_title, GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL));
	notebook_widget = gtk_notebook_new();
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook_widget), GTK_POS_TOP);
	appearance_table = gtk_table_new(8, 2, FALSE);
	appearance_tab_label = gtk_label_new(_("Appearance & Links"));
	duration_table = gtk_table_new(8, 2, FALSE);
	duration_tab_label = gtk_label_new(_("Duration"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook_widget), appearance_table, appearance_tab_label);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook_widget), duration_table, duration_tab_label);
	gtk_box_pack_start(GTK_BOX(text_dialog->vbox), GTK_WIDGET(notebook_widget), FALSE, FALSE, 0);

	// * Appearance & Links tab fields *

	// Create the text view that accepts the new text data
	text_frame = gtk_frame_new(NULL);
	gtk_container_set_border_width(GTK_CONTAINER(text_frame), 2);
	gtk_frame_set_shadow_type(GTK_FRAME(text_frame), GTK_SHADOW_OUT);
	text_view = gtk_text_view_new();
	gtk_widget_set_size_request(GTK_WIDGET(text_view), 0, 100);
	gtk_container_add(GTK_CONTAINER(text_frame), text_view);
	gtk_table_attach(GTK_TABLE(appearance_table), GTK_WIDGET(text_frame), 0, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Copy the text string from the existing text buffer to a new, temporary one
	// (Note - this is so we don't work directly with the text buffer, which would keep edits even if the user hits the Cancel button)
	gtk_text_buffer_get_start_iter(tmp_text_ob->text_buffer, &text_start);
	gtk_text_buffer_get_end_iter(tmp_text_ob->text_buffer, &text_end);
	text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
	text_gstring = g_string_new(gtk_text_buffer_get_slice(tmp_text_ob->text_buffer, &text_start, &text_end, TRUE));
	gtk_text_buffer_set_text(GTK_TEXT_BUFFER(text_buffer), text_gstring->str, text_gstring->len);

	// Create the label asking for the layer name
	name_label = gtk_label_new(_("Layer name: "));
	gtk_misc_set_alignment(GTK_MISC(name_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(appearance_table), GTK_WIDGET(name_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the new layer name
	name_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(name_entry), valid_fields[LAYER_NAME].max_value);
	gtk_entry_set_text(GTK_ENTRY(name_entry), tmp_layer->name->str);
	gtk_table_attach(GTK_TABLE(appearance_table), GTK_WIDGET(name_entry), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the font size
	font_label = gtk_label_new(_("Font size: "));
	gtk_misc_set_alignment(GTK_MISC(font_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(appearance_table), GTK_WIDGET(font_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the font size input
	font_button = gtk_spin_button_new_with_range(valid_fields[FONT_SIZE].min_value, valid_fields[FONT_SIZE].max_value, 1);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(font_button), 2);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(font_button), tmp_text_ob->font_size);
	gtk_table_attach(GTK_TABLE(appearance_table), GTK_WIDGET(font_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the foreground colour selection label
	fg_colour_label = gtk_label_new(_("Text color: "));
	gtk_misc_set_alignment(GTK_MISC(fg_colour_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(appearance_table), GTK_WIDGET(fg_colour_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the foreground colour selection button
    fg_colour_button = gtk_color_button_new_with_color(&(tmp_text_ob->text_color));
    gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(fg_colour_button), TRUE);
	gtk_table_attach(GTK_TABLE(appearance_table), GTK_WIDGET(fg_colour_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the background fill colour selection label
	fill_colour_label = gtk_label_new(_("Background fill color: "));
	gtk_misc_set_alignment(GTK_MISC(fill_colour_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(appearance_table), GTK_WIDGET(fill_colour_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the background fill colour selection button
    fill_colour_button = gtk_color_button_new_with_color(&(tmp_text_ob->bg_fill_colour));
    gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(fill_colour_button), TRUE);
	gtk_table_attach(GTK_TABLE(appearance_table), GTK_WIDGET(fill_colour_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the background line colour selection label
	border_colour_label = gtk_label_new(_("Background border color: "));
	gtk_misc_set_alignment(GTK_MISC(border_colour_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(appearance_table), GTK_WIDGET(border_colour_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the background line colour selection button
    border_colour_button = gtk_color_button_new_with_color(&(tmp_text_ob->bg_border_colour));
    gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(border_colour_button), TRUE);
	gtk_table_attach(GTK_TABLE(appearance_table), GTK_WIDGET(border_colour_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the background border width
	border_width_label = gtk_label_new(_("Background border width: "));
	gtk_misc_set_alignment(GTK_MISC(border_width_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(appearance_table), GTK_WIDGET(border_width_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the background border width input
	border_width_button = gtk_spin_button_new_with_range(valid_fields[LINE_WIDTH].min_value, valid_fields[LINE_WIDTH].max_value, 0.1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(border_width_button), tmp_text_ob->bg_border_width);
	gtk_table_attach(GTK_TABLE(appearance_table), GTK_WIDGET(border_width_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for an external link
	external_link_label = gtk_label_new(_("External link: "));
	gtk_misc_set_alignment(GTK_MISC(external_link_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(appearance_table), GTK_WIDGET(external_link_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts an external link
	external_link_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(external_link_entry), valid_fields[EXTERNAL_LINK].max_value);
	gtk_entry_set_text(GTK_ENTRY(external_link_entry), tmp_layer->external_link->str);
	gtk_table_attach(GTK_TABLE(appearance_table), GTK_WIDGET(external_link_entry), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the window to open the external link in
	external_link_win_label = gtk_label_new(_("External link window: "));
	gtk_misc_set_alignment(GTK_MISC(external_link_win_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(appearance_table), GTK_WIDGET(external_link_win_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts a text string for the window to open the external link in
	external_link_win_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(external_link_win_entry), valid_fields[EXTERNAL_LINK_WINDOW].max_value);
	gtk_entry_set_text(GTK_ENTRY(external_link_win_entry), tmp_layer->external_link_window->str);
	gtk_table_attach(GTK_TABLE(appearance_table), GTK_WIDGET(external_link_win_entry), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the check box asking if the background should be visible
	display_bg_checkbox = gtk_check_button_new_with_label(_("Display the background?"));
	if (FALSE == tmp_text_ob->show_bg)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(display_bg_checkbox), FALSE);
	} else
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(display_bg_checkbox), TRUE);
	}
	gtk_table_attach(GTK_TABLE(appearance_table), GTK_WIDGET(display_bg_checkbox), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the check box asking if the layer should be visible
	visibility_checkbox = gtk_check_button_new_with_label(_("Is this layer visible?"));
	if (FALSE == tmp_layer->visible)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(visibility_checkbox), FALSE);
	} else
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(visibility_checkbox), TRUE);
	}
	gtk_table_attach(GTK_TABLE(appearance_table), GTK_WIDGET(visibility_checkbox), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// * Duration tab fields *

	// Reset the row counter
	row_counter = 0;

	// Create the label asking for the starting X Offset
	x_off_label_start = gtk_label_new(_("Start X Offset: "));
	gtk_misc_set_alignment(GTK_MISC(x_off_label_start), 0, 0.5);
	gtk_table_attach(GTK_TABLE(duration_table), GTK_WIDGET(x_off_label_start), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the starting X Offset input
	x_off_button_start = gtk_spin_button_new_with_range(0, project_width, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(x_off_button_start), tmp_layer->x_offset_start);
	gtk_table_attach(GTK_TABLE(duration_table), GTK_WIDGET(x_off_button_start), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the starting Y Offset
	y_off_label_start = gtk_label_new(_("Start Y Offset: "));
	gtk_misc_set_alignment(GTK_MISC(y_off_label_start), 0, 0.5);
	gtk_table_attach(GTK_TABLE(duration_table), GTK_WIDGET(y_off_label_start), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the starting Y Offset input
	y_off_button_start = gtk_spin_button_new_with_range(0, project_height, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(y_off_button_start), tmp_layer->y_offset_start);
	gtk_table_attach(GTK_TABLE(duration_table), GTK_WIDGET(y_off_button_start), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the finishing X Offset
	x_off_label_finish = gtk_label_new(_("Finish X Offset: "));
	gtk_misc_set_alignment(GTK_MISC(x_off_label_finish), 0, 0.5);
	gtk_table_attach(GTK_TABLE(duration_table), GTK_WIDGET(x_off_label_finish), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the finishing X Offset input
	x_off_button_finish = gtk_spin_button_new_with_range(0, project_width, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(x_off_button_finish), tmp_layer->x_offset_finish);
	gtk_table_attach(GTK_TABLE(duration_table), GTK_WIDGET(x_off_button_finish), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the finishing Y Offset
	y_off_label_finish = gtk_label_new(_("Finish Y Offset: "));
	gtk_misc_set_alignment(GTK_MISC(y_off_label_finish), 0, 0.5);
	gtk_table_attach(GTK_TABLE(duration_table), GTK_WIDGET(y_off_label_finish), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the finishing Y Offset input
	y_off_button_finish = gtk_spin_button_new_with_range(0, project_height, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(y_off_button_finish), tmp_layer->y_offset_finish);
	gtk_table_attach(GTK_TABLE(duration_table), GTK_WIDGET(y_off_button_finish), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the starting time
	start_label = gtk_label_new(_("Starting time (seconds): "));
	gtk_misc_set_alignment(GTK_MISC(start_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(duration_table), GTK_WIDGET(start_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the starting time input
	start_button = gtk_spin_button_new_with_range(valid_fields[FRAME_NUMBER].min_value, valid_fields[FRAME_NUMBER].max_value, 0.1);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(start_button), 2);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(start_button), tmp_layer->start_time);
	gtk_table_attach(GTK_TABLE(duration_table), GTK_WIDGET(start_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Appearance transition type
	label_trans_in_type = gtk_label_new(_("Start how: "));
	gtk_misc_set_alignment(GTK_MISC(label_trans_in_type), 0, 0.5);
	gtk_table_attach(GTK_TABLE(duration_table), GTK_WIDGET(label_trans_in_type), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
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
	gtk_table_attach(GTK_TABLE(duration_table), GTK_WIDGET(selector_trans_in_type), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Appearance transition duration label
	label_trans_in_duration = gtk_label_new(_("Start duration (seconds):"));
	gtk_misc_set_alignment(GTK_MISC(label_trans_in_duration), 0, 0.5);
	gtk_table_attach(GTK_TABLE(duration_table), GTK_WIDGET(label_trans_in_duration), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Appearance transition duration entry
	button_trans_in_duration = gtk_spin_button_new_with_range(valid_fields[TRANSITION_DURATION].min_value, valid_fields[TRANSITION_DURATION].max_value, 0.1);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(button_trans_in_duration), 2);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_trans_in_duration), tmp_layer->transition_in_duration);
	gtk_table_attach(GTK_TABLE(duration_table), GTK_WIDGET(button_trans_in_duration), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the layer duration
	duration_label = gtk_label_new(_("Display for (seconds): "));
	gtk_misc_set_alignment(GTK_MISC(duration_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(duration_table), GTK_WIDGET(duration_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the duration input
	duration_button = gtk_spin_button_new_with_range(valid_fields[FRAME_NUMBER].min_value, valid_fields[FRAME_NUMBER].max_value, 0.1);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(duration_button), 2);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(duration_button), tmp_layer->duration);
	gtk_table_attach(GTK_TABLE(duration_table), GTK_WIDGET(duration_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Exit Transition type
	label_trans_out_type = gtk_label_new(_("Exit how: "));
	gtk_misc_set_alignment(GTK_MISC(label_trans_out_type), 0, 0.5);
	gtk_table_attach(GTK_TABLE(duration_table), GTK_WIDGET(label_trans_out_type), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
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
	gtk_table_attach(GTK_TABLE(duration_table), GTK_WIDGET(selector_trans_out_type), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Exit transition duration label
	label_trans_out_duration = gtk_label_new(_("Exit duration (seconds):"));
	gtk_misc_set_alignment(GTK_MISC(label_trans_out_duration), 0, 0.5);
	gtk_table_attach(GTK_TABLE(duration_table), GTK_WIDGET(label_trans_out_duration), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Exit transition duration entry
	button_trans_out_duration = gtk_spin_button_new_with_range(valid_fields[TRANSITION_DURATION].min_value, valid_fields[TRANSITION_DURATION].max_value, 0.1);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(button_trans_out_duration), 2);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_trans_out_duration), tmp_layer->transition_out_duration);
	gtk_table_attach(GTK_TABLE(duration_table), GTK_WIDGET(button_trans_out_duration), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Ensure everything will show
	gtk_widget_show_all(GTK_WIDGET(text_dialog));

	// Loop around until we have all valid values, or the user cancels out
	validated_string = NULL;
	do
	{
		// Display the dialog
		if (GTK_RESPONSE_ACCEPT != gtk_dialog_run(GTK_DIALOG(text_dialog)))
		{
			// The dialog was cancelled, so destroy it and return to the caller
			gtk_widget_destroy(GTK_WIDGET(text_dialog));
			g_string_free(valid_ext_link, TRUE);
			g_string_free(valid_ext_link_win, TRUE);
			g_string_free(valid_name, TRUE);
			return FALSE;
		}

		// Reset the useable input flag
		useable_input = TRUE;

		// Validate the layer name input
		validated_string = validate_value(LAYER_NAME, V_CHAR, (gchar *) gtk_entry_get_text(GTK_ENTRY(name_entry)));
		if (NULL == validated_string)
		{
			g_string_printf(message, "%s ED177: %s", _("Error"), _("There was something wrong with the layer name value.  Please try again."));
			display_warning(message->str);
			useable_input = FALSE;
		} else
		{
			valid_name = g_string_assign(valid_name, validated_string->str);
			g_string_free(validated_string, TRUE);
			validated_string = NULL;
		}

		// Retrieve the new starting frame x offset
		guint_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(x_off_button_start));
		validated_guint = validate_value(X_OFFSET, V_INT_UNSIGNED, &guint_val);
		if (NULL == validated_guint)
		{
			g_string_printf(message, "%s ED172: %s", _("Error"), _("There was something wrong with the starting frame X offset value.  Please try again."));
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
			g_string_printf(message, "%s ED173: %s", _("Error"), _("There was something wrong with the starting frame Y offset value.  Please try again."));
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
			g_string_printf(message, "%s ED174: %s", _("Error"), _("There was something wrong with the finish frame X offset value.  Please try again."));
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
			g_string_printf(message, "%s ED175: %s", _("Error"), _("There was something wrong with the finish frame X offset value.  Please try again."));
			display_warning(message->str);
			useable_input = FALSE;
		} else
		{
			valid_y_offset_finish = *validated_guint;
			g_free(validated_guint);
		}

		// Retrieve the new font size
		gfloat_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(font_button));
		validated_gfloat = validate_value(FONT_SIZE, V_FLOAT_UNSIGNED, &gfloat_val);
		if (NULL == validated_gfloat)
		{
			g_string_printf(message, "%s ED176: %s", _("Error"), _("There was something wrong with the font size value.  Please try again."));
			display_warning(message->str);
			useable_input = FALSE;
		} else
		{
			valid_font_size = *validated_gfloat;
			g_free(validated_gfloat);
		}

		// Retrieve the new background border width
		gfloat_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(border_width_button));
		validated_gfloat = validate_value(LINE_WIDTH, V_FLOAT_UNSIGNED, &gfloat_val);
		if (NULL == validated_gfloat)
		{
			g_string_printf(message, "%s ED387: %s", _("Error"), _("There was something wrong with the background border width value.  Please try again."));
			display_warning(message->str);
			useable_input = FALSE;
		} else
		{
			valid_border_width = *validated_gfloat;
			g_free(validated_gfloat);
		}

		// Retrieve the new starting time
		gfloat_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(start_button));
		validated_gfloat = validate_value(LAYER_DURATION, V_FLOAT_UNSIGNED, &gfloat_val);
		if (NULL == validated_gfloat)
		{
			g_string_printf(message, "%s ED178: %s", _("Error"), _("There was something wrong with the starting time value.  Please try again."));
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
			g_string_printf(message, "%s ED179: %s", _("Error"), _("There was something wrong with the duration value.  Please try again."));
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
			g_string_printf(message, "%s ED180: %s", _("Error"), _("There was something wrong with the external link value.  Please try again."));
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
			g_string_printf(message, "%s ED181: %s", _("Error"), _("There was something wrong with the external link target window value.  Please try again."));
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
			g_string_printf(message, "%s ED309: %s", _("Error"), _("There was something wrong with the appearance transition type selected.  Please try again."));
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
			g_string_printf(message, "%s ED310: %s", _("Error"), _("There was something wrong with the appearance transition duration value.  Please try again."));
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
			g_string_printf(message, "%s ED311: %s", _("Error"), _("There was something wrong with the exit transition type selected.  Please try again."));
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
			g_string_printf(message, "%s ED312: %s", _("Error"), _("There was something wrong with the exit transition duration value.  Please try again."));
			display_warning(message->str);
			useable_input = FALSE;
		} else
		{
			valid_trans_out_duration = *validated_gfloat;
			g_free(validated_gfloat);
		}
	} while (FALSE == useable_input);

	// * We only get here after all input is considered valid *

	// Fill out the temporary layer with the requested details
	tmp_layer->x_offset_start = valid_x_offset_start;
	tmp_layer->y_offset_start = valid_y_offset_start;
	tmp_layer->x_offset_finish = valid_x_offset_finish;
	tmp_layer->y_offset_finish = valid_y_offset_finish;
	tmp_text_ob->font_size = valid_font_size;
	tmp_layer->start_time = valid_start_time;
	tmp_layer->duration = valid_duration;
	if (TRUE == gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(display_bg_checkbox)))
	{
		tmp_text_ob->show_bg = TRUE;
	} else
	{
		tmp_text_ob->show_bg = FALSE;
	}

	if (TRUE == gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(visibility_checkbox)))
	{
		tmp_layer->visible = TRUE;
	} else
	{
		tmp_layer->visible = FALSE;
	}
	g_string_printf(tmp_layer->name, "%s", valid_name->str);
	g_string_printf(tmp_layer->external_link, "%s", valid_ext_link->str);
	g_string_printf(tmp_layer->external_link_window, "%s", valid_ext_link_win->str);
	tmp_layer->transition_in_type = valid_trans_in_type;
	tmp_layer->transition_in_duration = valid_trans_in_duration;
	tmp_layer->transition_out_type = valid_trans_out_type;
	tmp_layer->transition_out_duration = valid_trans_out_duration;
	gtk_color_button_get_color(GTK_COLOR_BUTTON(fg_colour_button), &(tmp_text_ob->text_color));
	gtk_color_button_get_color(GTK_COLOR_BUTTON(border_colour_button), &(tmp_text_ob->bg_border_colour));
	gtk_color_button_get_color(GTK_COLOR_BUTTON(fill_colour_button), &(tmp_text_ob->bg_fill_colour));
	tmp_text_ob->bg_border_width = valid_border_width;

	// Copy the text buffer from the onscreen widget to our existing text buffer
	gtk_text_buffer_get_start_iter(text_buffer, &text_start);
	gtk_text_buffer_get_end_iter(text_buffer, &text_end);
	text_gstring = g_string_new(gtk_text_buffer_get_slice(text_buffer, &text_start, &text_end, TRUE));
	gtk_text_buffer_set_text(GTK_TEXT_BUFFER(tmp_text_ob->text_buffer), text_gstring->str, text_gstring->len);

	// Destroy the dialog box
	gtk_widget_destroy(GTK_WIDGET(text_dialog));

	// Free the memory allocated in this function
	g_string_free(message, TRUE);
	g_string_free(text_gstring, TRUE);
	g_string_free(valid_ext_link, TRUE);
	g_string_free(valid_ext_link_win, TRUE);
	g_string_free(valid_name, TRUE);

	return TRUE;
}