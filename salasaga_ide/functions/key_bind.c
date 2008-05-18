/*
 * $Id$
 *
 * Salasaga: Setup the Control-Printscreen key combination to capture screenshots.  Non-windows only. 
 * 
 * Copyright (C) 2005-2008 Justin Clift <justin@salasaga.org>
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


// GTK includes
#include <gtk/gtk.h>

#ifndef _WIN32
	// Non-windows code
	#include <gdk/gdkx.h>
	#include <gconf/gconf.h>
#endif

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"
#include "display_warning.h"


gint key_bind(void)
{
	// Local variables
	GString				*command_key;				// Used to work out paths into the GConf structure
	GConfEngine			*gconf_engine;				// GConf engine
	gchar				*gconf_value;				//
	gboolean			key_already_set = FALSE;	// Used to work out which metacity run command is unassigned
	gchar				*return_code_gchar;			// Catches string based return codes
	guint				tmp_guint;					// Temporary guint
	guint				command_num = 0;			// Used to work out which metacity run command to use


	// If we're not running Metacity as our window manager, then display a warning to the user
	if (0 != g_ascii_strncasecmp(gdk_x11_screen_get_window_manager_name(gdk_screen_get_default()), "Metacity", 8))
	{
		if (TRUE == metacity_key_warning)
		{
			// Display the warning
			display_warning("Error ED379: Unable to set screenshot key to Control-Printscreen.  (Not running Metacity, so not sure how to.)  You will need to do this yourself manually.");

			// Ensure the the warning is only displayed once unless the user specifically requests otherwise 
			metacity_key_warning = FALSE;

			// Enable screenshots, as the user should have set up the key binding themself
			screenshots_enabled = TRUE;
			return -1;
		}
	}

	// Check if the "salasaga_screencapture" program is in the OS search path
	return_code_gchar = g_find_program_in_path("salasaga_screencapture");
	if (NULL == return_code_gchar)
	{
		display_warning("Error ED114: 'salasaga_screencapture' not found in the search path. Screenshot capturing is disabled.");
		return -1;
	}

	// Enable screenshots
	screenshots_enabled = TRUE;

	// Search for the first unused run command
	gconf_engine = gconf_engine_get_default();
	command_key = g_string_new(NULL);
	for (tmp_guint = 12; tmp_guint >= 1; tmp_guint--)
	{
		// Create the name of the key to check
		g_string_printf(command_key, "%s%u", "/apps/metacity/keybinding_commands/command_", tmp_guint);

		// Get the value for the key
		gconf_value = gconf_engine_get_string(gconf_engine, command_key->str, NULL);

		// Check if the key is unused
		if (0 == g_ascii_strncasecmp(gconf_value, "", 1))
		{
			// Yes it's unused, so make a note of it
			command_num = tmp_guint;
		} else
		{
			// * This command is being used *
			
			// Check if it's already assigned to salasaga_screencapture
			if (TRUE == g_str_has_suffix(gconf_value, "salasaga_screencapture"))
			{
				// Yes it is, so we make a note of it
				command_num = tmp_guint;
				key_already_set = TRUE;
				break;
			} else
			{
				// Check if it's already assigned to flame-capture, the previous name for the screenshot program
				if (TRUE == g_str_has_suffix(gconf_value, "flame-capture"))
				{
					// Yes it is, so we'll overwrite it with the new one
					command_num = tmp_guint;
					break;
				}
			}
		}
	}

	// If an unused run command was found and we haven't already assigned a screenshot key, then assign one
	if (FALSE == key_already_set)
	{
		if (0 != command_num)
		{
			g_string_printf(command_key, "%s%u", "/apps/metacity/keybinding_commands/command_", command_num);
			gconf_engine_set_string(gconf_engine, command_key->str, return_code_gchar, NULL);
			g_string_printf(command_key, "%s%u", "/apps/metacity/global_keybindings/run_command_", command_num);
			gconf_engine_set_string(gconf_engine, command_key->str, "<Control>Print", NULL);
		}
	}
	g_string_free(command_key, TRUE);

	// Free our GConf engine
	gconf_engine_unref(gconf_engine);

	// Return the command number that was set, or -1 if it wasn't
	if (0 != command_num)
	{
		return command_num;
	} else
	{
		return -1;
	}
}
