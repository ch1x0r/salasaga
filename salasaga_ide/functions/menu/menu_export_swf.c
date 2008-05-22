/*
 * $Id$
 *
 * Salasaga: Function called when the user selects Export -> Flash Animation from the top menu 
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

// Ming include
#include <ming.h>

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../validate_value.h"
#include "../dialog/display_warning.h"
#include "../export/swf/export_swf_inner.h"


void menu_export_swf(void)
{
	// Local variables
	GtkFileFilter		*all_filter;				// Filter for *.*
	GtkWidget 			*export_dialog;				// Dialog widget
	gchar				*filename;					// Pointer to the chosen file name
	GtkFileFilter		*flash_filter;				// Filter for *.swf
	gint				return_code_gint;			// Catches the return code from the inner swf export function
	gboolean			useable_input;				// Used to control loop flow
	GString				*validated_string;			// Receives known good strings from the validation function
	GtkWidget			*warn_dialog;				// Widget for overwrite warning dialog

	GString				*tmp_gstring;				// Temporary GString


	// Check if there is an active project
	if (NULL == slides)
	{
		// No project is active, so display a message and return
		gdk_beep();
		display_warning("Error ED35: There is no project loaded\n");
		return;
	}

	// * Pop open a dialog asking the user for their desired filename *

	// Create the dialog asking the user for the name to save as
	export_dialog = gtk_file_chooser_dialog_new("Export as Flash",
						GTK_WINDOW(main_window),
						GTK_FILE_CHOOSER_ACTION_SAVE,
						GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
						NULL);

	// Create the filter so only *.flash files are displayed
	flash_filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(flash_filter, "*.swf");
	gtk_file_filter_set_name(flash_filter, "Adobe Flash (*.swf)");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(export_dialog), flash_filter);

	// Create the filter so all files (*.*) can be displayed
	all_filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(all_filter, "*.*");
	gtk_file_filter_set_name(all_filter, "All files (*.*)");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(export_dialog), all_filter);

	// Set the name of the file to save as
	tmp_gstring = g_string_new(NULL);
	g_string_printf(tmp_gstring, "%s.swf", project_name->str);
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(export_dialog), tmp_gstring->str);

	// Change to the default output directory
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(export_dialog), output_folder->str);

	// Loop around until we have a valid filename or the user cancels out
	useable_input = FALSE;
	validated_string = NULL;
	do
	{
		// Get a filename to save as
		if (gtk_dialog_run(GTK_DIALOG(export_dialog)) != GTK_RESPONSE_ACCEPT)
		{
			// The dialog was cancelled, so free the memory allocated in this function, destroy the dialog, and return to the caller
			g_string_free(tmp_gstring, TRUE);
			gtk_widget_destroy(export_dialog);
			return;
		}

		// Retrieve the filename from the dialog box
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(export_dialog));

		// Free the validated_string variable before recreating it
		if (NULL != validated_string)
		{
			g_string_free(validated_string, TRUE);
			validated_string = NULL;
		}

		// Validate the filename input
		validated_string = validate_value(FILE_PATH, V_CHAR, filename);
		if (NULL == validated_string)
		{
			// Invalid file name
			display_warning("Error ED182: There was something wrong with the file name given.  Please try again.");
		} else
		{
			// * Valid file name, so check if there's an existing file of this name, and give an Overwrite? type prompt if there is
			if (TRUE == g_file_test(validated_string->str, G_FILE_TEST_EXISTS))
			{
				// Something with this name already exists
				warn_dialog = gtk_message_dialog_new(GTK_WINDOW(main_window),
									GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
									GTK_MESSAGE_QUESTION,
									GTK_BUTTONS_YES_NO,
									"Overwrite existing file?");
				if (GTK_RESPONSE_YES == gtk_dialog_run(GTK_DIALOG(warn_dialog)))
				{
					// We've been told to overwrite the existing file
					useable_input = TRUE;
				}
				gtk_widget_destroy(warn_dialog);
			} else
			{
				// The indicated file name is unique, we're fine to save
				useable_input = TRUE;
			}
		}
	} while (FALSE == useable_input);

	// * We only get to here if a file was chosen *

	// Destroy the dialog box, as it's not needed any more
	gtk_widget_destroy(export_dialog);

	// Export the swf
	return_code_gint = export_swf_inner(validated_string->str);
	if (FALSE == return_code_gint)
	{
		// Something went wrong when creating the swf output stream
		display_warning("Error ED91: Something went wrong when creating the swf movie.");
	} else
	{
		// Movie created successfully, so update the status bar to let the user know
		g_string_printf(tmp_gstring, " Exported %u x %u flash - %s", output_width, output_height, validated_string->str);
		gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, tmp_gstring->str);
		gdk_flush();
	}

	// * Function clean up area *

	// Frees the memory holding the file name
	g_free(filename);

	// Free the temporary gstring
	g_string_free(tmp_gstring, TRUE);
	g_string_free(validated_string, TRUE);
}
