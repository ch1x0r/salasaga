/*
 * $Id$
 *
 * Salasaga: Function called when the user selects Export -> Html wrapper from the top menu
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


// Standard includes
#include <stdio.h>

// GTK includes
#include <gtk/gtk.h>

// Salasaga includes
#include "../../../salasaga_types.h"
#include "../../../externs.h"
#include "../../dialog/display_warning.h"
#include "../../other/validate_value.h"
#include "../../preference/project_preferences.h"


void export_html_wrapper(void)
{
	// Local variables
	GtkFileFilter		*all_filter;				// Filter for *.*
	gchar				*base_name;					// Pointer to the file name, excluding the path component
	gchar				*dir_name;					// Temporarily holds the name of the directory being saved into
	GtkWidget 			*export_dialog;				// Dialog widget
	gchar				*filename;					// Pointer to the chosen file name
	GtkFileFilter		*file_filter;				// Filter for *.html
	FILE				*output_file;				// Name of the html output file
	GString				*message;					// Used to construct message strings
	gchar				**string_tokens;
	gchar				*suffix_start;
	gchar				*swf_file;					// Name of the swf file we add to the html content
	GString				*tmp_gstring;				// Temporary GString
	gboolean			usable_input;				// Used to control loop flow
	GString				*validated_string;			// Receives known good strings from the validation function
	GtkWidget			*warn_dialog;				// Widget for overwrite warning dialog


	// * Pop open a dialog asking the user for their desired filename *

	// Initialisation
	message = g_string_new(NULL);

	// Create the dialog asking the user for the name to save as
	export_dialog = gtk_file_chooser_dialog_new(_("Export Html"),
						GTK_WINDOW(get_main_window()),
						GTK_FILE_CHOOSER_ACTION_SAVE,
						GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
						NULL);

	// Create the filter so only *.html files are displayed
	file_filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(file_filter, "*.html");
	gtk_file_filter_set_name(file_filter, _("HyperText Markup Language (*.html)"));
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(export_dialog), file_filter);

	// Create the filter so all files (*.*) can be displayed
	all_filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(all_filter, "*.*");
	gtk_file_filter_set_name(all_filter, _("All files (*.*)"));
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(export_dialog), all_filter);

	// Set the name of the file to save as
	tmp_gstring = g_string_new(NULL);
	g_string_printf(tmp_gstring, "%s.html", get_project_name());
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
			g_string_printf(message, "%s ED418: %s", _("Error"), _("There was something wrong with the file name given.  Please try again."));
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

	// Get a pointer to the start of file name suffix
	base_name = g_path_get_basename(validated_string->str);
	suffix_start = g_strrstr(base_name, ".");

	// Get the file name, minus the suffix
	string_tokens = g_strsplit(base_name, suffix_start, 2);

	// Create a string for the matching .swf file
	swf_file = g_strjoin(NULL, string_tokens[0], ".swf", NULL);

	// Create the html wrapper file
	output_file = fopen(validated_string->str, "w");
	if (NULL == output_file)
	{
		g_string_printf(message, "%s ED419: %s", _("Error"), _("Couldn't create the output file."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		return;
	}

	// Start the html output
	fprintf(output_file, "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"\n"
			"\"http://www.w3.org/TR/1999/REC-html401-19991224/loose.dtd\">\n");
	fprintf(output_file, "<html>\n");
	fprintf(output_file, "\t<head>\n");
	fprintf(output_file, "\t\t<meta name=\"generator\" content=\"Salasaga, see www.salasaga.org\">\n");
	fprintf(output_file, "\t\t<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n");

	// If the project has a title, add it to the output file
	if (0 < get_project_name_length())
	{
		fprintf(output_file, "\t\t<title>%s</title>\n", get_project_name());
	}
	fprintf(output_file, "\t</head>\n");

	// Create the html wrapper for the given resolution
	fprintf(output_file, "\t<body>\n"
			"\t\t<object classid=\"clsid:D27CDB6E-AE6D-11cf-96B8-444553540000\" codebase=\"http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=7,0,0,0\" width=\"%u\" height=\"%u\">\n"
			"\t\t\t<param name=\"movie\" value=\"%s\"></param>\n"
			"\t\t\t<param name=\"quality\" value=\"high\"></param>\n"
			"\t\t\t<embed src=\"%s\" quality=\"high\" pluginspage=\"http://www.macromedia.com/shockwave/download/index.cgi?P1_Prod_Version=ShockwaveFlash\" type=\"application/x-shockwave-flash\" align=\"middle\" width=\"%u\" height=\"%u\"></embed>\n"
			"\t\t</object>\n", get_output_width(), get_output_height(), swf_file, swf_file, get_output_width(), get_output_height());
	fprintf(output_file, "\t</body>\n</html>");

	// Close the output file
	fclose(output_file);

	// Html wrapper file was created successfully, so update the status bar to let the user know
	g_string_printf(tmp_gstring, " %s - %s", _("Wrote html wrapper"), validated_string->str);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), tmp_gstring->str);
	gdk_flush();

	// * Function clean up area *

	// Frees the memory holding the file name
	g_free(filename);

	// Free the temporary strings
	g_string_free(tmp_gstring, TRUE);
	g_string_free(message, TRUE);
	g_strfreev(string_tokens);
	g_string_free(validated_string, TRUE);
}
