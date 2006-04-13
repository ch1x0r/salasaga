/*
 * $Id$
 * 
 * Flame Project: Background capture process
 * 
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.1  2006/04/13 15:57:49  vapour
 * Initial version, copied from my local CVS repository.
 *
 * Revision 1.20  2006/01/15 05:14:24  jc
 * Adjusted a comment for clarity.
 *
 * Revision 1.19  2005/11/20 00:49:25  jc
 * Removed the last of the include files that aren't needed on my system.  Might have just made it less portable (not sure).
 *
 * Revision 1.18  2005/11/20 00:47:46  jc
 * Cleaning up the include files.
 *
 * Revision 1.17  2005/11/20 00:38:35  jc
 * Updated to use execl() instead of execve(), so it inherits the right environment variables.
 *
 * Revision 1.16  2005/11/20 00:26:56  jc
 * Small tweak so it works on my system.
 *
 * Revision 1.15  2005/11/20 00:23:04  jc
 * Added initial working code to run the old gnome-screenshot program if no lock file is found.
 *
 * Revision 1.14  2005/11/19 14:51:43  jc
 * Cleaned up take_screenshot().
 *
 * Revision 1.13  2005/11/19 14:26:03  jc
 * Debugging code removed.  Screenshotting code needs to be cleaned up next.
 *
 * Revision 1.12  2005/11/19 14:18:00  jc
 * Added working code to increment the screenshot suffix.  Not thoroughly tested yet, and still has debugging stuff in it.
 * Needs cleaning up.
 *
 * Revision 1.11  2005/11/19 13:35:38  jc
 * Late in the night/morning.  Writing ugly code. :(
 * Now extracting the number from the highest screenshot.
 *
 * Revision 1.10  2005/11/19 13:16:37  jc
 * Adding further code to work out the highest numbered screen shot.
 *
 * Revision 1.9  2005/11/19 13:05:54  jc
 * Adding untested code to determine the highest existing screenshot number.
 *
 * Revision 1.8  2005/11/19 12:23:22  jc
 * Fixed a bug where g_build_filename() wasn't given a NULL as the last argument.
 *
 * Revision 1.7  2005/11/19 12:15:52  jc
 * Writing code to determine the correct target file name, and increase portability.
 *
 * Revision 1.6  2005/11/19 10:29:11  jc
 * Tweaked code to construct the screenshot file name from the correct target components.
 *
 * Revision 1.5  2005/11/19 10:18:56  jc
 * Tweaked code (tested) to always look for the .flame-lock file in the users home directory.
 *
 * Revision 1.4  2005/11/19 09:59:38  jc
 * Added some commented pseudo code (?) logic flow, and also added working code to save the screenshot into the correct directory.
 * Still need to add the code to determine the correct file name and create the target directory if it is not there.
 *
 * Revision 1.3  2005/11/19 09:25:56  jc
 * Added initial untested code to read parameters from the users .flame-lock file.
 *
 * Revision 1.2  2005/11/19 00:29:09  jc
 * Stripped out the GUI creation code, and added comments to outline program flow.
 *
 * Revision 1.1  2005/11/17 10:47:16  jc
 * Renamed main.c to flame-capture.c.
 *
 * Revision 1.6  2005/11/12 11:12:44  jc
 * Whoo Hooo!  Now displays the screenshot. :)
 *
 * Revision 1.5  2005/11/12 10:05:39  jc
 * Added working code that saves the screenshot (in my home directory with a fixed file name for now).
 *
 * Revision 1.4  2005/11/12 09:40:21  jc
 * Tried adding a routine to automatically save the captured screen shot.  Doesn't seem to be working however. :(
 *
 * Revision 1.3  2005/11/05 22:46:01  jc
 * Added small comment.
 *
 * Revision 1.2  2005/11/05 22:44:01  jc
 * Added code from gnome-utils 2.10.1, screenshot-utils.c, to do the actual screen shot.
 *
 * Revision 1.1  2005/11/05 11:48:37  jc
 * Initial placeholder.
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

// GDK functions
#include <gdk/gdkx.h>


// Application constants
#define	APP_VERSION 0.50


GdkPixbuf *take_screenshot(Window win, gint x_off, gint x_len, gint y_off, gint y_len)
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


// The main program loop
gint main(gint argc, gchar *argv[])
{
	// Local variables
	Window				win;				// Holds the Window ID we're screenshot-ing
	static GdkPixbuf	*screenshot = NULL;	// Holds the screen shot image

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


	// Initialise GTK
	gtk_init(&argc, &argv);

	// Construct the fullly qualified path name for ~/.flame-lock
	tmp_ptr = (gchar *) g_get_home_dir();
	full_file_name = g_build_filename(tmp_ptr, ".flame-lock", NULL);

	// Look for a ~/.flame-lock file
	lock_file = g_key_file_new();  // Create a file object in memory
	if (!g_key_file_load_from_file(lock_file, full_file_name, G_KEY_FILE_NONE, NULL))
	{
		// * If no ~/.flame-lock file is found, run the standard gnome-screenshot program instead *

		// Local variables
		gchar	*argv = "/usr/bin/gnome-screenshot.old";  // fixme4: Hard coded path name, definitely non-optimal
		gint	return_code;

		g_free(full_file_name);
		return_code = execl(argv, argv, (char *) NULL);  // fixme5: Should probably use this return_code in the error message below

		// This will only get here if running the old gnome-screenshot program failed
		g_error("Error 04: Unable to run standard gnome-screenshot program.");
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
	win = GDK_ROOT_WINDOW();
	screenshot = take_screenshot(win, x_offset, x_length, y_offset, y_length);
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
	g_slist_free(entries);

	// Construct the screen shot file name
	tmp_ptr = g_stpcpy(tmp_string, name);
	tmp_ptr = g_stpcpy(tmp_ptr, suffix);
	tmp_ptr = g_stpcpy(tmp_ptr, ".png");
	full_file_name = g_build_filename(directory, tmp_string, NULL);

	// Save the screenshot
	// file:///usr/share/gtk-doc/html/gdk-pixbuf/gdk-pixbuf-file-saving.html
	// Using PNG storage for now, as it seems much more efficient.
	// Because it has no option to adjust the quality of the save, I'm hoping it's set to
	// something lossless (equivalent of quality=100 for jpeg)
	gdk_pixbuf_save (screenshot, full_file_name, "png", NULL, NULL);

	// g_build_filename() requires the returned string to be g_free'd
	g_free(full_file_name);

	// Exit
	exit(0);
}
