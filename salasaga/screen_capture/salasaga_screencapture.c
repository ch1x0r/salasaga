/*
 * $Id$
 *
 * Salasaga: Background screen capture process
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

// Standard includes
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <libnotify/notify.h>

// GTK includes
#include <gtk/gtk.h>

// GLib string functions
#include <glib/gprintf.h>

#ifndef _WIN32
	// Non-windows code (GDK X functions)
	#include <gdk/gdkx.h>
#else
	// Windows only code
	#include <windows.h>
#endif

// PNG include
#include <png.h>

// Add our functions
#include "../config.h"
#include "salasaga_types.h"
#include "display_warning.h"
#include "validate_value.h"


#ifdef _WIN32
	// Windows global variables
	HDC			desktop_device_context;
	HBITMAP		our_bitmap;
	HDC			our_device_context;
#endif


#ifndef _WIN32
// Non-windows screenshot function
GdkPixbuf *non_win_take_screenshot(Window win, gint x_off, gint x_len, gint y_off, gint y_len)
{
	// Local variable(s)
	GString				*message;					// Used to construct message strings
	GdkPixbuf			*screenshot;
	GdkWindow			*window;


	// Initialisation
	message = g_string_new(NULL);

	window = gdk_window_foreign_new(win);
	if (NULL == window)
	{
		// Getting a GDKWindow identifier failed
		g_string_printf(message, "%s CA01: %s", _("Error"), _("Something went wrong when getting the window identifier."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		return NULL;
	}

	screenshot = gdk_pixbuf_get_from_drawable(NULL, window, NULL, x_off, y_off, 0, 0, x_len, y_len);
	if (NULL == screenshot)
	{
		// Getting the screenshot failed
		g_string_printf(message, "%s CA03: %s", _("Error"), _("Something went wrong when getting the screenshot."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		return NULL;
	}

	// Free memory used in this function
	g_string_free(message, TRUE);

	return screenshot;
}
#else
// Windows screenshot function
HBITMAP win_take_screenshot(HWND desktop_window_handle, gint x_off, gint x_len, gint y_off, gint y_len)
{
	// Local variables for error handling
	GString				*message;					// Used to construct message strings
	BOOL				return_code_bool;
	HGDIOBJ				return_code_hgdiobj;


	// Initialise other bits
	message = g_string_new(NULL);

	// Create bitmap of requested desktop area
	desktop_device_context = GetDC(desktop_window_handle);
	if (NULL == desktop_device_context)
	{
		// GetDC failed
		g_string_printf(message, "%s CA21: %s", _("Error"), _("Screenshot GetDC failed."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		exit(19);
	}
	our_device_context = CreateCompatibleDC(desktop_device_context);
	if (NULL == our_device_context)
	{
		// CreateCompatibleDC failed
		g_string_printf(message, "%s CA20: %s", _("Error"), _("Screenshot CreateCompatibleDC failed."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		exit(18);
	}
	our_bitmap = CreateCompatibleBitmap(desktop_device_context, x_len, y_len);
	if (NULL == our_bitmap)
	{
		// CreateCompatibleBitmap failed
		g_string_printf(message, "%s CA19: %s", _("Error"), _("Screenshot CreateCompatibleBitmap failed."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		exit(17);
	}
	return_code_hgdiobj = SelectObject(our_device_context, our_bitmap);
	if (NULL == return_code_hgdiobj)
	{
		// Selected object is not a region
		g_string_printf(message, "%s CA17: %s", _("Error"), _("Screenshot SelectObject failed: selected object is not a region."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		exit(15);
	}
	if (HGDI_ERROR == return_code_hgdiobj)
	{
		// SelectObject failed
		g_string_printf(message, "%s CA18: %s", _("Error"), _("Screenshot SelectObject failed: unknown error."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		exit(16);
	}
	return_code_bool = BitBlt(our_device_context, 0, 0, x_len, y_len, desktop_device_context, x_off, y_off, SRCCOPY);
	if (FALSE == return_code_bool)
	{
		// BitBlt failed
		g_string_printf(message, "%s CA14: %s", _("Error"), _("Screenshot BitBlt failed."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		exit(12);
	}

	// Free memory used in this function
	g_string_free(message, TRUE);

	return our_bitmap;
}
#endif


// The main program loop
gint main(gint argc, gchar *argv[])
{
	// Local variables
	gint				delay_counter;				// Simple loop counter
	const gchar			*dir_entry;					// Holds a file name
	GDir				*dir_ptr;					// Pointer to the directory entry structure
	GString				*directory;					// Name of the folder to save the screenshot in
	GSList				*entries = NULL;			// Holds a list of screen shot file names
	GError				*error = NULL;				// Pointer to error return structure
	gchar				*full_file_name;			// Holds the fully worked out file name to save as
	gboolean			libnotify_is_notify_osd = FALSE;  // Indicates whether the libnotify server is Notify-OSD
	GKeyFile			*lock_file;					// Pointer to the lock file structure
	GString				*message;					// Used to construct message strings
	GString				*name;						// Project name from the lock file
	gchar				*notify_server_name;		// Receives the name of the libnotify server
	gchar				*notify_server_vendor;		// Receives the vendor name of the libnotify server
	gchar				*notify_server_version;		// Receives the version string of the libnotify server
	gchar				*notify_server_spec_version;  // Receives the libnotify spec of the libnotify server
	gboolean			return_code_gboolean;		// Receives a boolean return code from the libnotify server
	gint				screenshot_delay = 5;		// The number of seconds to delay before triggering a screenshot
	gboolean			screenshots_exist = FALSE;	// Switch to track if other screenshots already exist
	GString				*short_file_name;			// Name of the file to save as
	NotifyNotification	*status_notify = NULL;		// Status message
	GString				*suffix;					// Holds the screenshot file suffix
	gpointer			tmp_ptr;					// Temporary pointer
	gboolean			usable_input;				// Used as a flag to indicate if all validation was successful
	GString				*valid_project_name;		// Receives the project name once validated
	GString				*valid_screenshot_folder;	// Receives the screenshot folder once validated
	guint				valid_height = 0;			// Receives the screenshot height once validated
	guint				valid_width = 0;			// Receives the screenshot width once validated
	guint				valid_screenshot_delay = 0;	// Receives the screenshot delay time once validated
	guint				valid_x_offset = 0;			// Receives the screenshot x offset once validated
	guint				valid_y_offset = 0;			// Receives the screenshot y offset once validated
	guint				*validated_guint;			// Receives known good guint values from the validation function
	GString				*validated_string;			// Receives known good strings from the validation function
	gint				x_offset, x_length;			// Values from the lock file
	gint				y_offset, y_length;			// Values from the lock file


#ifndef _WIN32
	// Non-windows only variables
	static GdkPixbuf	*screenshot = NULL;			// Holds the screen shot image
	Window				win;						// Holds the Window ID we're screenshot-ing
#else
	// Windows only variables
	png_text			salasaga_text;				// Pointer to structure holding Salasaga homepage URL
	gint				num_bytes;
	FILE				*output_file_pointer;
	png_infop			png_info_pointer;
	png_structp			png_pointer;
	gchar				png_text_homepage[] = "Salasaga: http://www.salasaga.org\0";
	gchar				png_text_key[] = "Software\0";
	gint				return_code_int;			// Holds integer return codes
	gint				row_counter;
	png_byte			**row_pointers;				// Points to an array of pointers for screenshot byte rows
	HBITMAP				screenshot;					// Holds the screen shot image
	gpointer			screenshot_copy;			// Holds a copy of the screen shot image
	BITMAP				screenshot_data;			// Metadata about the screenshot (width, height, etc)
	HWND				windows_win;				// Holds the Window ID we're screenshot-ing
#endif


	// Initialse i18n
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	// Initialise GTK
	gtk_init(&argc, &argv);
	valid_project_name = g_string_new(NULL);
	valid_screenshot_folder = g_string_new(NULL);

	// Initialise notification
	notify_init("Salasaga");

	// Initialise other bits
	message = g_string_new(NULL);

	// Construct the fullly qualified path name for ~/.salasaga-lock
	tmp_ptr = (gchar *) g_get_home_dir();
	full_file_name = g_build_filename(tmp_ptr, ".salasaga-lock", NULL);

	// Look for a ~/.salasaga-lock file
	lock_file = g_key_file_new();  // Create a file object in memory
	if (!g_key_file_load_from_file(lock_file, full_file_name, G_KEY_FILE_NONE, NULL))
	{
		// * If no ~/.salasaga-lock file is found then exit *
		g_free(full_file_name);
		exit(1);
	}

	// g_build_filename() requires the returned string to be g_free'd
	g_free(full_file_name);

	// ** Read parameters from the lock file **

	// Reset the usable input flag
	usable_input = TRUE;

	// Validate the project name input
	validated_string = validate_value(PROJECT_NAME, V_CHAR, g_key_file_get_string(lock_file, "Project", "Name", NULL));
	if (NULL == validated_string)
	{
		g_string_printf(message, "%s CA22: %s", _("Error"), _("There was something wrong with the project name value.  Aborting screenshot."));
		display_warning(message->str);
		usable_input = FALSE;
	} else
	{
		valid_project_name = g_string_assign(valid_project_name, validated_string->str);
		g_string_free(validated_string, TRUE);
		validated_string = NULL;
	}

	// Retrieve the new screenshot folder input
	validated_string = validate_value(FOLDER_PATH, V_CHAR, g_key_file_get_string(lock_file, "Project", "Directory", NULL));
	if (NULL == validated_string)
	{
		g_string_printf(message, "%s CA23: %s", _("Error"), _("There was something wrong with the screenshot folder value.  Aborting screenshot."));
		display_warning(message->str);
		usable_input = FALSE;
	} else
	{
		valid_screenshot_folder = g_string_assign(valid_screenshot_folder, validated_string->str);
		g_string_free(validated_string, TRUE);
		validated_string = NULL;
	}

	// Get the starting x offset
	validated_guint = validate_value(SCREENSHOT_X_OFFSET, V_CHAR, g_key_file_get_string(lock_file, "Project", "X_Offset", NULL));
	if (NULL == validated_guint)
	{
		g_string_printf(message, "%s CA24: %s", _("Error"), _("There was something wrong with the x offset start value.  Aborting screenshot."));
		display_warning(message->str);
		usable_input = FALSE;
	} else
	{
		valid_x_offset = *validated_guint;
		g_free(validated_guint);
	}

	// Get the starting y offset
	validated_guint = validate_value(SCREENSHOT_Y_OFFSET, V_CHAR, g_key_file_get_string(lock_file, "Project", "Y_Offset", NULL));
	if (NULL == validated_guint)
	{
		g_string_printf(message, "%s CA25: %s", _("Error"), _("There was something wrong with the y offset start value.  Aborting screenshot."));
		display_warning(message->str);
		usable_input = FALSE;
	} else
	{
		valid_y_offset = *validated_guint;
		g_free(validated_guint);
	}

	// Get the width
	validated_guint = validate_value(SCREENSHOT_WIDTH, V_CHAR, g_key_file_get_string(lock_file, "Project", "X_Length", NULL));
	if (NULL == validated_guint)
	{
		g_string_printf(message, "%s CA26: %s", _("Error"), _("There was something wrong with the width value.  Aborting screenshot."));
		display_warning(message->str);
		usable_input = FALSE;
	} else
	{
		valid_width = *validated_guint;
		g_free(validated_guint);
	}

	// Get the height
	validated_guint = validate_value(SCREENSHOT_HEIGHT, V_CHAR, g_key_file_get_string(lock_file, "Project", "Y_Length", NULL));
	if (NULL == validated_guint)
	{
		g_string_printf(message, "%s CA27: %s", _("Error"), _("There was something wrong with the height value.  Aborting screenshot."));
		display_warning(message->str);
		usable_input = FALSE;
	} else
	{
		valid_height = *validated_guint;
		g_free(validated_guint);
	}

	// Get the screenshot delay
	validated_guint = validate_value(SCREENSHOT_DELAY, V_CHAR, g_key_file_get_string(lock_file, "Project", "Screenshot_Delay", NULL));
	if (NULL == validated_guint)
	{
		g_string_printf(message, "%s CA29: %s", _("Error"), _("There was something wrong with the screenshot delay value.  Aborting screenshot."));
		display_warning(message->str);
		usable_input = FALSE;
	} else
	{
		valid_screenshot_delay = *validated_guint;
		g_free(validated_guint);
	}

	// Abort if any of the input isn't valid
	if (TRUE != usable_input)
	{
		exit(20);
	}

	// ** We only get here if all the input is considered valid **

	name = valid_project_name;  // Name of project
	directory = valid_screenshot_folder;  // Directory to save screenshots in
	x_offset = valid_x_offset;  // Top left X coordinate of screen area
	x_length = valid_width;  // Width of screen area to grab
	y_offset = valid_y_offset;  // Top left Y coordinate of screen area
	y_length = valid_height;  // Height of screen area to grab
	screenshot_delay = valid_screenshot_delay;  // Number of seconds to delay the screenshot by

	// * Other potentially useful things to include *
	// Which displays to grab, for multi-monitor display
	//   + Will prob have to give offsets and length for each then, rather than the x_offset as above
	//   + disp_0_x_offset, disp_0_x_length, disp_1_x_offset, disp_1_x_length, etc.
	// File type to save as?

	// Close the lock file
	g_key_file_free(lock_file);

	// Find out if we're running Notify-OSD or not
	// Note: Notify-OSD is a reduced functionality notify daemon that (unfortunately) Ubuntu
	// are strong arming people into using.  Instead of point blank refusing to use it, it's
	// probably better to just operate in a reduced functionality mode for notifications if
	// it's detected to be running
	return_code_gboolean = notify_get_server_info(&notify_server_name, &notify_server_vendor, &notify_server_version, &notify_server_spec_version);
	if (TRUE == return_code_gboolean)
	{
		if (0 == g_strcmp0(notify_server_name, "notify-osd"))
		{
			libnotify_is_notify_osd = TRUE;
		}
	}
	else
	{
		// We weren't able to retrieve the libnotify server info.  For now we just assume the libnotify server has the
		// capabilities we need
		libnotify_is_notify_osd = FALSE;
	}

	// We only do full onscreen notifications if the libnotify daemon supports timeout values,
	// because if it doesn't, the notification bubble itself remains onscreen is captured in
	// the screenshot (not what we want)
	if (FALSE == libnotify_is_notify_osd)
	{
		// If there is a delay of greater than 1 second, then do a count down
		if (1 < screenshot_delay)
		{
			g_string_printf(message, _("Screenshot in %u seconds"), screenshot_delay);
			status_notify = notify_notification_new(message->str, NULL, NULL, NULL);
		}

		// Delay for the requested number of seconds before the screenshot
		for (delay_counter = 0; delay_counter < screenshot_delay; delay_counter++)
		{
			// If appropriate, update the notification time
			if (1 < screenshot_delay)
			{
				// Display the updated count down time
				notify_notification_show(status_notify, &error);

				// Delay for 1/2 second
				g_usleep(500000);

				// Delay for another 1/2 second if this isn't the last update of the count down
				if (delay_counter < screenshot_delay - 1)
				{
					g_usleep(500000);
				}

				// Update the notification time
				g_string_printf(message, _("Screenshot in %u seconds"), screenshot_delay - delay_counter - 1);
				notify_notification_update(status_notify, message->str, NULL, NULL);

			} else
			{
				// Delay for 1 second
				g_usleep(1000000);
			}
		}

		// Remove the notification half a second before the screenshot is taken,
		// so the notification message can be captured in the screenshot itself
		if (1 < screenshot_delay)
		{
			// Remove the notification message
			notify_notification_close(status_notify, &error);

			// Delay for 1/2 second
			g_usleep(500000);
		}
	}
	else
	{
		// Notify-OSD is running, so we do a very quick notification bubble ("confirmation bubble") if there's sufficient time
		if (2 < screenshot_delay)
		{
			g_string_printf(message, _("Screenshot in %u seconds"), screenshot_delay);
			status_notify = notify_notification_new(message->str, NULL, NULL, NULL);
			notify_notification_set_hint_string(status_notify, "x-canonical-private-synchronous", "");
			notify_notification_show(status_notify, &error);
		}

		// Delay for the requested number of seconds before the screenshot
		for (delay_counter = 0; delay_counter < screenshot_delay; delay_counter++)
		{
			// Delay for 1 second
			g_usleep(1000000);
		}
	}

	// Take screenshot
#ifdef _WIN32
	// Windows only code
	windows_win = GetDesktopWindow();
	screenshot = win_take_screenshot(windows_win, x_offset, x_length, y_offset, y_length);
#else
	// Non-windows code
	win = GDK_ROOT_WINDOW();
	screenshot = non_win_take_screenshot(win, x_offset, x_length, y_offset, y_length);
#endif
	if (NULL == screenshot)
	{
		// Something went wrong getting the screenshot, so give an error and exit
		g_string_printf(message, "%s CA02: %s", _("Error"), _("Something went wrong when getting the screenshot."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		exit(2);
	}

	// * Visually let the user know the screenshot was taken *

	// Create a new status notification message
	status_notify = notify_notification_new(_("Screenshot taken"), NULL, NULL, NULL);
	if (TRUE == libnotify_is_notify_osd)
	{
		// Notify-OSD needs special treatment in order to display a notify message for only
		// a short duration
		notify_notification_set_hint_string(status_notify, "x-canonical-private-synchronous", "");
	}
	else
	{
		// Set the timeout for the notification message
		notify_notification_set_timeout(status_notify, 2000);
	}

	// Display the notification message
	notify_notification_show(status_notify, &error);

	// Check if the output folder exists
	if (!(dir_ptr = g_dir_open(directory->str, 0, &error)))
	{
		// Something went wrong when opening the folder
		if (G_FILE_ERROR_NOENT != error->code)
		{
			// The error was something other than the folder not existing (which we can cope with)
			g_string_printf(message, "%s CA05: %s '%s': %s", _("Error"), _("Something went wrong opening"), directory->str, error->message);
			display_warning(message->str);
			g_string_free(message, TRUE);
			g_error_free(error);
			exit(3);
		}

		// The directory doesn't exist
		// fixme3: Add code to create the directory
		g_string_printf(message, "%s CA06: %s - '%s'", _("Error"), _("The target directory doesn't exist."), directory->str);
		display_warning(message->str);
		g_string_free(message, TRUE);
		g_error_free(error);
		exit(4);
	}

	// ** Work out the file name to save as **

	// * Scan the directory for the highest numbered screenshot file *
	// * Our screenshot will be named with a suffix one higher       *
	while ((dir_entry = g_dir_read_name(dir_ptr)) != NULL)
	{
		// Look for files starting with the same name as the screenshot
		if (g_str_has_prefix(dir_entry, name->str))
		{
			// The directory entry starts with the correct prefix, now let's check the file extension
			if (g_str_has_suffix(dir_entry, ".png"))
			{
				// The directory entry has the correct prefix too, so it's very likely one of our screenshots
				// We add it to the list of screenshot entries we know about
				entries = g_slist_append(entries, g_strdup_printf("%s", dir_entry));
				screenshots_exist = TRUE;
			}
		}
	}

	// Close the directory
	g_dir_close(dir_ptr);

	// Set the suffix to the default starting number
	suffix = g_string_new("0001");

	// If screenshots were found, then sort them and extract the highest numbered suffix
	if (TRUE == screenshots_exist)
	{
		// Very temporary variables
		GSList		*highest_entry;					// The highest numbered screen shot
		gchar		**tmp_strings, **tmp_strings2;	// Strings returned during splitting
		gint		tmp_int;						// Temporary integer

		entries = g_slist_sort(entries, (GCompareFunc) strcmp);

		// Get the highest numbered suffix (it's the last entry after sorting)
		highest_entry = g_slist_last(entries);
		suffix = g_string_assign(suffix, highest_entry->data);

		// Extract just the required numeric part of the highest screenshot, then increment it
		tmp_strings = g_strsplit(suffix->str, ".png", 2);  // tmp_strings[0] will always have the first part of the file name
		tmp_strings2 = g_strsplit(tmp_strings[0], name->str, 2);  // tmp_strings2[1] has the numeric string we want
		tmp_int = atoi(tmp_strings2[1]);
		tmp_int++;
		g_string_printf(suffix, "%0*u", 4, tmp_int);

		// Free the NULL terminated arrays of strings
		g_strfreev(tmp_strings);
		g_strfreev(tmp_strings2);
	}

	// Free the memory used by the GSList
	// fixme5: (commented out because it seemed to be causing a segfault on windows)
	// fixme5: (Note - This was probably resolved on 2008-02-04, but hasn't been tested on Windows yet)
	// g_slist_free(entries);

	// Construct the screenshot file name
	short_file_name = g_string_new(NULL);
	g_string_printf(short_file_name, "%s%s%s", name->str, suffix->str, ".png");
	full_file_name = g_build_filename(directory->str, short_file_name->str, NULL);

#ifndef _WIN32
	// Non-windows code to save the screenshot
	gdk_pixbuf_save(screenshot, full_file_name, "png", NULL, "tEXt::Software", "Salasaga: http://www.salasaga.org", NULL);
#else
	// * Windows code to save the screenshot bitmap as a PNG file *

	// Initialise variables
	salasaga_text.compression = PNG_TEXT_COMPRESSION_NONE;
	salasaga_text.key = png_text_key;
	salasaga_text.text = png_text_homepage;
	salasaga_text.text_length = strlen(png_text_homepage);

	// Open screenshot output file for writing
	output_file_pointer = fopen(full_file_name, "wb");
	if (FALSE == output_file_pointer)
	{
		// Something went wrong when opening the output file
		g_string_printf(message, "%s CA09: %s", _("Error"), _("Something went wrong when opening the output screenshot file for writing."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		exit(7);
	}

	png_pointer = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (NULL == png_pointer)
	{
		// Something went wrong when creating the png write structure
		g_string_printf(message, "%s CA10: %s", _("Error"), _("Something went wrong when creating the png write structure."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		exit(8);
	}

	png_info_pointer = png_create_info_struct(png_pointer);
	if (NULL == png_info_pointer)
	{
		// Something went wrong when creating the png info structure

		// Clean up
		png_destroy_write_struct(&png_pointer, (png_infopp) NULL);

		// Generate a warning then exit
		g_string_printf(message, "%s CA11: %s", _("Error"), _("Something went wrong when creating the png info structure."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		exit(9);
	}

	// Initialise the PNG output code
	png_init_io(png_pointer, output_file_pointer);

	// Fill in the PNG information structure
	png_set_IHDR(png_pointer, png_info_pointer,
						x_length,  // Width
						y_length,  // Height
						8,  // Bit depth
						PNG_COLOR_TYPE_RGB,  // Color type
						PNG_INTERLACE_NONE,  // Interlace type
						PNG_COMPRESSION_TYPE_DEFAULT,  // Compression type
						PNG_FILTER_TYPE_DEFAULT);  // Filter method

	// Set the correct pixel ordering
	png_set_bgr(png_pointer);

	// Embed a text comment with Salasaga's home page
	png_set_text(png_pointer, png_info_pointer, &salasaga_text, 1);

	// Get metadata about the screenshot
	return_code_int = GetObject(screenshot, sizeof(BITMAP), &screenshot_data);
	if (0 == return_code_int)
	{
		// Something went wrong when retrieving the screenshot metadata

		// Clean up
		png_destroy_write_struct(&png_pointer, &png_info_pointer);

		g_string_printf(message, "%s CA12: %s", _("Error"), _("Something went wrong when retrieving the screenshot metadata."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		exit(10);
	}

	// Copy the bitmap so we can use it
	num_bytes = screenshot_data.bmHeight * screenshot_data.bmWidthBytes;
	screenshot_copy = g_new(unsigned char, num_bytes);
	return_code_int = GetBitmapBits(screenshot, num_bytes, screenshot_copy);
	if (0 == return_code_int)
	{
		// Something went wrong when copying the screenshot data

		// Clean up
		png_destroy_write_struct(&png_pointer, &png_info_pointer);

		g_string_printf(message, "%s CA13: %s", _("Error"), _("Something went wrong when copying the screenshot data."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		exit(11);
	}

	// Create an in-memory row structure of the screenshot
	row_pointers = png_malloc(png_pointer, y_length * sizeof(png_bytep *));
	for (row_counter = 0; row_counter < screenshot_data.bmHeight; row_counter++)
	{
		row_pointers[row_counter] = screenshot_copy + (screenshot_data.bmWidthBytes * row_counter);
	}
	png_set_rows(png_pointer, png_info_pointer, row_pointers);

	// Write the PNG data to the output file
	png_write_info(png_pointer, png_info_pointer);
	png_set_filler(png_pointer, 0, PNG_FILLER_AFTER);
	png_set_packing(png_pointer);
	png_write_image(png_pointer, row_pointers);
	png_write_end(png_pointer, NULL);

	// Free memory used in PNG code
	png_destroy_write_struct(&png_pointer, &png_info_pointer);

	// Free memory used in screen capturing code
	ReleaseDC(windows_win, desktop_device_context);
	DeleteDC(our_device_context);
	DeleteObject(our_bitmap);
#endif

	// g_build_filename() requires the returned string to be g_free'd
	g_free(full_file_name);

	// Free the GString's we allocated
	g_string_free(suffix, TRUE);
	g_string_free(short_file_name, TRUE);
	g_string_free(valid_project_name, TRUE);
	g_string_free(valid_screenshot_folder, TRUE);

	// Exit
	exit(0);
}
