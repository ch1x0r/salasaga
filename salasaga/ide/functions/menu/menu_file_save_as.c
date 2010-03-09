/*
 * $Id$
 *
 * Salasaga: Function called when the user selects File -> Save As from the top menu
 *
 * Copyright (C) 2005-2010 Justin Clift <justin@salasaga.org>
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

// XML includes
#include <libxml/xmlsave.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../../salasaga_types.h"
#include "../global_functions.h"
#include "../dialog/display_warning.h"
#include "../other/validate_value.h"
#include "../preference/application_preferences.h"
#include "../preference/project_preferences.h"
#include "../save/save_slide.h"
#include "menu_file_save.h"


void menu_file_save_as(void)
{
	// Local variables
	GtkFileFilter		*all_filter;				// Filter for *.*
	gchar				*dir_name;					// Temporarily holds the name of the directory being saved into
	gchar				*dir_name_part;				// Briefly used for holding a directory name
	gchar				*filename;					// Pointer to the chosen file name
	gchar				*file_name_part;			// Briefly used for holding a file name
	GString				*message;					// Used to construct message strings
	GtkFileFilter		*salasaga_filter;			// Filter for *.salasaga
	GtkWidget 			*save_dialog;				// Dialog widget
	GString				*tmp_gstring;				// Temporary GString
	gboolean			usable_input;				// Used to control loop flow
	GString				*validated_string;			// Receives known good strings from the validation function
	GtkWidget			*warn_dialog;				// Widget for overwrite warning dialog


	// If there's no project active, we just beep and return
	if (FALSE == get_project_active())
	{
		gdk_beep();
		return;
	}

	// Initialise some things
	message = g_string_new(NULL);
	tmp_gstring = g_string_new(NULL);

	// Create the dialog asking the user for the name to save as
	save_dialog = gtk_file_chooser_dialog_new(_("Save As"),
											  GTK_WINDOW(get_main_window()),
											  GTK_FILE_CHOOSER_ACTION_SAVE,
											  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
											  GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
											  NULL);

	// Create the filter so only *.salasaga files are displayed
	salasaga_filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(salasaga_filter, "*.salasaga");
	gtk_file_filter_set_name(salasaga_filter, _("Salasaga Project file (*.salasaga)"));
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(save_dialog), salasaga_filter);

	// Create the filter so all files (*.*) can be displayed
	all_filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(all_filter, "*.*");
	gtk_file_filter_set_name(all_filter, _("All files (*.*)"));
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(save_dialog), all_filter);

	// Set the path and name of the file to save as.  Use project_name as a default
	if (NULL != get_file_name())
	{
		// Work out the directory and file name components
		dir_name_part = g_path_get_dirname(get_file_name());
		file_name_part = g_path_get_basename(get_file_name());

		// Set the default directory and file name for the dialog
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(save_dialog), dir_name_part);
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(save_dialog), file_name_part);

		// Free the string
		g_free(dir_name_part);
		g_free(file_name_part);
	} else
	{
		// Nothing has been established, so use project_name
		g_string_printf(tmp_gstring, "%s.salasaga", get_project_name());
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(save_dialog), tmp_gstring->str);

		// Change to the default project directory
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(save_dialog), get_default_project_folder());
	}

	// Loop around until we have a valid filename or the user cancels out
	usable_input = FALSE;
	validated_string = NULL;
	do
	{
		// Get a filename to save as
		if (gtk_dialog_run(GTK_DIALOG(save_dialog)) != GTK_RESPONSE_ACCEPT)
		{
			// The dialog was cancelled, so free the memory allocated in this function, destroy the dialog, and return to the caller
			g_string_free(tmp_gstring, TRUE);
			gtk_widget_destroy(save_dialog);
			return;
		}

		// Retrieve the filename from the dialog box
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(save_dialog));

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
			g_string_printf(message, "%s ED125: %s", _("Error"), _("There was something wrong with the file name given.  Please try again."));
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

	// Destroy the dialog box, as it's not needed any more
	gtk_widget_destroy(save_dialog);

	// Keep the full file name for future reference
	set_file_name(validated_string->str);

	// Update the project folder variable with this new path
	dir_name = g_path_get_dirname(validated_string->str);
	set_project_folder(dir_name);
	g_free(dir_name);

	// Save the file
	menu_file_save();

	// * Function clean up area *

	// Free the memory allocated in this function
	g_string_free(message, TRUE);
	g_free(filename);
	g_string_free(validated_string, TRUE);
	g_string_free(tmp_gstring, TRUE);
}
