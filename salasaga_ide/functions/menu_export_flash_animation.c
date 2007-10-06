/*
 * $Id$
 *
 * Flame Project: Function called when the user selects Export -> Flash Animation from the top menu 
 * 
 * Copyright (C) 2007 Justin Clift <justin@postgresql.org>
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

// Standard includes
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

// GTK includes
#include <glib/gstdio.h>
#include <gtk/gtk.h>

// Gnome includes
#include <libgnome/gnome-url.h>

// XML includes
#include <libxml/xmlsave.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
	#include "flame-keycapture.h"
#endif

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"
#include "display_warning.h"
#include "menu_export_flash_inner.h"
#include "sound_beep.h"


void menu_export_flash_animation(void)
{
	// Local variables
	GtkFileFilter			*all_filter;				// Filter for *.*
	GError				*error = NULL;				// Pointer to error return structure
	GtkWidget 			*export_dialog;				// Dialog widget
	gchar				*filename;				// Pointer to the chosen file name
	GtkFileFilter			*flash_filter;				// Filter for *.swf
	GIOStatus			return_value;				// Return value used in most GIOChannel functions
	GByteArray			*swf_buffer;				// Buffer for the swf output
	gboolean			unique_name;				// Switch used to mark when we have a valid filename
	GtkWidget			*warn_dialog;				// Widget for overwrite warning dialog

	gsize				tmp_gsize;				// Temporary gsize
	GString				*tmp_gstring;				// Temporary GString
	gint				tmp_int;				// Temporary integer


	// Check if there is an active project
	if (NULL == slides)
	{
		// No project is active, so display a message and return
		// fixme4: This code should never be reached any more, as exporting is disabled while no project loaded
		sound_beep();
		display_warning("ED35: There is no project loaded\n");
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
	gtk_file_filter_set_name(flash_filter, "Macromedia Flash (*.swf)");
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

	// Run the dialog and wait for user input
	unique_name = FALSE;
	while (TRUE != unique_name)
	{
		// Get the filename to export to
		if (gtk_dialog_run(GTK_DIALOG(export_dialog)) != GTK_RESPONSE_ACCEPT)
		{
			// The dialog was cancelled, so destroy it and return to the caller
			gtk_widget_destroy(export_dialog);
			return;
		}

		// Retrieve the filename from the dialog box
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(export_dialog));

		// Check if there's an existing file of this name, and give an Overwrite? type prompt if there is
		if (TRUE == g_file_test(filename, G_FILE_TEST_EXISTS))
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
				unique_name = TRUE;
			}
			gtk_widget_destroy(warn_dialog);
		} else
		{
			// The indicated file name is unique, we're fine to save
			unique_name = TRUE;
		}
	}

	// * We only get to here if a file was chosen *

	// Get the filename from the dialog box
	filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(export_dialog));

	// Destroy the dialog box, as it's not needed any more
	gtk_widget_destroy(export_dialog);

	// Open output file for writing
	output_file = g_io_channel_new_file(filename, "w", &error);
	if (NULL == output_file)
	{
		// * An error occured when opening the file for writing, so alert the user, and return to the calling routine indicating failure *
		g_string_printf(tmp_gstring, "Error ED80: An error '%s' occured when opening '%s' for writing", error->message, filename);
		display_warning(tmp_gstring->str);

		// Free the memory allocated in this function
		g_string_free(tmp_gstring, TRUE);
		g_error_free(error);

		return;
	}

	// Work out how many slides there are in the whole project
	slides = g_list_first(slides);
	tmp_int = g_list_length(slides);

	// Export all slides to the swf buffer
	swf_buffer = g_byte_array_new();
	swf_buffer = menu_export_flash_inner(swf_buffer);
//	swf_buffer = menu_export_flash_inner(swf_buffer, 0, tmp_int - 1);

	// Add the end tag to the swf buffer
	// fixme3: For some unknown reason having \0 crashes out, whereas every other character works
	//         It's a pity the \0 is actually needed, so we'll need to add this later somehow
//	swf_buffer = g_byte_array_append(swf_buffer, "\0", 1);

	// Add the header to the swf buffer
	// fixme3: Need to expand this next bit to also include the frame size, frame rate, and frame count
	g_string_printf(tmp_gstring, "FWS%c%u", 0x06, (guint32) swf_buffer->len);  // 0x06 = swf version 6
	swf_buffer = g_byte_array_prepend(swf_buffer, (const guint8 *) tmp_gstring->str, tmp_gstring->len);

	// Write the swf data to the output file
	return_value = g_io_channel_write_chars(output_file, (const gchar *) swf_buffer->data, swf_buffer->len, &tmp_gsize, &error);
	if (G_IO_STATUS_ERROR == return_value)
	{
		// * An error occured when writing the swf data to the output file, so alert the user, and return to the calling routine indicating failure *
		g_string_printf(tmp_gstring, "Error ED81: An error '%s' occured when writing the swf data to the output file '%s'", error->message, filename);
		display_warning(tmp_gstring->str);

		// Free the memory allocated in this function
		g_string_free(tmp_gstring, TRUE);
		g_error_free(error);

		return;
	}

	// Close the output file
	return_value = g_io_channel_shutdown(output_file, TRUE, &error);
	if (G_IO_STATUS_ERROR == return_value)
	{
		// * An error occured when closing the output file, so alert the user, and return to the calling routine indicating failure *
		g_string_printf(tmp_gstring, "Error ED82: An error '%s' occured when closing the output file '%s'", error->message, filename);

		// Display the warning message using our function
		display_warning(tmp_gstring->str);

		// Free the memory allocated in this function
		g_string_free(tmp_gstring, TRUE);
		g_error_free(error);
		return;
	}

	// Small update to the status bar, to show progress to the user
	g_string_printf(tmp_gstring, "Wrote Flash file '%s'.", filename);
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, tmp_gstring->str);
	gdk_flush();

	// * Function clean up area *

	// Free the swf buffer array
	g_byte_array_free(swf_buffer, TRUE);

	// Frees the memory holding the file name
	g_free(filename);

	// Free the temporary gstring
	g_string_free(tmp_gstring, TRUE);
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.2  2007/10/06 11:38:28  vapour
 * Continued adjusting function include definitions.
 *
 * Revision 1.1  2007/09/29 04:22:15  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */
