/*
 * $Id$
 *
 * Flame Project: Function called when the user selects Screenshots -> Capture from the top menu
 * 
 * Copyright (C) 2007-2008 Justin Clift <justin@postgresql.org>
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
#include <unistd.h>
#include <string.h>

// GTK includes
#include <gtk/gtk.h>

#ifndef _WIN32
	// GDK X11 (non-windows only) include
	#include <gdk/gdkx.h>
#else
	// Windows only includes
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

	gchar				*tmp_gchar;					// Temporary gchar
	gsize				tmp_gsize;					// Temporary gsize
	gpointer			tmp_ptr;					// Temporary pointer
	GString				*tmp_gstring;				// Temporary string

	GKeyFile			*lock_file;					// Pointer to the lock file structure


	// Initialise various things
	tmp_gstring = g_string_new(NULL);

#ifndef _WIN32
	// Variables used by the non-windows code
	guint				border_width;
	Window				capture_window;				// The window the user selected
	int					mouse_buttons_pressed;
	Cursor				new_cursor;					// The new cursor to display while the user is choosing
	XEvent				new_event;
	gint				return_code_int;
	Display				*x_display;
	gint				x_offset;					// X offset to start capturing from
	Window				x_root_window;
	gint				x_screen_num;
	gint				y_offset;					// Y offset to start capturing from
	guint				win_depth;
	guint				win_height;					// The height to capture with
	guint				win_width;					// The width to capture with
	Status				window_status;

	GtkWidget			*message_dialog;			// Dialog box for messages to the user


	// Initialise various things
	capture_window = None;
	mouse_buttons_pressed = 0;

	// Display a message to the user, asking them to click on the window they want to capture
	message_dialog = gtk_message_dialog_new(GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
											"Please click on the window you want to capture (after closing this dialog box).");
	gtk_dialog_run(GTK_DIALOG(message_dialog));
	gtk_widget_destroy(message_dialog);

	// Minimise Flame so the user can choose another application
	gtk_window_iconify(GTK_WINDOW(main_window));

	// * Determine the area of screen covered by the window that the user wants to capture *

	// Get the default X11 display window and screen
	x_display = gdk_x11_get_default_xdisplay();
	x_screen_num = DefaultScreen(x_display);
	x_root_window = RootWindow(x_display, x_screen_num);

	// Prepare a new mouse cursor to use
	new_cursor = XCreateFontCursor(x_display, GDK_QUESTION_ARROW);
	if (None == new_cursor)
	{
		display_warning("Error ED112: Unable to create alternative cursor.");
	}

	// Flush the X11 output buffer and process events
	XSync(x_display, False);

	// Take control of the mouse pointer
	return_code_int = XGrabPointer(x_display,					// X11 display to grab on
						x_root_window,							// Grab on the root window
						False,									// Unsure exactly.  I think it means whether to filter events via the mask (?)
						ButtonPressMask | ButtonReleaseMask,	// The events to report (the mask)
						GrabModeSync,
						GrabModeAsync,
						None,
						new_cursor,								// We use a different mouse cursor, to re-inforce to the user that something interesting is happening
						CurrentTime);
	if (GrabSuccess != return_code_int)
	{
		display_warning("Error ED112: Unable to grab the mouse pointer for selected display.  Please report this error!");
		return;
    }

	// Display debugging info
	if (debug_level)
	{
		printf("Mouse pointer grabbed.\n");
	}

	// Wait for the user to select a window for capture
	while (None == capture_window || 0 != mouse_buttons_pressed)
	{
		// Get the next events
		XAllowEvents(x_display, SyncPointer, CurrentTime);

		// Look for the events we want
		XWindowEvent(x_display, x_root_window, ButtonPressMask | ButtonReleaseMask, &new_event);

		// Work out which of the acceptable event types we received
		switch (new_event.type)
		{
			case ButtonPress:
				// We received a button press event, so retrieve the associated window id
				capture_window = new_event.xbutton.subwindow;

				// Display debugging info
				if (debug_level)
				{
					printf("Capture window set.\n");
				}

				// Increase the count of mouse buttons pressed
				mouse_buttons_pressed++;

				continue;

			case ButtonRelease:
				// We received a button release event, so decrease the count of buttons pressed
				if (0 < mouse_buttons_pressed)
					mouse_buttons_pressed--;

				continue;
		}
	}

	// Release control of the mouse pointer
	XUngrabPointer(x_display, CurrentTime);

	// Display debugging info
	if (debug_level)
	{
		printf("Mouse pointer grab released.\n");
	}

	// Flush the X11 output buffer and process events
	XSync(x_display, False);

	// Retrieve the geometry for the window
	window_status = XGetGeometry(x_display,
						capture_window,
						&x_root_window,
						&x_offset,
						&y_offset,
						&win_width,
						&win_height,
						&border_width,
						&win_depth);

	// Display debugging info
	if (debug_level)
	{
		printf("Window geometry retrieved. X offset: %d\tY offset: %d\tWidth: %d\tWeight: %d\n", x_offset, y_offset, win_width, win_height);
	}

	// Use the returned offset and dimensions
	capture_x = (guint) x_offset;
	capture_y = (guint) y_offset;
	capture_width = (guint) win_width;
	capture_height = (guint) win_height;

	// Let the user know that the window they selected has been successfully grabbed
	message_dialog = gtk_message_dialog_new(GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
			"Window dimensions successfully retrieved.  Please use Control-Printscreen to take screenshots then Import when finished.");
	gtk_dialog_run(GTK_DIALOG(message_dialog));
	gtk_widget_destroy(message_dialog);

	// Free the mouse cursor object
	XFreeCursor(x_display, new_cursor);

#else
	// * Pop open a dialog box asking the user for the offset and size of capture area *
	// (this is the old, manual way of getting the screen area to capture)

	// Variables used by the windows only code
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

	// Retrieve the values given by the user
	capture_x = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(x_offset_button));
	capture_y = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(y_offset_button));
	capture_width = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(x_length_button));
	capture_height = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(y_length_button));

	// Destroy the dialog box widget
	gtk_widget_destroy(GTK_WIDGET(capture_dialog));
#endif

	// Construct the fullly qualified path name for ~/.flame-lock file (to hold capture settings in)
	tmp_ptr = (gchar *) g_get_home_dir();
	full_file_name = g_build_filename(tmp_ptr, ".flame-lock", NULL);

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
		// * An error occured when writing to the output file, so alert the user, and return to the calling routine indicating failure *
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
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.6  2008/02/04 06:40:04  vapour
 * Mouse cursor is changed from default while selection operation is in progress, to re-inforce whats going on.
 *
 * Revision 1.5  2008/02/04 06:07:06  vapour
 * Added new code, X11 only, that allows the user to select the window they want to capture rather than typing in manual coordinates.
 *
 * Revision 1.4  2008/02/03 05:45:24  vapour
 * Fixed a typo in a comment.
 *
 * Revision 1.3  2008/01/15 16:19:00  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.2  2007/10/06 11:38:28  vapour
 * Continued adjusting function include definitions.
 *
 * Revision 1.1  2007/09/29 04:22:15  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */
