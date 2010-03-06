/*
 * $Id$
 *
 * Salasaga: Function called when the user selects Screenshots -> Capture full screen from the top menu
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
#include <string.h>

// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only includes
	#include <windows.h>
	#include "salasaga_keycapture.h"
#endif

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../dialog/display_warning.h"


void menu_screenshots_capture_full_screen(void)
{
	// Local variables
	GdkScreen			*default_screen;			// The screen we'll be taking screenshots of
	GError				*error = NULL;				// Pointer to error return structure
	gchar				*full_file_name;			// Holds the fully worked out file name to save as
	GKeyFile			*lock_file;					// Pointer to the lock file structure
	GString				*message;					// Used to construct message strings
	GIOChannel			*output_file;				// The output file handle
	GIOStatus			return_value;				// Return value used in most GIOChannel functions
	gchar				*tmp_gchar;					// Temporary gchar
	gsize				tmp_gsize;					// Temporary gsize
	gpointer			tmp_ptr;					// Temporary pointer
	gint				win_height;					// The height to capture with
	gint				win_width;					// The width to capture with


	// Initialise various things
	message = g_string_new(NULL);

#ifndef _WIN32
	// Variables used by the non-windows code
	GtkWidget			*message_dialog;			// Dialog box for messages to the user


	// Retrieve the default screen information
	default_screen = gdk_screen_get_default();
	if (NULL == default_screen)
	{
		g_string_printf(message, "%s ED378: %s", _("Error"), _("Unable to retrieve details for the screen to capture."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		return;
	}

	// Retrieve the dimensions of the screen
	win_width = gdk_screen_get_width(GDK_SCREEN(default_screen));
	win_height = gdk_screen_get_height(GDK_SCREEN(default_screen));

	// Display debugging info
	if (get_debug_level())
	{
		printf(_("Window geometry retrieved. Width: %d\tHeight: %d\n"), win_width, win_height);
	}

	// Use the returned offset and dimensions
	set_capture_x((guint) 0);
	set_capture_y((guint) 0);
	set_capture_width((guint) win_width);
	set_capture_height((guint) win_height);

	// Let the user know that the window they selected has been successfully grabbed
	message_dialog = gtk_message_dialog_new(GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
			_("Full screen capture set.  Right click on the status bar icon to take screenshots, then Import when finished."));
	gtk_dialog_run(GTK_DIALOG(message_dialog));
	gtk_widget_destroy(message_dialog);

	// Update the status icon
	gtk_status_icon_set_tooltip_text(status_icon, _("Salasaga - Capture active"));

#else
	// * Pop open a dialog box asking the user for the offset and size of capture area *
	// (this is the old, manual way of getting the screen area to capture)

	// Variables used by the windows only code
	guint				row_counter = 0;			// Used to count which row things are up to

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


	// Create the dialog window, and table to hold its children
	capture_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(_("Capture screenshots"), GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL));
	capture_table = gtk_table_new(3, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(capture_dialog->vbox), GTK_WIDGET(capture_table), FALSE, FALSE, 10);

	// Create the label asking for the X Offset
	x_offset_label = gtk_label_new(_("X Offset: "));
	gtk_misc_set_alignment(GTK_MISC(x_offset_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(capture_table), GTK_WIDGET(x_offset_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the new X Offset data
	x_offset_button = gtk_spin_button_new_with_range(0, project_width, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(x_offset_button), capture_x);
	gtk_table_attach(GTK_TABLE(capture_table), GTK_WIDGET(x_offset_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the Y Offset
	y_offset_label = gtk_label_new(_("Y Offset: "));
	gtk_misc_set_alignment(GTK_MISC(y_offset_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(capture_table), GTK_WIDGET(y_offset_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the new Y Offset data
	y_offset_button = gtk_spin_button_new_with_range(0, project_height, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(y_offset_button), capture_y);
	gtk_table_attach(GTK_TABLE(capture_table), GTK_WIDGET(y_offset_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Which monitor are we displaying on?
	which_screen = gtk_window_get_screen(GTK_WINDOW(main_window));

	// Create the label asking for the X Length
	x_length_label = gtk_label_new(_("Width: "));
	gtk_misc_set_alignment(GTK_MISC(x_length_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(capture_table), GTK_WIDGET(x_length_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the new X Length data
	x_length_button = gtk_spin_button_new_with_range(0, gdk_screen_get_width(which_screen), 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(x_length_button), capture_width);
	gtk_table_attach(GTK_TABLE(capture_table), GTK_WIDGET(x_length_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the Y Length
	y_length_label = gtk_label_new(_("Height: "));
	gtk_misc_set_alignment(GTK_MISC(y_length_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(capture_table), GTK_WIDGET(y_length_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);;

	// Create the entry that accepts the new Y Length data
	y_length_button = gtk_spin_button_new_with_range(0, gdk_screen_get_height(which_screen), 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(y_length_button), capture_height);
	gtk_table_attach(GTK_TABLE(capture_table), GTK_WIDGET(y_length_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label that tells the user about Control-Printscreen
	key_combo_label = gtk_label_new(_("Hint: Use the Control-Printscreen key\ncombination to capture screenshots,\nthen the Import button to import them."));
	gtk_misc_set_alignment(GTK_MISC(key_combo_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(capture_table), GTK_WIDGET(key_combo_label), 0, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

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

	// Retrieve the values given by the user
	capture_x = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(x_offset_button));
	capture_y = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(y_offset_button));
	capture_width = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(x_length_button));
	capture_height = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(y_length_button));

	// Destroy the dialog box widget
	gtk_widget_destroy(GTK_WIDGET(capture_dialog));
#endif

	// Construct the fullly qualified path name for ~/.salasaga-lock file (to hold capture settings in)
	tmp_ptr = (gchar *) g_get_home_dir();
	full_file_name = g_build_filename(tmp_ptr, ".salasaga-lock", NULL);

	// Create the contents of the ~/.salasaga-lock file in memory
	lock_file = g_key_file_new();
	g_key_file_set_string(lock_file, "Project", "Name", _("screenshot"));  // Name to give screenshot files
	g_key_file_set_string(lock_file, "Project", "Directory", screenshots_folder->str);  // Directory to save screenshots in
	g_key_file_set_integer(lock_file, "Project", "X_Offset", get_capture_x());  // Top left X coordinate of screen area
	g_key_file_set_integer(lock_file, "Project", "X_Length", get_capture_width());  // Width of screen area to grab
	g_key_file_set_integer(lock_file, "Project", "Y_Offset", get_capture_y());  // Top left Y coordinate of screen area
	g_key_file_set_integer(lock_file, "Project", "Y_Length", get_capture_height());  // Height of screen area to grab
	g_key_file_set_integer(lock_file, "Project", "Screenshot_Delay", screenshot_delay_time);  // Number of seconds to delay the screenshot capture

	// Create IO channel for writing to
	output_file = g_io_channel_new_file(full_file_name, "w", &error);
	if (NULL == output_file)
	{
		// * An error occured when opening the file for writing, so alert the user, and return to the calling routine indicating failure *

		// Display a warning message using our function
		g_string_printf(message, "%s ED16: ", _("Error"));
		g_string_append_printf(message, _("An error '%s' occurred when opening '%s' for writing."), error->message, full_file_name);
		display_warning(message->str);

		// Free the memory allocated in this function
		g_string_free(message, TRUE);
		g_error_free(error);

		return;
	}

	// Write the ~/.salasaga-lock file to disk
	tmp_gchar = g_key_file_to_data(lock_file, NULL, NULL);
	return_value = g_io_channel_write_chars(output_file, tmp_gchar, strlen(tmp_gchar), &tmp_gsize, &error);
	if (G_IO_STATUS_ERROR == return_value)
	{
		// * An error occured when writing to the output file, so alert the user, and return to the calling routine indicating failure *
		g_string_printf(message, "%s ED17: ", _("Error"));
		g_string_append_printf(message, _("An error '%s' occurred when writing data to the '%s' file."), error->message, full_file_name);
		display_warning(message->str);

		// Free the memory allocated in this function
		g_string_free(message, TRUE);
		g_error_free(error);

		return;
	}

	// Close the IO channel
	return_value = g_io_channel_shutdown(output_file, TRUE, &error);
	if (G_IO_STATUS_ERROR == return_value)
	{
		// * An error occured when closing the output file, so alert the user, and return to the calling routine indicating failure *
		g_string_printf(message, "%s ED18: ", _("Error"));
		g_string_append_printf(message, _("An error '%s' occurred when closing the output file '%s'."), error->message, full_file_name);
		display_warning(message->str);

		// Free the memory allocated in this function
		g_string_free(message, TRUE);
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
		dll_handle = LoadLibrary((LPCTSTR) "libsalasaga-keycapture.dll");
		if (NULL == dll_handle)
		{
			last_error = GetLastError();
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, last_error,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &message_buffer_pointer, 0, NULL);
			wsprintf(text_buffer, _("Keyboard hook LoadLibrary failed with error %d: %s"), last_error, message_buffer_pointer);
			MessageBox(NULL, text_buffer, _("Error", MB_OK);
			LocalFree(message_buffer_pointer);
			exit(98);
		}
		hook_address = (HOOKPROC) GetProcAddress(dll_handle, "win32_keyboard_press_hook");
		if (NULL == hook_address)
		{
			last_error = GetLastError();
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, last_error,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &message_buffer_pointer, 0, NULL);
			wsprintf(text_buffer, _("Keyboard hook GetProcAddress failed with error %d: %s"), last_error, message_buffer_pointer);
			MessageBox(NULL, text_buffer, _("Error", MB_OK);
			LocalFree(message_buffer_pointer);
			exit(97);
		}

		win32_keyboard_hook_handle = SetWindowsHookEx(WH_KEYBOARD_LL, hook_address, dll_handle, 0);
		if (NULL == win32_keyboard_hook_handle)
		{
			last_error = GetLastError();
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, last_error,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &message_buffer_pointer, 0, NULL);
			wsprintf(text_buffer, _("Keyboard hooking failed with error %d: %s"), last_error, message_buffer_pointer);
			MessageBox(NULL, text_buffer, _("Error", MB_OK);
			LocalFree(message_buffer_pointer);
			exit(99);
		}

		// Add a message to the status bar so the user gets visual feedback
		g_string_printf(message, _(" Wrote capture settings file - %s - and installed Control-Printscreen keyboard hook"), full_file_name);
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(status_bar), message->str);
		gdk_flush();
	}
#else
	// Add a message to the status bar so the user gets visual feedback
	g_string_printf(message, " %s - %s", _("Wrote screenshot settings file"), full_file_name);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(status_bar), message->str);
	gdk_flush();
#endif

	// * Function clean up area *

	// Close the lock file
	g_key_file_free(lock_file);

	// Free the memory used in this function
	g_string_free(message, TRUE);
}
