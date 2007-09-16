/*
 * $Id$
 *
 * Flame Project: Background capture process
 * 
 * Copyright (C) 2005-2007 Justin Clift <justin@postgresql.org>
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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

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

// Application constants
#define	APP_VERSION 0.51


#ifdef _WIN32
	// Windows global variables
	HDC			desktop_device_context;
	HDC			our_device_context;
	HBITMAP		our_bitmap;
#endif


#ifndef _WIN32
// Non-windows screenshot function
GdkPixbuf *non_win_take_screenshot(Window win, gint x_off, gint x_len, gint y_off, gint y_len)
{
	GdkWindow	*window;
	GdkPixbuf	*screenshot;

	window = gdk_window_foreign_new(win);
	if (NULL == window)
	{
		// Getting a GDKWindow identifier failed
		g_error("Error 01: Something went wrong when getting the window identifier");
		return NULL;
	}

	screenshot = gdk_pixbuf_get_from_drawable(NULL, window, NULL, x_off, y_off, 0, 0, x_len, y_len);
	if (NULL == screenshot)
	{
		// Getting the screenshot failed
		g_error("Error 03: Something went wrong when getting the screenshot");
		return NULL;
	}

	return screenshot;
}
#else
// Windows screenshot function
HBITMAP win_take_screenshot(HWND desktop_window_handle, gint x_off, gint x_len, gint y_off, gint y_len)
{
	// Create bitmap of requested desktop area
	desktop_device_context = GetDC(desktop_window_handle);
	our_device_context = CreateCompatibleDC(desktop_device_context);
	our_bitmap = CreateCompatibleBitmap(desktop_device_context, x_len, y_len);
	SelectObject(our_device_context, our_bitmap);
	BitBlt(our_device_context, 0, 0, x_off, y_len, desktop_device_context, x_off, y_off, SRCCOPY);
	
	return	our_bitmap;
}
#endif


// The main program loop
gint main(gint argc, gchar *argv[])
{
	// Local variables
	GKeyFile			*lock_file;			// Pointer to the lock file structure
	gchar				*name, *directory;	// Strings from the lock file
	gint				x_offset, x_length;	// Values from the lock file
	gint				y_offset, y_length;	// Values from the lock file

	GDir				*dir_ptr;			// Pointer to the directory entry structure
	gchar				*full_file_name;	// Holds the fully worked out file name to save as
	const gchar			*dir_entry;			// Holds a file name
	GSList				*entries = NULL;	// Holds a list of screen shot file names
	gboolean			screenshots_exist = FALSE;	// Switch to track if other screenshots already exist
	gchar				suffix[10];			// Holds the screenshot file suffix

	GError				*error = NULL;		// Pointer to error return structure

	gchar				tmp_string[1024];	// fixme4: Potential buffer overflow hazard?
	gpointer			tmp_ptr;			// Temporary pointer

#ifndef _WIN32
	// Non-windows only variables
	static GdkPixbuf	*screenshot = NULL;	// Holds the screen shot image
	Window				win;				// Holds the Window ID we're screenshot-ing
#else
	// Windows only variables
	png_text			flame_text;			// Pointer to structure holding Flame homepage URL
	FILE				*output_file_pointer;
	png_infop			png_info_pointer;
	png_structp			png_pointer;
	gchar				png_text_homepage[] = "The Flame Project: http://www.flameproject.org\0";
	gchar				png_text_key[] = "Software\0";
	HBITMAP				screenshot;			// Holds the screen shot image
	HWND				windows_win;		// Holds the Window ID we're screenshot-ing
#endif


	// Initialise GTK
	gtk_init(&argc, &argv);

	// Construct the fullly qualified path name for ~/.flame-lock
	tmp_ptr = (gchar *) g_get_home_dir();
	full_file_name = g_build_filename(tmp_ptr, ".flame-lock", NULL);

	// Look for a ~/.flame-lock file
	lock_file = g_key_file_new();  // Create a file object in memory
	if (!g_key_file_load_from_file(lock_file, full_file_name, G_KEY_FILE_NONE, NULL))
	{
		// * If no ~/.flame-lock file is found then exit *
		g_free(full_file_name);
		exit(1);
	}

	// g_build_filename() requires the returned string to be g_free'd
	g_free(full_file_name);

	// Read parameters from the lock file
	name = g_key_file_get_string(lock_file, "Project", "Name", NULL);  // Name of project
	directory = g_key_file_get_string(lock_file, "Project", "Directory", NULL);  // Directory to save screenshots in
	x_offset = g_key_file_get_integer(lock_file, "Project", "X_Offset", NULL);  // Top left X coordinate of screen area
	x_length = g_key_file_get_integer(lock_file, "Project", "X_Length", NULL);  // Width of screen area to grab
	y_offset = g_key_file_get_integer(lock_file, "Project", "Y_Offset", NULL);  // Top left Y coordinate of screen area
	y_length = g_key_file_get_integer(lock_file, "Project", "Y_Length", NULL);  // Height of screen area to grab

	// * Other potentially useful things to include *
	// Which displays to grab, for multi-monitor display
	//   + Will prob have to give offsets and length for each then, rather than the x_offset as above
	//   + disp_0_x_offset, disp_0_x_length, disp_1_x_offset, disp_1_x_length, etc.
	// File type to save as?

	// Close the lock file
	g_key_file_free(lock_file);

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
		g_error("Error 02: Something went wrong when getting the screenshot");
		exit(2);
	}

	// Check if the output folder exists
	if (!(dir_ptr = g_dir_open(directory, 0, &error)))
	{
		// Something went wrong when opening the folder
		if (G_FILE_ERROR_NOENT != error->code)
		{
			// The error was something other than the folder not existing (which we can cope with)
			g_warning("Error 05: Something went wrong opening '%s': %s", directory, error->message);
			g_error_free(error);
			exit(3);
		}

		// The directory doesn't exist
		// fixme3: Add code to create the directory
		g_warning("Error 06: The target directory doesn't exist");
		g_error_free(error);
		exit(4);
	}

	// ** Work out the file name to save as **

	// * Scan the directory for the highest numbered screenshot file *
	// * Our screenshot will be named with a suffix one higher       *
	while ((dir_entry = g_dir_read_name(dir_ptr)) != NULL)
	{
		// Look for files starting with the same name as the screenshot
		if (g_str_has_prefix(dir_entry, name))
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
	g_stpcpy(suffix, "0001");

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
		g_stpcpy(suffix, highest_entry->data);

		// Extract just the required numeric part of the highest screenshot, then increment it
		tmp_strings = g_strsplit(suffix, ".png", 2);  // tmp_strings[0] will always have the first part of the file name
		tmp_strings2 = g_strsplit(tmp_strings[0], name, 2);  // tmp_strings2[1] has the numeric string we want
		tmp_int = atoi(tmp_strings2[1]);
		tmp_int++;
		g_sprintf(suffix, "%0*u", 4, tmp_int);

		// Free the NULL terminated arrays of strings		
		g_strfreev(tmp_strings);
		g_strfreev(tmp_strings2);
	}

	// Free the memory used by the GSList
	// fixme5: (commented out because it seemed to be causing a segfault on windows)
	// g_slist_free(entries);

	// Construct the screen shot file name
	tmp_ptr = g_stpcpy(tmp_string, name);
	tmp_ptr = g_stpcpy(tmp_ptr, suffix);
	tmp_ptr = g_stpcpy(tmp_ptr, ".png");

	full_file_name = g_build_filename(directory, tmp_string, NULL);

#ifndef _WIN32
	// Non-windows code to save the screenshot
	gdk_pixbuf_save(screenshot, full_file_name, "png", NULL, NULL);
#else
	// * Windows code to save the screenshot bitmap as a PNG file *

	// Initialise variables
	flame_text.compression = PNG_TEXT_COMPRESSION_NONE;
	flame_text.key = png_text_key;
	flame_text.text = png_text_homepage;
	flame_text.text_length = strlen(png_text_homepage);

	// Open screenshot output file for writing
	output_file_pointer = fopen(full_file_name, "wb");
	if (FALSE == output_file_pointer)
	{
		// Something went wrong when opening the output file
		g_warning("Error 09: Something went wrong when opening the output screenhot file for writing");
		exit(7);
	}

	png_pointer = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (NULL == png_pointer)
	{
		// Something went wrong when creating the png write structure
		g_warning("Error 10: Something went wrong when creating the png write structure");
		exit(8);
	}

	png_info_pointer = png_create_info_struct(png_pointer);
	if (NULL == png_info_pointer)
	{
		// Something went wrong when creating the png info structure

		// Clean up
		png_destroy_write_struct(&png_pointer, (png_infopp) NULL);

		// Generate a warning then exit
		g_warning("Error 11: Something went wrong when creating the png info structure");
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

	// Create an in-memory row structure of the screenshot
// fixme1: Needs to be written

	// Point the information structure to the in-memory bitmap data
// fixme1: Needs to be written

	// Embed a text comment with Flame's home page
	png_set_text(png_pointer, png_info_pointer, &flame_text, 1);

	// Write the PNG data to the output file
	png_write_png(png_pointer, png_info_pointer, PNG_TRANSFORM_IDENTITY, NULL);

	// Free memory
	ReleaseDC(windows_win, desktop_device_context);
	DeleteDC(our_device_context);
	DeleteObject(our_bitmap);
#endif

	// g_build_filename() requires the returned string to be g_free'd
	g_free(full_file_name);

	// Exit
	exit(0);
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.7  2007/09/16 10:15:41  vapour
 * Re-writing the windows file saving code to use libpng.
 *
 * Revision 1.6  2007/09/11 13:51:24  vapour
 * Adjusted to save as jpeg files on windows.  Doesn't seem to be converting the bitmap to a gdk pixbuf properly though.
 *
 * Revision 1.5  2007/09/11 13:18:42  vapour
 * Started adding code to do screenshots in windows.  Not yet functional.
 *
 * Revision 1.4  2006/12/26 09:26:23  vapour
 * Updated to no longer run the hard coded old gnome screenshot application if there are no default settings specified.
 *
 * Revision 1.3  2006/04/21 17:53:51  vapour
 * Fixed the copyright notice, to include 2005, and remove the comments added through my local CVS repository.
 *
 * Revision 1.2  2006/04/21 17:51:04  vapour
 * + Updated header with clearer copyright and license details.
 * + Moved the History section to the end of the file.
 *
 * Revision 1.1  2006/04/13 15:57:49  vapour
 * Initial version, copied from my local CVS repository.
 *
 */
