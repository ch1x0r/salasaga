/*
 * $Id$
 *
 * Salasaga: Display a dialog box asking for empty layer settings 
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


gboolean display_dialog_empty(layer *tmp_layer, gchar *dialog_title)
{
	// Local variables
	GtkDialog			*empty_dialog;				// Widget for the dialog
	GtkWidget			*empty_table;				// Table used for neat layout of the dialog box
	GString				*message;					// Used to construct message strings
	guint				row_counter = 0;			// Used to count which row things are up to
	gboolean			useable_input;				// Used as a flag to indicate if all validation was successful
	GString				*valid_ext_link;			// Receives the new external link once validated
	GString				*valid_ext_link_win;		// Receives the new external link window once validated
	GString				*validated_string;			// Receives known good strings from the validation function

	GtkWidget			*label_bg_colour;			// Background colour
	GtkWidget			*button_bg_colour;			// Color button

	GtkWidget			*external_link_label;		// Label widget
	GtkWidget			*external_link_entry;		// Widget for accepting an external link for clicking on

	GtkWidget			*external_link_win_label;	// Label widget
	GtkWidget			*external_link_win_entry;	//

	layer_empty			*tmp_empty_ob;				// Temporary empty layer object


	// Initialise some things
	tmp_empty_ob = (layer_empty *) tmp_layer->object_data;
	message = g_string_new(NULL);
	valid_ext_link = g_string_new(NULL);
	valid_ext_link_win = g_string_new(NULL);

	// * Pop open a dialog box asking the user for the details of the layer *

	// Create the dialog window, and table to hold its children
	empty_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(dialog_title, GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL));
	empty_table = gtk_table_new(3, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(empty_dialog->vbox), GTK_WIDGET(empty_table), FALSE, FALSE, 10);

	// Background Colour
	label_bg_colour = gtk_label_new(_("Background Colour: "));
	gtk_misc_set_alignment(GTK_MISC(label_bg_colour), 0, 0.5);
	gtk_table_attach(GTK_TABLE(empty_table), GTK_WIDGET(label_bg_colour), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	button_bg_colour = gtk_color_button_new_with_color(&tmp_empty_ob->bg_color);
	gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(button_bg_colour), TRUE);
	gtk_table_attach(GTK_TABLE(empty_table), GTK_WIDGET(button_bg_colour), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for an external link
	external_link_label = gtk_label_new(_("External link: "));
	gtk_misc_set_alignment(GTK_MISC(external_link_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(empty_table), GTK_WIDGET(external_link_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts an external link
	external_link_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(external_link_entry), valid_fields[EXTERNAL_LINK].max_value);
	gtk_entry_set_text(GTK_ENTRY(external_link_entry), tmp_layer->external_link->str);
	gtk_table_attach(GTK_TABLE(empty_table), GTK_WIDGET(external_link_entry), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the window to open the external link in
	external_link_win_label = gtk_label_new(_("External link window: "));
	gtk_misc_set_alignment(GTK_MISC(external_link_win_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(empty_table), GTK_WIDGET(external_link_win_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts a text string for the window to open the external link in
	external_link_win_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(external_link_win_entry), valid_fields[EXTERNAL_LINK_WINDOW].max_value);
	gtk_entry_set_text(GTK_ENTRY(external_link_win_entry), tmp_layer->external_link_window->str);
	gtk_table_attach(GTK_TABLE(empty_table), GTK_WIDGET(external_link_win_entry), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Ensure everything will show
	gtk_widget_show_all(GTK_WIDGET(empty_dialog));

	// Loop around until we have all valid values, or the user cancels out
	validated_string = NULL;
	do
	{
		// Display the dialog
		if (GTK_RESPONSE_ACCEPT != gtk_dialog_run(GTK_DIALOG(empty_dialog)))
		{
			// The dialog was cancelled, so destroy it and return to the caller
			gtk_widget_destroy(GTK_WIDGET(empty_dialog));
			g_string_free(valid_ext_link, TRUE);
			g_string_free(valid_ext_link_win, TRUE);
			return FALSE;
		}

		// Reset the useable input flag
		useable_input = TRUE;

		// Validate the external link input
		validated_string = validate_value(EXTERNAL_LINK, V_CHAR, (gchar *) gtk_entry_get_text(GTK_ENTRY(external_link_entry)));
		if (NULL == validated_string)
		{
			g_string_printf(message, "%s ED142: %s", _("Error"), _("There was something wrong with the external link value.  Please try again."));
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
			g_string_printf(message, "%s ED143: %s", _("Error"), _("There was something wrong with the external link window target value.  Please try again."));
			display_warning(message->str);
			useable_input = FALSE;
		} else
		{
			valid_ext_link_win = g_string_assign(valid_ext_link_win, validated_string->str);
			g_string_free(validated_string, TRUE);
			validated_string = NULL;
		}
	} while (FALSE == useable_input);

	// * We only get here after all input is considered valid *

	// Retrieve the updated values
	gtk_color_button_get_color(GTK_COLOR_BUTTON(button_bg_colour), &tmp_empty_ob->bg_color);
	g_string_printf(tmp_layer->external_link, "%s", valid_ext_link->str);
	g_string_printf(tmp_layer->external_link_window, "%s", valid_ext_link_win->str);

	// Destroy the dialog box
	gtk_widget_destroy(GTK_WIDGET(empty_dialog));

	// Free the memory allocated in this function
	g_string_free(message, TRUE);
	g_string_free(valid_ext_link, TRUE);
	g_string_free(valid_ext_link_win, TRUE);
	return TRUE;
}
