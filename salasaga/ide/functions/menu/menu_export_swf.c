/*
 * $Id$
 *
 * Salasaga: Function called when the user selects Export -> Flash Animation from the top menu
 *
 * Copyright (C) 2005-2010 Digital Distribution Global Training Solutions Pty. Ltd.
 * <justin@salasaga.org>
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
#include "../global_functions.h"
#include "../dialog/display_warning.h"
#include "../export/swf/export_swf_inner.h"
#include "../other/validate_value.h"
#include "../preference/project_preferences.h"


void menu_export_swf(void)
{
	// Local variables
	GtkFileFilter		*all_filter;				// Filter for *.*
	gchar				*dir_name;					// Temporarily holds the name of the directory being saved into
	GtkWidget 			*export_dialog;				// Dialog widget
	gchar				*filename;					// Pointer to the chosen file name
	GtkFileFilter		*flash_filter;				// Filter for *.swf
	GString				*message;					// Used to construct message strings
	gint				return_code_gint;			// Catches the return code from the inner swf export function
	gboolean			usable_input;				// Used to control loop flow
	GString				*validated_string;			// Receives known good strings from the validation function
	GtkWidget			*warn_dialog;				// Widget for overwrite warning dialog

	GString				*tmp_gstring;				// Temporary GString


	// Initialisation
	message = g_string_new(NULL);

	// Check if there is an active project
	if (NULL == get_slides())
	{
		// No project is active, so display a message and return
		gdk_beep();
		g_string_printf(message, "%s ED35: %s", _("Error"), _("There is no project loaded."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		return;
	}

	// * Pop open a dialog asking the user for their desired filename *

	// Create the dialog asking the user for the name to save as
	export_dialog = gtk_file_chooser_dialog_new(_("Export as Flash"),
						GTK_WINDOW(get_main_window()),
						GTK_FILE_CHOOSER_ACTION_SAVE,
						GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
						NULL);

	// Create the filter so only *.flash files are displayed
	flash_filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(flash_filter, "*.swf");
	gtk_file_filter_set_name(flash_filter, _("Adobe Flash (*.swf)"));
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(export_dialog), flash_filter);

	// Create the filter so all files (*.*) can be displayed
	all_filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(all_filter, "*.*");
	gtk_file_filter_set_name(all_filter, _("All files (*.*)"));
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(export_dialog), all_filter);

	// Set the name of the file to save as
	tmp_gstring = g_string_new(NULL);
	g_string_printf(tmp_gstring, "%s.swf", get_project_name());
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(export_dialog), tmp_gstring->str);

	// Change to the default output directory
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(export_dialog), get_output_folder());

	// Loop around until we have a valid filename or the user cancels out
	usable_input = FALSE;
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
			g_string_printf(message, "%s ED182: %s", _("Error"), _("There was something wrong with the file name given.  Please try again."));
			display_warning(message->str);
		} else
		{
			// * Valid file name, so check if there's an existing file of this name, and give an Overwrite? type prompt if there is
			if (TRUE == g_file_test(validated_string->str, G_FILE_TEST_EXISTS))
			{
				// Something with this name already exists
				warn_dialog = gtk_message_dialog_new(GTK_WINDOW(get_main_window()),
									GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
									GTK_MESSAGE_QUESTION,
									GTK_BUTTONS_YES_NO,
									_("Overwrite existing file?"));
				if (GTK_RESPONSE_YES == gtk_dialog_run(GTK_DIALOG(warn_dialog)))
				{
					// We've been told to overwrite the existing file
					usable_input = TRUE;
				}
				gtk_widget_destroy(warn_dialog);
			} else
			{
				// The indicated file name is unique, we're fine to save
				usable_input = TRUE;
			}
		}
	} while (FALSE == usable_input);

	// * We only get to here if a file was chosen *

	// Destroy the dialog box, as it's not needed any more
	gtk_widget_destroy(export_dialog);

	// Update the output folder variable with this new path
	dir_name = g_path_get_dirname(validated_string->str);
	set_output_folder(dir_name);
	g_free(dir_name);

	// Update the status bar to indicate we're exporting the swf file
	g_string_printf(tmp_gstring, " %s %u x %u flash - %s", _("Exporting"), get_output_width(), get_output_height(), validated_string->str);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), tmp_gstring->str);

	// Export the swf
	return_code_gint = export_swf_inner(validated_string->str);
	if (FALSE == return_code_gint)
	{
		// Something went wrong when creating the swf output stream
		g_string_printf(message, "%s ED91: %s", _("Error"), _("Something went wrong when creating the swf movie."));
		display_warning(message->str);
	} else
	{
		// Movie created successfully, so update the status bar to let the user know
		g_string_printf(tmp_gstring, " %s %u x %u flash - %s", _("Exported"), get_output_width(), get_output_height(), validated_string->str);
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), tmp_gstring->str);
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(get_status_bar()), 0.0);
		gdk_flush();
	}

	// * Function clean up area *

	// Frees the memory holding the file name
	g_free(filename);

	// Free the temporary gstring
	g_string_free(message, TRUE);
	g_string_free(tmp_gstring, TRUE);
	g_string_free(validated_string, TRUE);
}
