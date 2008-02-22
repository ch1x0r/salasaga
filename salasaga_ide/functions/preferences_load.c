/*
 * $Id$
 *
 * Flame Project: 
 * 
 * Copyright (C) 2008 Justin Clift <justin@postgresql.org>
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
#else
	// Windows only code
	#include <windows.h>
#endif

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"
#include "validate_value.h"

gboolean preferences_load()
{
#ifndef _WIN32  // Non-windows check

	// Local variables
	GError				*error = NULL;				// Pointer to error return structure
	GConfEngine			*gconf_engine;				// GConf engine
	gboolean			should_maximise = FALSE;	// Briefly keeps track of whether the window should be maximised


	// Check if we have a saved configuration in GConf
	gconf_engine = gconf_engine_get_default();
	if (TRUE == gconf_engine_dir_exists(gconf_engine, "/apps/flame", &error))
	{
		// Load the GConf configuration
		g_string_printf(default_project_folder, "%s", gconf_engine_get_string(gconf_engine, "/apps/flame/defaults/project_folder", NULL));
		g_string_printf(screenshots_folder, "%s", gconf_engine_get_string(gconf_engine, "/apps/flame/defaults/screenshots_folder", NULL));
		g_string_printf(default_output_folder, "%s", gconf_engine_get_string(gconf_engine, "/apps/flame/defaults/output_folder", NULL));
		if (NULL != gconf_engine_get_string(gconf_engine, "/apps/flame/defaults/zoom_level", NULL))
		{
			g_string_printf(default_zoom_level, "%s", gconf_engine_get_string(gconf_engine, "/apps/flame/defaults/zoom_level", NULL));
		}
		project_width = gconf_engine_get_int(gconf_engine, "/apps/flame/defaults/project_width", NULL);
		project_height = gconf_engine_get_int(gconf_engine, "/apps/flame/defaults/project_height", NULL);
		default_output_width = gconf_engine_get_int(gconf_engine, "/apps/flame/defaults/output_width", NULL);
		default_output_height = gconf_engine_get_int(gconf_engine, "/apps/flame/defaults/output_height", NULL);
		default_slide_length = slide_length = gconf_engine_get_int(gconf_engine, "/apps/flame/defaults/slide_length", NULL);
		default_bg_colour.red = gconf_engine_get_int(gconf_engine, "/apps/flame/defaults/default_bg_colour_red", NULL);
		default_bg_colour.green = gconf_engine_get_int(gconf_engine, "/apps/flame/defaults/default_bg_colour_green", NULL);
		default_bg_colour.blue = gconf_engine_get_int(gconf_engine, "/apps/flame/defaults/default_bg_colour_blue", NULL);
		preview_width = gconf_engine_get_int(gconf_engine, "/apps/flame/defaults/thumbnail_width", NULL);
		if (0 == preview_width) preview_width = 300;
		default_fps = frames_per_second = gconf_engine_get_int(gconf_engine, "/apps/flame/defaults/frames_per_second", NULL);

		// Check if the application should start maximised or not
		should_maximise = gconf_engine_get_bool(gconf_engine, "/apps/flame/defaults/window_maximised", NULL);

		// Free our GConf engine
		gconf_engine_unref(gconf_engine);
	}
	else
	{
		return FALSE;
	}
#else

	// * Registry related code (windows only) *

	// Check if we have a saved configuration in the windows registry
	HKEY				hkey;
	guchar				buffer_data[1000];
	LPSTR				buffer_ptr = &buffer_data[0];
	glong				buffer_size;
	gboolean			missing_keys = FALSE;
	glong				return_code;


	// Check if the Flame Project registry keys exist
	if (!RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject", 0, KEY_QUERY_VALUE, &hkey))
	{
		// They do, so load the default values
		RegCloseKey(hkey);

		// Retrieve the value for the project folder
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, KEY_QUERY_VALUE, &hkey))
		{
			// Value is missing, so warn the user and set a sensible default
			missing_keys = TRUE;
			g_string_printf(default_project_folder, "%s%c%s", g_get_home_dir(), G_DIR_SEPARATOR, "projects");
		} else
		{
			// Retrieve the value
			buffer_size = sizeof(buffer_data);
			return_code = RegQueryValueExA(hkey, "project_folder", NULL, NULL, buffer_ptr, &buffer_size);
			if (ERROR_SUCCESS == return_code)
			{
				g_string_printf(default_project_folder, "%s", buffer_ptr);
			}

			// Close the registry key
			RegCloseKey(hkey);
		}

		// Retrieve the value for the screenshots folder
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, KEY_QUERY_VALUE, &hkey))
		{
			// Value is missing, so warn the user and set a sensible default
			missing_keys = TRUE;
			g_string_printf(screenshots_folder, "%s%c%s", g_get_home_dir(), G_DIR_SEPARATOR, "screenshots");
		} else
		{
			// Retrieve the value
			buffer_size = sizeof(buffer_data);
			return_code = RegQueryValueExA(hkey, "screenshots_folder", NULL, NULL, buffer_ptr, &buffer_size);
			if (ERROR_SUCCESS == return_code)
			{
				g_string_printf(screenshots_folder, "%s", buffer_ptr);
			}

			// Close the registry key
			RegCloseKey(hkey);
		}

		// Retrieve the value for the default output folder
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, KEY_QUERY_VALUE, &hkey))
		{
			// Value is missing, so warn the user and set a sensible default
			missing_keys = TRUE;
			g_string_printf(default_output_folder, "%s%c%s", g_get_home_dir(), G_DIR_SEPARATOR, "output");
		} else
		{
			// Retrieve the value
			buffer_size = sizeof(buffer_data);
			return_code = RegQueryValueExA(hkey, "default_output_folder", NULL, NULL, buffer_ptr, &buffer_size);
			if (ERROR_SUCCESS == return_code)
			{
				g_string_printf(default_output_folder, "%s", buffer_ptr);
			}

			// Close the registry key
			RegCloseKey(hkey);
		}

		// Retrieve the value for the default zoom level
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, KEY_QUERY_VALUE, &hkey))
		{
			// Value is missing, so warn the user (we've already set a default)
			missing_keys = TRUE;
		} else
		{
			// Retrieve the value
			buffer_size = sizeof(buffer_data);
			return_code = RegQueryValueExA(hkey, "default_zoom_level", NULL, NULL, buffer_ptr, &buffer_size);
			if (ERROR_SUCCESS == return_code)
			{
				g_string_printf(default_zoom_level, "%s", buffer_ptr);
			}

			// Close the registry key
			RegCloseKey(hkey);
		}

		// Retrieve the value for the project width
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, KEY_QUERY_VALUE, &hkey))
		{
			// Value is missing, so warn the user and set a sensible default
			missing_keys = TRUE;
			which_screen = gtk_window_get_screen(GTK_WINDOW(main_window));
			project_width = gdk_screen_get_width(which_screen);
		} else
		{
			// Retrieve the value
			buffer_size = sizeof(buffer_data);
			return_code = RegQueryValueExA(hkey, "project_width", NULL, NULL, buffer_ptr, &buffer_size);
			if (ERROR_SUCCESS == return_code)
			{
				project_width = atoi(buffer_ptr);
			}

			// Close the registry key
			RegCloseKey(hkey);
		}

		// Retrieve the value for the project height
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, KEY_QUERY_VALUE, &hkey))
		{
			// Value is missing, so warn the user and set a sensible default
			missing_keys = TRUE;
			which_screen = gtk_window_get_screen(GTK_WINDOW(main_window));
			project_height = gdk_screen_get_height(which_screen);
		} else
		{
			// Retrieve the value
			buffer_size = sizeof(buffer_data);
			return_code = RegQueryValueExA(hkey, "project_height", NULL, NULL, buffer_ptr, &buffer_size);
			if (ERROR_SUCCESS == return_code)
			{
				project_height = atoi(buffer_ptr);
			}

			// Close the registry key
			RegCloseKey(hkey);
		}

		// Retrieve the value for the output width
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, KEY_QUERY_VALUE, &hkey))
		{
			// Value is missing, so warn the user and set a sensible default
			missing_keys = TRUE;
			default_output_width = 800;
		} else
		{
			// Retrieve the value
			buffer_size = sizeof(buffer_data);
			return_code = RegQueryValueExA(hkey, "output_width", NULL, NULL, buffer_ptr, &buffer_size);
			if (ERROR_SUCCESS == return_code)
			{
				default_output_width = atoi(buffer_ptr);
			}

			// Close the registry key
			RegCloseKey(hkey);
		}

		// Retrieve the value for the output height
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, KEY_QUERY_VALUE, &hkey))
		{
			// Value is missing, so warn the user and set a sensible default
			missing_keys = TRUE;
			default_output_height = 600;
		} else
		{
			// Retrieve the value
			buffer_size = sizeof(buffer_data);
			return_code = RegQueryValueExA(hkey, "output_height", NULL, NULL, buffer_ptr, &buffer_size);
			if (ERROR_SUCCESS == return_code)
			{
				default_output_height = atoi(buffer_ptr);
			}

			// Close the registry key
			RegCloseKey(hkey);
		}

		// Retrieve the value for the default slide length
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, KEY_QUERY_VALUE, &hkey))
		{
			// Value is missing, so warn the user and set a sensible default
			missing_keys = TRUE;
			default_slide_length = slide_length = 60;
		} else
		{
			// Retrieve the value
			buffer_size = sizeof(buffer_data);
			return_code = RegQueryValueExA(hkey, "slide_length", NULL, NULL, buffer_ptr, &buffer_size);
			if (ERROR_SUCCESS == return_code)
			{
				default_slide_length = slide_length = atoi(buffer_ptr);
			}

			// Close the registry key
			RegCloseKey(hkey);
		}

		// Retrieve the value for the default thumbnail width
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, KEY_QUERY_VALUE, &hkey))
		{
			// Value is missing, so warn the user and set a sensible default
			missing_keys = TRUE;
			preview_width = 300;
		} else
		{
			// Retrieve the value
			buffer_size = sizeof(buffer_data);
			return_code = RegQueryValueExA(hkey, "thumbnail_width", NULL, NULL, buffer_ptr, &buffer_size);
			if (ERROR_SUCCESS == return_code)
			{
				preview_width = atoi(buffer_ptr);
			}

			// Close the registry key
			RegCloseKey(hkey);
		}

		// Retrieve the value for the default slide length
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, KEY_QUERY_VALUE, &hkey))
		{
			// Value is missing, so warn the user and set a sensible default
			missing_keys = TRUE;
			default_slide_length = slide_length = 60;
		} else
		{
			// Retrieve the value
			buffer_size = sizeof(buffer_data);
			return_code = RegQueryValueExA(hkey, "slide_length", NULL, NULL, buffer_ptr, &buffer_size);
			if (ERROR_SUCCESS == return_code)
			{
				default_slide_length = slide_length = atoi(buffer_ptr);
			}

			// Close the registry key
			RegCloseKey(hkey);
		}

		// Retrieve the value for the default frames per second
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, KEY_QUERY_VALUE, &hkey))
		{
			// Value is missing, so warn the user and set a sensible default
			missing_keys = TRUE;
			default_fps = frames_per_second = 12;
		} else
		{
			// Retrieve the value
			buffer_size = sizeof(buffer_data);
			return_code = RegQueryValueExA(hkey, "frames_per_second", NULL, NULL, buffer_ptr, &buffer_size);
			if (ERROR_SUCCESS == return_code)
			{
				default_fps = frames_per_second = atoi(buffer_ptr);
			}

			// Close the registry key
			RegCloseKey(hkey);
		}

		// Retrieve the value for the red component of the default background colour
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, KEY_QUERY_VALUE, &hkey))
		{
			// Value is missing, so warn the user and set a sensible default
			missing_keys = TRUE;
			default_bg_colour.red = 0;
		} else
		{
			// Retrieve the value
			buffer_size = sizeof(buffer_data);
			return_code = RegQueryValueExA(hkey, "default_bg_colour_red", NULL, NULL, buffer_ptr, &buffer_size);
			if (ERROR_SUCCESS == return_code)
			{
				default_bg_colour.red = atoi(buffer_ptr);
			}

			// Close the registry key
			RegCloseKey(hkey);
		}

		// Retrieve the value for the green component of the default background colour
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, KEY_QUERY_VALUE, &hkey))
		{
			// Value is missing, so warn the user and set a sensible default
			missing_keys = TRUE;
			default_bg_colour.green = 0;
		} else
		{
			// Retrieve the value
			buffer_size = sizeof(buffer_data);
			return_code = RegQueryValueExA(hkey, "default_bg_colour_green", NULL, NULL, buffer_ptr, &buffer_size);
			if (ERROR_SUCCESS == return_code)
			{
				default_bg_colour.green = atoi(buffer_ptr);
			}

			// Close the registry key
			RegCloseKey(hkey);
		}

		// Retrieve the value for the blue component of the default background colour
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, KEY_QUERY_VALUE, &hkey))
		{
			// Value is missing, so warn the user and set a sensible default
			missing_keys = TRUE;
			default_bg_colour.blue = 0;
		} else
		{
			// Retrieve the value
			buffer_size = sizeof(buffer_data);
			return_code = RegQueryValueExA(hkey, "default_bg_colour_blue", NULL, NULL, buffer_ptr, &buffer_size);
			if (ERROR_SUCCESS == return_code)
			{
				default_bg_colour.blue = atoi(buffer_ptr);
			}

			// Close the registry key
			RegCloseKey(hkey);
		}

		// Retrieve the value for the initial window maximisation
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, KEY_QUERY_VALUE, &hkey))
		{
			// Value is missing, so warn the user and set a sensible default
			missing_keys = TRUE;
		} else
		{
			// Retrieve the value
			buffer_size = sizeof(buffer_data);
			return_code = RegQueryValueExA(hkey, "window_maximised", NULL, NULL, buffer_ptr, &buffer_size);
			if (ERROR_SUCCESS == return_code)
			{
				should_maximise = atoi(buffer_ptr);
			}

			// Close the registry key
			RegCloseKey(hkey);
		}

		// If some of the registry keys were missing then alert the user
		if (TRUE == missing_keys)
		{
			display_warning("Some of the project default registry keys are missing\n");
		}
	}
	else
	{
		return FALSE;
	}

#endif  // End of windows code

	// Maximise the window if our saved configuration says to
	if (TRUE == should_maximise)
	{
		// Maximise the window
		gtk_window_maximize(GTK_WINDOW(main_window));
	}

	return TRUE;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.1  2008/02/22 14:08:39  vapour
 * Moved the loading of application preferences from the main function into its own one, to ease the addition of validation code.
 *
 */
