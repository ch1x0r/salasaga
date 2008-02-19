/*
 * $Id$
 *
 * Flame Project: Function to save the application preferences and go required cleanup, prior to exiting
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


// GTK includes
#include <gtk/gtk.h>

#ifndef _WIN32
	// Non-windows code
	#include <gconf/gconf.h>
	#include <libgnome/libgnome.h>
#else
	// Windows only code
	#include <windows.h>
#endif

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"


void save_preferences_and_exit(void)
{
#ifndef _WIN32
	// * Non-windows code *

	// Local variables
	GConfEngine			*gconf_engine;				// GConf engine

	guint				tmp_int;					// Temporary integer
	GtkWindow			*tmp_gtk_window;			// Temporary GtkWindow
	GdkWindow			*tmp_gdk_window;			// Temporary GdkWindow


	// Save the application preferences
	gconf_engine = gconf_engine_get_default();
	gconf_engine_set_string(gconf_engine, "/apps/flame/defaults/project_folder", default_project_folder->str, NULL);
	gconf_engine_set_string(gconf_engine, "/apps/flame/defaults/screenshots_folder", screenshots_folder->str, NULL);
	gconf_engine_set_string(gconf_engine, "/apps/flame/defaults/output_folder", default_output_folder->str, NULL);
	gconf_engine_set_string(gconf_engine, "/apps/flame/defaults/zoom_level", default_zoom_level->str, NULL);
	gconf_engine_set_int(gconf_engine, "/apps/flame/defaults/project_width", project_width, NULL);
	gconf_engine_set_int(gconf_engine, "/apps/flame/defaults/project_height", project_height, NULL);
	gconf_engine_set_int(gconf_engine, "/apps/flame/defaults/output_width", default_output_width, NULL);
	gconf_engine_set_int(gconf_engine, "/apps/flame/defaults/output_height", default_output_height, NULL);
	gconf_engine_set_int(gconf_engine, "/apps/flame/defaults/slide_length", default_slide_length, NULL);
	gconf_engine_set_int(gconf_engine, "/apps/flame/defaults/thumbnail_width", preview_width, NULL);
	gconf_engine_set_int(gconf_engine, "/apps/flame/defaults/frames_per_second", default_fps, NULL);
	gconf_engine_set_int(gconf_engine, "/apps/flame/defaults/default_bg_colour_red", default_bg_colour.red, NULL);
	gconf_engine_set_int(gconf_engine, "/apps/flame/defaults/default_bg_colour_green", default_bg_colour.green, NULL);
	gconf_engine_set_int(gconf_engine, "/apps/flame/defaults/default_bg_colour_blue", default_bg_colour.blue, NULL);

	// * Save the present window maximised state (i.e. if we're maximised or not) *

	// Get a pointer to the underlying GDK window
	tmp_gtk_window = GTK_WINDOW(main_window);
	if (NULL != tmp_gtk_window->frame)
	{
		tmp_gdk_window = tmp_gtk_window->frame;
	}
	else
	{
		tmp_gdk_window = main_window->window;
	}

	// If the main application window still exists, save the window maximisation state, else don't bother
	// (it doesn't exist if the application exits due to Alt-F4 or pressing the title bar close button)
	if (NULL != tmp_gdk_window)
	{
		// Find out if the window is presently maximised or not
		tmp_int = gdk_window_get_state(tmp_gdk_window);
		if (GDK_WINDOW_STATE_MAXIMIZED == tmp_int)
		{
			// The window is maximised, so save that info
			gconf_engine_set_bool(gconf_engine, "/apps/flame/defaults/window_maximised", TRUE, NULL);
		} else
		{
			// The window is not maximised, so save that info
			gconf_engine_set_bool(gconf_engine, "/apps/flame/defaults/window_maximised", FALSE, NULL);
		}
	}

	// Save a configuration structure version number
	gconf_engine_set_float(gconf_engine, "/apps/flame/defaults/config_version", 1.0, NULL);

	// fixme4: Should we save a list of recent projects worked on?

	// Notify the GConf engine that now is probably a good time to sync
	gconf_engine_suggest_sync(gconf_engine, NULL);

	// Free our GConf engine
	gconf_engine_unref(gconf_engine);

	// Shut down sound
	gnome_sound_shutdown();

#else

	// * Windows-only code (registry for now) *

	// Check if we have a saved configuration in the windows registry
	HKEY		hkey;
	glong		return_code;
	glong		string_size;

	GString		*tmp_gstring;


	// Initialise some things
	tmp_gstring = g_string_new(NULL);


	// Check if the base Flame Project registry key exists
	if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject", 0, KEY_QUERY_VALUE, &hkey))
	{
		// It doesn't, so create it
		return_code = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\FlameProject", 0, NULL,
						REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, NULL);
		if (ERROR_SUCCESS != return_code)
		{
			// Creating the base registry key failed, so display a warning then exit
			display_warning("Error ED54: Saving preferences in the registry failed\n");
			gtk_main_quit();
		}
	} else
	{
		// The base Flame Project registry key exists, so we don't need to create it
		RegCloseKey(hkey);
	}

	// Check if the defaults registry key exists
	if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hkey))
	{
		// It doesn't, so create it
		return_code = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, NULL,
						REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, NULL);
		if (ERROR_SUCCESS != return_code)
		{
			// Creating the defaults registry key failed, so display a warning then exit
			display_warning("Error ED55: Saving preferences in the registry failed\n");
			gtk_main_quit();
		}
	}

	// * At this point, the variable hkey should point to the opened defaults registry key *

	// Set the value for the project folder
	string_size = (default_project_folder->len) + 1;
	return_code = RegSetValueEx(hkey, "project_folder", 0, REG_SZ, default_project_folder->str, string_size);

	// Set the value for the screenshots folder
	string_size = (screenshots_folder->len) + 1;
	return_code = RegSetValueEx(hkey, "screenshots_folder", 0, REG_SZ, screenshots_folder->str, string_size);

	// Set the value for the output folder
	string_size = (default_output_folder->len) + 1;
	return_code = RegSetValueEx(hkey, "output_folder", 0, REG_SZ, default_output_folder->str, string_size);

	// Set the value for the zoom level
	string_size = (default_zoom_level->len) + 1;
	return_code = RegSetValueEx(hkey, "zoom_level", 0, REG_SZ, default_zoom_level->str, string_size);

	// Set the value for the project name
	string_size = (project_name->len) + 1;
	return_code = RegSetValueEx(hkey, "project_name", 0, REG_SZ, project_name->str, string_size);

	// Set the value for the project width
	g_string_printf(tmp_gstring, "%d", project_width);
	string_size = (tmp_gstring->len) + 1;
	return_code = RegSetValueEx(hkey, "project_width", 0, REG_SZ, tmp_gstring->str, string_size);

	// Set the value for the project height
	g_string_printf(tmp_gstring, "%d", project_height);
	string_size = (tmp_gstring->len) + 1;
	return_code = RegSetValueEx(hkey, "project_height", 0, REG_SZ, tmp_gstring->str, string_size);

	// Set the value for the default output width
	g_string_printf(tmp_gstring, "%d", default_output_width);
	string_size = (tmp_gstring->len) + 1;
	return_code = RegSetValueEx(hkey, "output_width", 0, REG_SZ, tmp_gstring->str, string_size);

	// Set the value for the default output height
	g_string_printf(tmp_gstring, "%d", default_output_height);
	string_size = (tmp_gstring->len) + 1;
	return_code = RegSetValueEx(hkey, "output_height", 0, REG_SZ, tmp_gstring->str, string_size);

	// Set the value for the default slide length
	g_string_printf(tmp_gstring, "%d", slide_length);
	string_size = (tmp_gstring->len) + 1;
	return_code = RegSetValueEx(hkey, "slide_length", 0, REG_SZ, tmp_gstring->str, string_size);

	// Set the value for the thumbnail width
	g_string_printf(tmp_gstring, "%d", preview_width);
	string_size = (tmp_gstring->len) + 1;
	return_code = RegSetValueEx(hkey, "thumbnail_width", 0, REG_SZ, tmp_gstring->str, string_size);

	// Set the value for the frames per second
	g_string_printf(tmp_gstring, "%d", default_fps);
	string_size = (tmp_gstring->len) + 1;
	return_code = RegSetValueEx(hkey, "frames_per_second", 0, REG_SZ, tmp_gstring->str, string_size);

	// Set the value for the red component of the default background colour
	g_string_printf(tmp_gstring, "%d", default_bg_colour.red);
	string_size = (tmp_gstring->len) + 1;
	return_code = RegSetValueEx(hkey, "default_bg_colour_red", 0, REG_SZ, tmp_gstring->str, string_size);

	// Set the value for the green component of the default background colour
	g_string_printf(tmp_gstring, "%d", default_bg_colour.green);
	string_size = (tmp_gstring->len) + 1;
	return_code = RegSetValueEx(hkey, "default_bg_colour_green", 0, REG_SZ, tmp_gstring->str, string_size);

	// Set the value for the blue component of the default background colour
	g_string_printf(tmp_gstring, "%d", default_bg_colour.blue);
	string_size = (tmp_gstring->len) + 1;
	return_code = RegSetValueEx(hkey, "default_bg_colour_blue", 0, REG_SZ, tmp_gstring->str, string_size);

	// Set the value for the window maximisation to TRUE (hard coded for now)
	// fixme4: We should make this dynamic rather than hard coded on
	g_string_printf(tmp_gstring, "%d", TRUE);
	string_size = (tmp_gstring->len) + 1;
	return_code = RegSetValueEx(hkey, "window_maximised", 0, REG_SZ, tmp_gstring->str, string_size);

	// All values saved in the windows registry
	RegCloseKey(hkey);

	// Unhook the Control-PrintScreen key hook (if it's set)
	if (NULL != win32_keyboard_hook_handle)
	{
		UnhookWindowsHookEx(win32_keyboard_hook_handle);
	}

#endif

	// Exit the application
	gtk_main_quit();
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.10  2008/02/19 14:13:31  vapour
 * Updated to save the default background colour between sessions.
 *
 * Revision 1.9  2008/02/19 13:42:44  vapour
 * Removed scaling quality variable, added default frames per second and default zoom variables.
 *
 * Revision 1.8  2008/02/05 09:18:25  vapour
 * Removed support of output quality variable, as the concept is no longer relevant.
 *
 * Revision 1.7  2008/02/05 06:36:44  vapour
 * No longer saves Project Name value as that's always set to New Project now.
 *
 * Revision 1.6  2008/02/04 17:11:52  vapour
 *  + Removed unnecessary includes.
 *
 * Revision 1.5  2008/01/19 06:59:05  vapour
 * Tweaked some error messages for clarity.
 *
 * Revision 1.4  2008/01/15 16:18:59  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.3  2007/09/29 04:22:12  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 * Revision 1.2  2007/09/28 12:05:07  vapour
 * Broke callbacks.c and callbacks.h into its component functions.
 *
 * Revision 1.1  2007/09/27 10:40:50  vapour
 * Broke backend.c and backend.h into its component functions.
 *
 */
