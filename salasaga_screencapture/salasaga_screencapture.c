/*
 * $Id$
 *
 * Flame Project: Background capture process
 * 
 * Copyright (C) 2005-2008 Justin Clift <justin@postgresql.org>
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
#define	APP_VERSION 0.52


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
	GdkPixbuf	*screenshot;
	GdkWindow	*window;


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

	// Generate a beep
	gdk_beep();

	return screenshot;
}
#else
// Windows screenshot function
HBITMAP win_take_screenshot(HWND desktop_window_handle, gint x_off, gint x_len, gint y_off, gint y_len)
{
	// Local variables for error handling
	BOOL				return_code_bool;
	HGDIOBJ				return_code_hgdiobj;


	// Create bitmap of requested desktop area
	desktop_device_context = GetDC(desktop_window_handle);
	if (NULL == desktop_device_context)
	{
		// GetDC failed
		g_error("Error 21: Screenshot GetDC failed");
		exit(19);
	}
	our_device_context = CreateCompatibleDC(desktop_device_context);
	if (NULL == our_device_context)
	{
		// CreateCompatibleDC failed
		g_error("Error 20: Screenshot CreateCompatibleDC failed");
		exit(18);
	}
	our_bitmap = CreateCompatibleBitmap(desktop_device_context, x_len, y_len);
	if (NULL == our_bitmap)
	{
		// CreateCompatibleBitmap failed
		g_error("Error 19: Screenshot CreateCompatibleBitmap failed");
		exit(17);
	}
	return_code_hgdiobj = SelectObject(our_device_context, our_bitmap);
	if (NULL == return_code_hgdiobj)
	{
		// Selected object is not a region
		g_error("Error 17: Screenshot SelectObject failed: selected object is not a region");
		exit(15);
	}
	if (HGDI_ERROR == return_code_hgdiobj)
	{
		// SelectObject failed
		g_error("Error 18: Screenshot SelectObject failed: unknown error");
		exit(16);
	}
	return_code_bool = BitBlt(our_device_context, 0, 0, x_len, y_len, desktop_device_context, x_off, y_off, SRCCOPY);
	if (FALSE == return_code_bool)
	{
		// BitBlt failed
		g_error("Error 14: Screenshot BitBlt failed");
		exit(12);		
	}

	return our_bitmap;
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
	GString				*short_file_name;	// Name of the file to save as
	GString				*suffix;			// Holds the screenshot file suffix

	GError				*error = NULL;		// Pointer to error return structure

	gpointer			tmp_ptr;			// Temporary pointer

#ifndef _WIN32
	// Non-windows only variables
	static GdkPixbuf	*screenshot = NULL;	// Holds the screen shot image
	Window				win;				// Holds the Window ID we're screenshot-ing
#else
	// Windows only variables
	png_text			flame_text;			// Pointer to structure holding Flame homepage URL
	gint				num_bytes;
	FILE				*output_file_pointer;
	png_infop			png_info_pointer;
	png_structp			png_pointer;
	gchar				png_text_homepage[] = "The Flame Project: http://www.flameproject.org\0";
	gchar				png_text_key[] = "Software\0";
	gint				return_code_int;	// Holds integer return codes
	gint				row_counter;
	png_byte			**row_pointers;		// Points to an array of pointers for screenshot byte rows
	HBITMAP				screenshot;			// Holds the screen shot image
	gpointer			screenshot_copy;	// Holds a copy of the screen shot image
	BITMAP				screenshot_data;	// Metadata about the screenshot (width, height, etc)
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
		g_error("Error 06: The target directory doesn't exist");
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
		tmp_strings2 = g_strsplit(tmp_strings[0], name, 2);  // tmp_strings2[1] has the numeric string we want
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
	g_string_printf(short_file_name, "%s%s%s", name, suffix->str, ".png");
	full_file_name = g_build_filename(directory, short_file_name->str, NULL);

#ifndef _WIN32
	// Non-windows code to save the screenshot
	gdk_pixbuf_save(screenshot, full_file_name, "png", NULL, "tEXt::Software", "The Flame Project: http://www.flameproject.org", NULL);
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
		g_error("Error 09: Something went wrong when opening the output screenhot file for writing");
		exit(7);
	}

	png_pointer = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (NULL == png_pointer)
	{
		// Something went wrong when creating the png write structure
		g_error("Error 10: Something went wrong when creating the png write structure");
		exit(8);
	}

	png_info_pointer = png_create_info_struct(png_pointer);
	if (NULL == png_info_pointer)
	{
		// Something went wrong when creating the png info structure

		// Clean up
		png_destroy_write_struct(&png_pointer, (png_infopp) NULL);

		// Generate a warning then exit
		g_error("Error 11: Something went wrong when creating the png info structure");
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

	// Embed a text comment with Flame's home page
	png_set_text(png_pointer, png_info_pointer, &flame_text, 1);

	// Get metadata about the screenshot
	return_code_int = GetObject(screenshot, sizeof(BITMAP), &screenshot_data);
	if (0 == return_code_int)
	{
		// Something went wrong when retrieving the screenshot metadata

		// Clean up
		png_destroy_write_struct(&png_pointer, &png_info_pointer);

		g_error("Error 12: Something went wrong when retrieving the screenshot metadata");
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

		g_error("Error 13: Something went wrong when copying the screenshot data");
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

	// Exit
	exit(0);
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.11  2008/02/04 08:35:23  vapour
 * Updated to use GString's rather than pre-allocated buffer, in order to fix a segfault and reduce the likelyhood of buffer overflows.
 *
 * Revision 1.10  2007/10/03 13:03:08  vapour
 * Updated PNG creation code for non windows to save the URL for Flame in the Software text string.
 *
 * Revision 1.9  2007/09/17 13:41:39  vapour
 * Writing PNG files now works (on my pc at least).
 *
 * Revision 1.8  2007/09/16 13:00:35  vapour
 * Still having extreme trouble with this.  At least it doesn't segfault now, but theres debugging code all over the place.
 *
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
