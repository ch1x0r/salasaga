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


// GTK includes
#include <glib/gstdio.h>
#include <gtk/gtk.h>

// Ming include
#include <ming.h>

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"
#include "display_warning.h"
#include "menu_export_flash_inner.h"
#include "sound_beep.h"


void menu_export_flash_animation(void)
{
	// Local variables
	GtkFileFilter		*all_filter;			// Filter for *.*
	GtkWidget 			*export_dialog;			// Dialog widget
	gchar				*filename;				// Pointer to the chosen file name
	GtkFileFilter		*flash_filter;			// Filter for *.swf
	gboolean			unique_name;			// Switch used to mark when we have a valid filename
	GtkWidget			*warn_dialog;			// Widget for overwrite warning dialog

	SWFMovie			swf_movie;				// Swf movie object

	GString				*tmp_gstring;			// Temporary GString


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

	// Initialise Ming and create an empty swf movie object
	Ming_init();
	swf_movie = newSWFMovieWithVersion(7);
	Ming_setSWFCompression(9);

	// Set the output size of the swf movie 
	SWFMovie_setDimension(swf_movie, output_width, output_height);

	// Set the frame rate for the movie
	SWFMovie_setRate(swf_movie, frames_per_second);

	// Set the background color for the animation
	SWFMovie_setBackground(swf_movie, 0x00, 0x00, 0x00);  // RGB value - black

	// Export all slides to the swf buffer
	swf_movie = menu_export_flash_inner(swf_movie);
	if (NULL == swf_movie)
	{
		// Something went wrong when creating the swf output stream
		display_warning("Error ED91: Something went wrong when creating the swf movie.");

		// Free the swf movie buffer
		destroySWFMovie(swf_movie);

		// Frees the memory holding the file name
		g_free(filename);

		// Free the memory allocated in this function
		g_string_free(tmp_gstring, TRUE);

		return;
	}

	// Save the swf movie file to disk
	SWFMovie_save(swf_movie, filename);

	// Update the status bar, to show progress to the user
	g_string_printf(tmp_gstring, "Wrote Flash file '%s'.", filename);
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, tmp_gstring->str);
	gdk_flush();

	// * Function clean up area *

	// Free the swf movie buffer
	destroySWFMovie(swf_movie);

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
 * Revision 1.9  2008/01/15 11:57:59  vapour
 * Added code to free memory better if an error occurs.
 *
 * Revision 1.8  2008/01/15 06:17:19  vapour
 * Removed old variables and code no longer used.
 *
 * Revision 1.7  2008/01/13 10:40:33  vapour
 * Updated to set the dimensions, frame rate, and background colour of the swf output here.
 * Also updated to use the new definition for the inner swf generation function.
 *
 * Revision 1.6  2008/01/13 05:37:40  vapour
 * Started conveting the existing swf output code to use Ming.
 *
 * Revision 1.5  2007/10/10 15:01:17  vapour
 * Had to set the gio stream to NULL encoding, so it is safe with binary data.
 *
 * Revision 1.4  2007/10/07 14:22:16  vapour
 * Moved initial allocation of swf buffer into the inner function.
 *
 * Revision 1.3  2007/10/07 08:44:38  vapour
 * Added initial (untested) code for creating the swf header.
 *
 * Revision 1.2  2007/10/06 11:38:28  vapour
 * Continued adjusting function include definitions.
 *
 * Revision 1.1  2007/09/29 04:22:15  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */
