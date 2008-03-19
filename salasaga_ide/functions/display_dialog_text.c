/*
 * $Id$
 *
 * Salasaga: Display a dialog box asking for text layer settings 
 * 
 * Copyright (C) 2007-2008 Justin Clift <justin@salasaga.org>
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

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"
#include "display_warning.h"
#include "validate_value.h"


gboolean display_dialog_text(layer *tmp_layer, gchar *dialog_title)
{
	// Local variables
	gfloat				gfloat_val;					// Temporary gfloat value used for validation	
	guint				guint_val;					// Temporary guint value used for validation
	guint				row_counter = 0;			// Used to count which row things are up to
	GtkDialog			*text_dialog;				// Widget for the text dialog
	GtkWidget			*text_table;				// Table used for neat layout of the dialog box
	gboolean			useable_input;				// Used as a flag to indicate if all validation was successful
	GString				*valid_ext_link;			// Receives the new external link once validated
	GString				*valid_ext_link_win;		// Receives the new external link window once validated
	GString				*valid_layer_name;			// Receives the new layer name once validated
	guint				valid_finish_frame;			// Receives the new finish frame once validated
	gfloat				valid_font_size;			// Receives the new font size once validated
	guint				valid_start_frame;			// Receives the new start frame once validated
	guint				valid_x_offset_finish;		// Receives the new finish frame x offset once validated
	guint				valid_x_offset_start;		// Receives the new start frame x offset once validated
	guint				valid_y_offset_finish;		// Receives the new finish frame y offset once validated
	guint				valid_y_offset_start;		// Receives the new start frame y offset once validated
	gfloat				*validated_gfloat;			// Receives known good gfloat values from the validation function
	guint				*validated_guint;			// Receives known good guint values from the validation function
	GString				*validated_string;			// Receives known good strings from the validation function

	GtkTextBuffer		*text_buffer;				// Temporary text buffer the user words with 
	GtkTextIter			text_end;					// End position of text buffer
	GtkWidget			*text_frame;				// Frame to go around the text widget
	GString				*text_gstring;				// Temporary text buffer
	GtkTextIter			text_start;					// Start position of text buffer
	GtkWidget			*text_view;					// Widget for accepting the new text data

	GtkWidget			*name_label;				// Label widget
	GtkWidget			*name_entry;				// Widget for accepting the name of the new layer

	GtkWidget			*color_label;				// Label widget
	GtkWidget			*color_button;				// Color selection button

	GtkWidget			*x_off_label_start;			// Label widget
	GtkWidget			*x_off_button_start;		//

	GtkWidget			*y_off_label_start;			// Label widget
	GtkWidget			*y_off_button_start;		//

	GtkWidget			*x_off_label_finish;		// Label widget
	GtkWidget			*x_off_button_finish;		//

	GtkWidget			*y_off_label_finish;		// Label widget
	GtkWidget			*y_off_button_finish;		//

	GtkWidget			*font_label;				// Label widget
	GtkWidget			*font_button;				//

	GtkWidget			*start_label;				// Label widget
	GtkWidget			*start_button;				//

	GtkWidget			*finish_label;				// Label widget
	GtkWidget			*finish_button;				//

	GtkWidget			*external_link_label;		// Label widget
	GtkWidget			*external_link_entry;		// Widget for accepting an external link for clicking on

	GtkWidget			*external_link_win_label;	// Label widget
	GtkWidget			*external_link_win_entry;	//

	layer_text			*tmp_text_ob;				// Temporary text layer object


	// Initialise some things
	tmp_text_ob = (layer_text *) tmp_layer->object_data;
	valid_ext_link = g_string_new(NULL);
	valid_ext_link_win = g_string_new(NULL);
	valid_layer_name = g_string_new(NULL);

	// * Open a dialog box asking the user for the details of the new text layer *

	// Create the dialog window, and table to hold its children
	text_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(dialog_title, GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL));
	text_table = gtk_table_new(8, 2, FALSE);
	gtk_box_pack_start(GTK_BOX(text_dialog->vbox), GTK_WIDGET(text_table), FALSE, FALSE, 0);

	// Create the text view that accepts the new text data
	text_frame = gtk_frame_new(NULL);
	gtk_container_set_border_width(GTK_CONTAINER(text_frame), 2);
	gtk_frame_set_shadow_type(GTK_FRAME(text_frame), GTK_SHADOW_OUT);
	text_view = gtk_text_view_new();
	gtk_widget_set_size_request(GTK_WIDGET(text_view), 0, 100);
	gtk_container_add(GTK_CONTAINER(text_frame), text_view);
	gtk_table_attach(GTK_TABLE(text_table), GTK_WIDGET(text_frame), 0, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Copy the text string from the existing text buffer to a new, temporary one
	// (Note - this is so we don't work directly with the text buffer, which would keep edits even if the user hits the Cancel button)
	gtk_text_buffer_get_start_iter(tmp_text_ob->text_buffer, &text_start);
	gtk_text_buffer_get_end_iter(tmp_text_ob->text_buffer, &text_end);
	text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
	text_gstring = g_string_new(gtk_text_buffer_get_slice(tmp_text_ob->text_buffer, &text_start, &text_end, TRUE));
	gtk_text_buffer_set_text(GTK_TEXT_BUFFER(text_buffer), text_gstring->str, text_gstring->len);

	// Create the label next to the color swatch
	color_label = gtk_label_new("Color: ");
	gtk_misc_set_alignment(GTK_MISC(color_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(text_table), GTK_WIDGET(color_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the color selection button
    color_button = gtk_color_button_new_with_color(&(tmp_text_ob->text_color));
    gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(color_button), TRUE);
	gtk_table_attach(GTK_TABLE(text_table), GTK_WIDGET(color_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the starting X Offset
	x_off_label_start = gtk_label_new("Start X Offset: ");
	gtk_misc_set_alignment(GTK_MISC(x_off_label_start), 0, 0.5);
	gtk_table_attach(GTK_TABLE(text_table), GTK_WIDGET(x_off_label_start), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the starting X Offset input
	x_off_button_start = gtk_spin_button_new_with_range(0, project_width, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(x_off_button_start), tmp_layer->x_offset_start);
	gtk_table_attach(GTK_TABLE(text_table), GTK_WIDGET(x_off_button_start), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the starting Y Offset
	y_off_label_start = gtk_label_new("Start Y Offset: ");
	gtk_misc_set_alignment(GTK_MISC(y_off_label_start), 0, 0.5);
	gtk_table_attach(GTK_TABLE(text_table), GTK_WIDGET(y_off_label_start), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the starting Y Offset input
	y_off_button_start = gtk_spin_button_new_with_range(0, project_height, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(y_off_button_start), tmp_layer->y_offset_start);
	gtk_table_attach(GTK_TABLE(text_table), GTK_WIDGET(y_off_button_start), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the finishing X Offset
	x_off_label_finish = gtk_label_new("Finish X Offset: ");
	gtk_misc_set_alignment(GTK_MISC(x_off_label_finish), 0, 0.5);
	gtk_table_attach(GTK_TABLE(text_table), GTK_WIDGET(x_off_label_finish), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the finishing X Offset input
	x_off_button_finish = gtk_spin_button_new_with_range(0, project_width, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(x_off_button_finish), tmp_layer->x_offset_finish);
	gtk_table_attach(GTK_TABLE(text_table), GTK_WIDGET(x_off_button_finish), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the finishing Y Offset
	y_off_label_finish = gtk_label_new("Finish Y Offset: ");
	gtk_misc_set_alignment(GTK_MISC(y_off_label_finish), 0, 0.5);
	gtk_table_attach(GTK_TABLE(text_table), GTK_WIDGET(y_off_label_finish), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the finishing Y Offset input
	y_off_button_finish = gtk_spin_button_new_with_range(0, project_height, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(y_off_button_finish), tmp_layer->y_offset_finish);
	gtk_table_attach(GTK_TABLE(text_table), GTK_WIDGET(y_off_button_finish), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the font size
	font_label = gtk_label_new("Font size: ");
	gtk_misc_set_alignment(GTK_MISC(font_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(text_table), GTK_WIDGET(font_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the font size input
	font_button = gtk_spin_button_new_with_range(0, valid_fields[FRAME_NUMBER].max_value, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(font_button), tmp_text_ob->font_size);
	gtk_table_attach(GTK_TABLE(text_table), GTK_WIDGET(font_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the new layer name
	name_label = gtk_label_new("Layer name: ");
	gtk_misc_set_alignment(GTK_MISC(name_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(text_table), GTK_WIDGET(name_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the new layer name
	name_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(name_entry), valid_fields[LAYER_NAME].max_value);
	gtk_entry_set_text(GTK_ENTRY(name_entry), tmp_layer->name->str);
	gtk_table_attach(GTK_TABLE(text_table), GTK_WIDGET(name_entry), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the starting frame
	start_label = gtk_label_new("Start frame: ");
	gtk_misc_set_alignment(GTK_MISC(start_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(text_table), GTK_WIDGET(start_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the starting frame input
	start_button = gtk_spin_button_new_with_range(0, valid_fields[FRAME_NUMBER].max_value, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(start_button), tmp_layer->start_frame);
	gtk_table_attach(GTK_TABLE(text_table), GTK_WIDGET(start_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the finishing frame
	finish_label = gtk_label_new("Finish frame: ");
	gtk_misc_set_alignment(GTK_MISC(finish_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(text_table), GTK_WIDGET(finish_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the finishing frame input
	finish_button = gtk_spin_button_new_with_range(0, valid_fields[FRAME_NUMBER].max_value, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(finish_button), tmp_layer->finish_frame);
	gtk_table_attach(GTK_TABLE(text_table), GTK_WIDGET(finish_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for an external link
	external_link_label = gtk_label_new("External link: ");
	gtk_misc_set_alignment(GTK_MISC(external_link_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(text_table), GTK_WIDGET(external_link_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts an external link
	external_link_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(external_link_entry), valid_fields[EXTERNAL_LINK].max_value);
	gtk_entry_set_text(GTK_ENTRY(external_link_entry), tmp_layer->external_link->str);
	gtk_table_attach(GTK_TABLE(text_table), GTK_WIDGET(external_link_entry), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the window to open the external link in
	external_link_win_label = gtk_label_new("External link window: ");
	gtk_misc_set_alignment(GTK_MISC(external_link_win_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(text_table), GTK_WIDGET(external_link_win_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts a text string for the window to open the external link in
	external_link_win_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(external_link_win_entry), valid_fields[EXTERNAL_LINK_WINDOW].max_value);
	gtk_entry_set_text(GTK_ENTRY(external_link_win_entry), tmp_layer->external_link_window->str);
	gtk_table_attach(GTK_TABLE(text_table), GTK_WIDGET(external_link_win_entry), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
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
			g_string_free(valid_layer_name, TRUE);
			return FALSE;
		}

		// Reset the useable input flag
		useable_input = TRUE;

		// Retrieve the new starting frame x offset
		guint_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(x_off_button_start));
		validated_guint = validate_value(X_OFFSET, V_INT_UNSIGNED, &guint_val);
		if (NULL == validated_guint)
		{
			display_warning("Error ED172: There was something wrong with the starting frame X offset value.  Please try again.");
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
			display_warning("Error ED173: There was something wrong with the starting frame Y offset value.  Please try again.");
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
			display_warning("Error ED174: There was something wrong with the finish frame X offset value.  Please try again.");
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
			display_warning("Error ED175: There was something wrong with the finish frame X offset value.  Please try again.");
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
			display_warning("Error ED176: There was something wrong with the font size value.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_font_size = *validated_gfloat;
			g_free(validated_gfloat);
		}

		// Validate the layer name input
		validated_string = validate_value(LAYER_NAME, V_CHAR, (gchar *) gtk_entry_get_text(GTK_ENTRY(name_entry)));
		if (NULL == validated_string)
		{
			display_warning("Error ED177: There was something wrong with the layer name value.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_layer_name = g_string_assign(valid_layer_name, validated_string->str);
			g_string_free(validated_string, TRUE);
			validated_string = NULL;
		}

		// Retrieve the new start frame
		guint_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(start_button));
		validated_guint = validate_value(FRAME_NUMBER, V_INT_UNSIGNED, &guint_val);
		if (NULL == validated_guint)
		{
			display_warning("Error ED178: There was something wrong with the start frame value.  Please try again.");
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
			display_warning("Error ED179: There was something wrong with the finish frame value.  Please try again.");
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
			display_warning("Error ED180: There was something wrong with the external link value.  Please try again.");
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
			display_warning("Error ED181: There was something wrong with the external link target window value.  Please try again.");
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
	tmp_text_ob->font_size = valid_font_size;
	tmp_layer->start_frame = valid_start_frame;
	tmp_layer->finish_frame = valid_finish_frame;
	g_string_printf(tmp_layer->name, "%s", valid_layer_name->str);
	g_string_printf(tmp_layer->external_link, "%s", valid_ext_link->str);
	g_string_printf(tmp_layer->external_link_window, "%s", valid_ext_link_win->str);
	gtk_color_button_get_color(GTK_COLOR_BUTTON(color_button), &(tmp_text_ob->text_color));

	// Copy the text buffer from the onscreen widget to our existing text buffer
	gtk_text_buffer_get_start_iter(text_buffer, &text_start);
	gtk_text_buffer_get_end_iter(text_buffer, &text_end);
	text_gstring = g_string_new(gtk_text_buffer_get_slice(text_buffer, &text_start, &text_end, TRUE));
	gtk_text_buffer_set_text(GTK_TEXT_BUFFER(tmp_text_ob->text_buffer), text_gstring->str, text_gstring->len);

	// Destroy the dialog box
	gtk_widget_destroy(GTK_WIDGET(text_dialog));

	// Free the memory allocated in this function
	g_object_unref(text_buffer);
	g_string_free(valid_ext_link, TRUE);
	g_string_free(valid_ext_link_win, TRUE);
	g_string_free(valid_layer_name, TRUE);
	g_string_free(text_gstring, TRUE);

	return TRUE;
}
