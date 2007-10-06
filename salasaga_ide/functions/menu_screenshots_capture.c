/*
 * $Id$
 *
 * Flame Project: Function called when the user selects Screenshots -> Capture from the top menu
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


void menu_screenshots_capture(void)
{
	// Local variables
	GError				*error = NULL;				// Pointer to error return structure
	gchar				*full_file_name;			// Holds the fully worked out file name to save as
	GIOChannel			*output_file;				// The output file handle
	GIOStatus			return_value;				// Return value used in most GIOChannel functions

	GtkDialog			*capture_dialog;			// Widget for the dialog
	GtkWidget			*capture_table;				// Table used for neat layout of the dialog box

	GtkWidget			*x_offset_label;			// Label widget
	GtkWidget			*x_offset_button;			// Widget for accepting the new X Offset data

	GtkWidget			*y_offset_label;			// Label widget
	GtkWidget			*y_offset_button;			// Widget for accepting the new Y Offset data

	GtkWidget			*x_length_label;			// Label widget
	GtkWidget			*x_length_button;			// Widget for accepting the new X Length data

	GtkWidget			*y_length_label;			// Label widget
	GtkWidget			*y_length_button;			// Widget for accepting the new Y Length data

	GtkWidget			*key_combo_label;			// Label widget

	gint				dialog_result;				// Catches the return code from the dialog box

	GdkScreen			*which_screen;				// Gets given the screen the monitor is on

	gchar				*tmp_gchar;				// Temporary gchar
	gsize				tmp_gsize;				// Temporary gsize
	gpointer			tmp_ptr;				// Temporary pointer
	GString				*tmp_gstring;				// Temporary string


	GKeyFile			*lock_file;				// Pointer to the lock file structure


	// Initialise various things
	tmp_gstring = g_string_new(NULL);

	// * Pop open a dialog box asking the user for the offset and size of capture area *

	// Create the dialog window, and table to hold its children
	capture_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons("Capture screenshots", GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL));
	capture_table = gtk_table_new(3, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(capture_dialog->vbox), GTK_WIDGET(capture_table), FALSE, FALSE, 10);

	// Create the label asking for the X Offset
	x_offset_label = gtk_label_new("X Offset: ");
	gtk_misc_set_alignment(GTK_MISC(x_offset_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(capture_table), GTK_WIDGET(x_offset_label), 0, 1, 0, 1);

	// Create the entry that accepts the new X Offset data
	x_offset_button = gtk_spin_button_new_with_range(0, project_width, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(x_offset_button), capture_x);
	gtk_table_attach_defaults(GTK_TABLE(capture_table), GTK_WIDGET(x_offset_button), 1, 2, 0, 1);

	// Create the label asking for the Y Offset
	y_offset_label = gtk_label_new("Y Offset: ");
	gtk_misc_set_alignment(GTK_MISC(y_offset_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(capture_table), GTK_WIDGET(y_offset_label), 0, 1, 1, 2);

	// Create the entry that accepts the new Y Offset data
	y_offset_button = gtk_spin_button_new_with_range(0, project_height, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(y_offset_button), capture_y);
	gtk_table_attach_defaults(GTK_TABLE(capture_table), GTK_WIDGET(y_offset_button), 1, 2, 1, 2);

	// Which monitor are we displaying on?
	which_screen = gtk_window_get_screen(GTK_WINDOW(main_window));

	// Create the label asking for the X Length
	x_length_label = gtk_label_new("Width: ");
	gtk_misc_set_alignment(GTK_MISC(x_length_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(capture_table), GTK_WIDGET(x_length_label), 0, 1, 2, 3);

	// Create the entry that accepts the new X Length data
	x_length_button = gtk_spin_button_new_with_range(0, gdk_screen_get_width(which_screen), 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(x_length_button), capture_width);
	gtk_table_attach_defaults(GTK_TABLE(capture_table), GTK_WIDGET(x_length_button), 1, 2, 2, 3);

	// Create the label asking for the Y Length
	y_length_label = gtk_label_new("Height: ");
	gtk_misc_set_alignment(GTK_MISC(y_length_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(capture_table), GTK_WIDGET(y_length_label), 0, 1, 3, 4);

	// Create the entry that accepts the new Y Length data
	y_length_button = gtk_spin_button_new_with_range(0, gdk_screen_get_height(which_screen), 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(y_length_button), capture_height);
	gtk_table_attach_defaults(GTK_TABLE(capture_table), GTK_WIDGET(y_length_button), 1, 2, 3, 4);

	// Create the label that tells the user about Control-Printscreen
	key_combo_label = gtk_label_new("Hint: Use the Control-Printscreen key\ncombination to capture screenshots,\nthen the Import button to import them.");
	gtk_misc_set_alignment(GTK_MISC(key_combo_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(capture_table), GTK_WIDGET(key_combo_label), 0, 2, 4, 5);

	// Run the dialog
	gtk_widget_show_all(GTK_WIDGET(capture_dialog));
	dialog_result = gtk_dialog_run(GTK_DIALOG(capture_dialog));

	// Was the anything other than the OK button pressed?
	if (GTK_RESPONSE_ACCEPT != dialog_result)
	{
		// The user decided to escape out of the dialog box, so just return
		gtk_widget_destroy(GTK_WIDGET(capture_dialog));
		return;
	}

	// Construct the fullly qualified path name for ~/.flame-lock file (to hold capture settings in)
	tmp_ptr = (gchar *) g_get_home_dir();
	full_file_name = g_build_filename(tmp_ptr, ".flame-lock", NULL);

	// Retrieve the values given by the user
	capture_x = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(x_offset_button));
	capture_y = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(y_offset_button));
	capture_width = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(x_length_button));
	capture_height = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(y_length_button));

	// Create the contents of the ~/.flame-lock file in memory
	lock_file = g_key_file_new();
	g_key_file_set_string(lock_file, "Project", "Name", project_name->str);  // Name of project
	g_key_file_set_string(lock_file, "Project", "Directory", screenshots_folder->str);  // Directory to save screenshots in
	g_key_file_set_integer(lock_file, "Project", "X_Offset", capture_x);  // Top left X coordinate of screen area
	g_key_file_set_integer(lock_file, "Project", "X_Length", capture_width);  // Width of screen area to grab
	g_key_file_set_integer(lock_file, "Project", "Y_Offset", capture_y);  // Top left Y coordinate of screen area
	g_key_file_set_integer(lock_file, "Project", "Y_Length", capture_height);  // Height of screen area to grab

	// Create IO channel for writing to
	output_file = g_io_channel_new_file(full_file_name, "w", &error);
	if (NULL == output_file)
	{
		// * An error occured when opening the file for writing, so alert the user, and return to the calling routine indicating failure *

		// Display a warning message using our function
		g_string_printf(tmp_gstring, "Error ED16: An error '%s' occured when opening '%s' for writing", error->message, full_file_name);
		display_warning(tmp_gstring->str);

		// Free the memory allocated in this function
		g_string_free(tmp_gstring, TRUE);
		g_error_free(error);

		return;
	}

	// Write the ~/.flame-lock file to disk
	tmp_gchar = g_key_file_to_data(lock_file, NULL, NULL);
	return_value = g_io_channel_write_chars(output_file, tmp_gchar, strlen(tmp_gchar), &tmp_gsize, &error);
	if (G_IO_STATUS_ERROR == return_value)
	{
		// * An error occured when writing the SVG header to the output file, so alert the user, and return to the calling routine indicating failure *
		g_string_printf(tmp_gstring, "Error ED17: An error '%s' occured when writing data to the '%s' file", error->message, full_file_name);
		display_warning(tmp_gstring->str);

		// Free the memory allocated in this function
		g_string_free(tmp_gstring, TRUE);
		g_error_free(error);

		return;
	}

	// Close the IO channel
	return_value = g_io_channel_shutdown(output_file, TRUE, &error);
	if (G_IO_STATUS_ERROR == return_value)
	{
		// * An error occured when closing the output file, so alert the user, and return to the calling routine indicating failure *
		g_string_printf(tmp_gstring, "Error ED18: An error '%s' occured when closing the output file '%s'", error->message, full_file_name);

		// Display the warning message using our function
		display_warning(tmp_gstring->str);

		// Free the memory allocated in this function
		g_string_free(tmp_gstring, TRUE);
		g_error_free(error);

		return;
	}

#ifdef _WIN32
	// If not already set, install keyboard hook for Control-Printscreen combination

	// Local variables
	HINSTANCE			dll_handle;
	HOOKPROC			hook_address; 

	// Variables for the error handler
	DWORD				last_error;
	LPVOID				message_buffer_pointer;
	TCHAR				text_buffer[120];

	// Does the keyboard hook need to be installed?
	if (NULL == win32_keyboard_hook_handle)
	{
		dll_handle = LoadLibrary((LPCTSTR) "libflame-keycapture.dll");
		if (NULL == dll_handle)
		{
			last_error = GetLastError();
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, last_error,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &message_buffer_pointer, 0, NULL);
			wsprintf(text_buffer, "Keyboard hook LoadLibrary failed with error %d: %s", last_error, message_buffer_pointer); 
			MessageBox(NULL, text_buffer, "Error", MB_OK);
			LocalFree(message_buffer_pointer);
			exit(98);
		}
		hook_address = (HOOKPROC) GetProcAddress(dll_handle, "win32_keyboard_press_hook");
		if (NULL == hook_address)
		{
			last_error = GetLastError();
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, last_error,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &message_buffer_pointer, 0, NULL);
			wsprintf(text_buffer, "Keyboard hook GetProcAddress failed with error %d: %s", last_error, message_buffer_pointer); 
			MessageBox(NULL, text_buffer, "Error", MB_OK);
			LocalFree(message_buffer_pointer);
			exit(97);
		}

		win32_keyboard_hook_handle = SetWindowsHookEx(WH_KEYBOARD_LL, hook_address, dll_handle, 0);
		if (NULL == win32_keyboard_hook_handle)
		{
			last_error = GetLastError();
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, last_error,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &message_buffer_pointer, 0, NULL);
			wsprintf(text_buffer, "Keyboard hooking failed with error %d: %s", last_error, message_buffer_pointer); 
			MessageBox(NULL, text_buffer, "Error", MB_OK);
			LocalFree(message_buffer_pointer);
			exit(99);	    
		}

		// Add a message to the status bar so the user gets visual feedback
		g_string_printf(tmp_gstring, "Wrote capture lock file '%s' and installed Control-Printscreen keyboard hook.", full_file_name);
		gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, tmp_gstring->str);
		gdk_flush();
	}
#else
	// Add a message to the status bar so the user gets visual feedback
	g_string_printf(tmp_gstring, "Wrote capture lock file '%s'.", full_file_name);
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, tmp_gstring->str);
	gdk_flush();
#endif

	// * Function clean up area *

	// Close the lock file
	g_key_file_free(lock_file);

	// Free the temporary GString
	g_string_free(tmp_gstring, TRUE);

	// Destroy the dialog box widget
	gtk_widget_destroy(GTK_WIDGET(capture_dialog));
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
