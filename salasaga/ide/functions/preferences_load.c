/*
 * $Id$
 *
 * Salasaga: Retrieves saved application defaults if available
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


// Turn on C99 compatibility
#define _ISOC99_SOURCE

// Standard includes
#include <stdlib.h>

// GTK includes
#include <gtk/gtk.h>

#ifndef _WIN32
	// Non-windows code
	#include <gconf/gconf.h>
#else
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"
#include "dialog/display_warning.h"
#include "validate_value.h"

gboolean preferences_load()
{
#ifndef _WIN32  // Non-windows check

	// Local variables
	GError				*error = NULL;				// Pointer to a error return structure
	GConfEngine			*gconf_engine;				// GConf engine
	gfloat				gfloat_val;					// Temporary gint value
	guint				guint_val;					// Temporary guint value used for validation
	GConfValue*         help_text_key_data;			// Receives a raw key value, to help determine whether it exists already
	GString				*message;					// Used to construct message strings
	gboolean			should_display_help_text = TRUE;	// Receives the gboolean as to whether help text should be displayed
	gboolean			should_maximise = FALSE;	// Briefly keeps track of whether the window should be maximised
	gboolean			should_keybind_warn = TRUE;	// Receives the gboolean as to whether the non-metacity key bind warning should be displayed
	gchar				*tmp_gchar;					// Used for temporary string retrieval
	gboolean			usable_input;				// Used to control loop flow
	GdkColor			valid_bg_colour = {0,0,0};	// Receives the new default background colour for slides once validated
	guint				valid_default_fps = 0;		// Receives the new default fps once validated
	GdkColor			valid_default_text_fg_colour = {0, 0, 0};  // Receives the new default text layer foreground colour once validated
	gint				valid_default_text_font_face = 0;  // Receives the new default text layer font face once validated
	gdouble				valid_default_text_font_size =0.0;  // Receives the new default text layer font size once validated
	guint				valid_icon_height = 0;		// Receives the new icon height once validated
	gfloat				valid_layer_duration = 0;	// Receives the new default layer duration once validated
	GString				*valid_output_folder;		// Receives the new output folder once validated
	guint				valid_output_height = 0;	// Receives the new default output height once validated
	guint				valid_output_width = 0;		// Receives the new default output width once validated
	guint				valid_preview_width = 0;	// Receives the new film strip thumbnail width once validated
	GString				*valid_project_folder;		// Receives the new default project folder once validated
	guint				valid_project_height = 0;	// Receives the new project height once validated
	guint				valid_project_width = 0;	// Receives the new project width once validated
	guint				valid_screenshot_delay = 0;  // Receives the new screenshot delay once validated
	GString				*valid_screenshot_folder;	// Receives the new screenshot folder once validated
	gfloat				valid_slide_duration = 0;	// Receives the new default slide duration once validated
	GString				*valid_zoom_level;			// Receives the new default zoom level once validated
	gfloat				*validated_gfloat;			// Receives known good gfloat values from the validation function
	guint				*validated_guint;			// Receives known good guint values from the validation function
	GString				*validated_string;			// Receives known good strings from the validation function


	// Initialise things
	message = g_string_new(NULL);
	valid_output_folder = g_string_new(NULL);
	valid_project_folder = g_string_new(NULL);
	valid_screenshot_folder = g_string_new(NULL);
	valid_zoom_level = g_string_new(NULL);

	// Check if we have a saved configuration in GConf
	gconf_engine = gconf_engine_get_default();
	if (FALSE == gconf_engine_dir_exists(gconf_engine, "/apps/salasaga", &error))
	{
		// We don't have a set of preferences available for loading
		g_string_free(valid_output_folder, TRUE);
		g_string_free(valid_project_folder, TRUE);
		g_string_free(valid_screenshot_folder, TRUE);
		if (NULL != error)
			g_error_free(error);
		return FALSE;
	}

	// Reset the usable input flag
	usable_input = TRUE;

	// Retrieve the new default project folder input
	tmp_gchar = gconf_engine_get_string(gconf_engine, "/apps/salasaga/defaults/project_folder", NULL);
	validated_string = validate_value(FOLDER_PATH, V_CHAR, tmp_gchar);
	g_free(tmp_gchar);
	if (NULL == validated_string)
	{
		g_string_printf(message, "%s ED185: %s", _("Error"), _("There was something wrong with the project folder value stored in the preferences.  Using default preferences instead."));
		display_warning(message->str);
		usable_input = FALSE;
	} else
	{
		valid_project_folder = g_string_assign(valid_project_folder, validated_string->str);
		g_string_free(validated_string, TRUE);
		validated_string = NULL;
	}

	// Retrieve the new screenshots folder input
	tmp_gchar = gconf_engine_get_string(gconf_engine, "/apps/salasaga/defaults/screenshots_folder", NULL);
	validated_string = validate_value(FOLDER_PATH, V_CHAR, tmp_gchar);
	g_free(tmp_gchar);
	if (NULL == validated_string)
	{
		g_string_printf(message, "%s ED186: %s", _("Error"), _("There was something wrong with the screenshots folder value stored in the preferences.  Using default preferences instead."));
		display_warning(message->str);
		usable_input = FALSE;
	} else
	{
		valid_screenshot_folder = g_string_assign(valid_screenshot_folder, validated_string->str);
		g_string_free(validated_string, TRUE);
		validated_string = NULL;
	}

	// Retrieve the new default output folder input
	tmp_gchar = gconf_engine_get_string(gconf_engine, "/apps/salasaga/defaults/output_folder", NULL);
	validated_string = validate_value(FOLDER_PATH, V_CHAR, tmp_gchar);
	g_free(tmp_gchar);
	if (NULL == validated_string)
	{
		g_string_printf(message, "%s ED187: %s", _("Error"), _("There was something wrong with the default output folder value stored in the preferences.  Using default preferences instead."));
		display_warning(message->str);
		usable_input = FALSE;
	} else
	{
		valid_output_folder = g_string_assign(valid_output_folder, validated_string->str);
		g_string_free(validated_string, TRUE);
		validated_string = NULL;
	}

	// Retrieve the new default zoom level input
	tmp_gchar = gconf_engine_get_string(gconf_engine, "/apps/salasaga/defaults/zoom_level", NULL);
	if (NULL != tmp_gchar)
	{
		validated_string = validate_value(ZOOM_LEVEL, V_ZOOM, tmp_gchar);
		if (NULL == validated_string)
		{
			g_string_printf(message, "%s ED188: %s", _("Error"), _("There was something wrong with the default zoom level value stored in the preferences.  Using default preferences instead."));
			display_warning(message->str);
			usable_input = FALSE;
		} else
		{
			valid_zoom_level = g_string_assign(valid_zoom_level, validated_string->str);
			g_string_free(validated_string, TRUE);
			validated_string = NULL;
		}
	}
	g_free(tmp_gchar);

	// Retrieve the new default project width input
	guint_val = gconf_engine_get_int(gconf_engine, "/apps/salasaga/defaults/project_width", NULL);
	validated_guint = validate_value(PROJECT_WIDTH, V_INT_UNSIGNED, &guint_val);
	if (NULL == validated_guint)
	{
		g_string_printf(message, "%s ED189: %s", _("Error"), _("There was something wrong with the default project width value stored in the preferences.  Using default preferences instead."));
		display_warning(message->str);
		usable_input = FALSE;
	} else
	{
		valid_project_width = *validated_guint;
		g_free(validated_guint);
	}

	// Retrieve the new default project height input
	guint_val = gconf_engine_get_int(gconf_engine, "/apps/salasaga/defaults/project_height", NULL);
	validated_guint = validate_value(PROJECT_HEIGHT, V_INT_UNSIGNED, &guint_val);
	if (NULL == validated_guint)
	{
		g_string_printf(message, "%s ED190: %s", _("Error"), _("There was something wrong with the default project width value stored in the preferences.  Using default preferences instead."));
		display_warning(message->str);
		usable_input = FALSE;
	} else
	{
		valid_project_height = *validated_guint;
		g_free(validated_guint);
	}

	// Retrieve the new default output width input
	guint_val = gconf_engine_get_int(gconf_engine, "/apps/salasaga/defaults/output_width", NULL);
	validated_guint = validate_value(PROJECT_WIDTH, V_INT_UNSIGNED, &guint_val);
	if (NULL == validated_guint)
	{
		g_string_printf(message, "%s ED191: %s", _("Error"), _("There was something wrong with the default output width value stored in the preferences.  Using default preferences instead."));
		display_warning(message->str);
		usable_input = FALSE;
	} else
	{
		valid_output_width = *validated_guint;
		g_free(validated_guint);
	}

	// Retrieve the new default output height input
	guint_val = gconf_engine_get_int(gconf_engine, "/apps/salasaga/defaults/output_height", NULL);
	validated_guint = validate_value(PROJECT_HEIGHT, V_INT_UNSIGNED, &guint_val);
	if (NULL == validated_guint)
	{
		g_string_printf(message, "%s ED192: %s", _("Error"), _("There was something wrong with the default output height value stored in the preferences.  Using default preferences instead."));
		display_warning(message->str);
		usable_input = FALSE;
	} else
	{
		valid_output_height = *validated_guint;
		g_free(validated_guint);
	}

	// Retrieve the new default slide duration input
	gfloat_val = gconf_engine_get_float(gconf_engine, "/apps/salasaga/defaults/slide_duration", NULL);
	validated_gfloat = validate_value(SLIDE_DURATION, V_FLOAT_UNSIGNED, &gfloat_val);
	if (NULL == validated_gfloat)
	{
		g_string_printf(message, "%s ED193: %s", _("Error"), _("There was something wrong with the default slide duration value stored in the preferences.  Using default preferences instead."));
		display_warning(message->str);
		usable_input = FALSE;
	} else
	{
		valid_slide_duration = *validated_gfloat;
		g_free(validated_gfloat);
	}

	// Retrieve the new default layer duration input
	gfloat_val = gconf_engine_get_float(gconf_engine, "/apps/salasaga/defaults/layer_duration", NULL);
	validated_gfloat = validate_value(LAYER_DURATION, V_FLOAT_UNSIGNED, &gfloat_val);
	if (NULL == validated_gfloat)
	{
		g_string_printf(message, "%s ED333: %s", _("Error"), _("There was something wrong with the default layer duration value stored in the preferences.  Using default preferences instead."));
		display_warning(message->str);
		usable_input = FALSE;
	} else
	{
		valid_layer_duration = *validated_gfloat;
		g_free(validated_gfloat);
	}

	// Retrieve the new default background colour, red component
	guint_val = gconf_engine_get_int(gconf_engine, "/apps/salasaga/defaults/default_bg_colour_red", NULL);
	validated_guint = validate_value(COLOUR_COMP16, V_INT_UNSIGNED, &guint_val);
	if (NULL == validated_guint)
	{
		g_string_printf(message, "%s ED194: %s", _("Error"), _("There was something wrong with the default background color value stored in the preferences.  Using default preferences instead."));
		display_warning(message->str);
		usable_input = FALSE;
	} else
	{
		valid_bg_colour.red = *validated_guint;
		g_free(validated_guint);
	}

	// Retrieve the new default background colour, green component
	guint_val = gconf_engine_get_int(gconf_engine, "/apps/salasaga/defaults/default_bg_colour_green", NULL);
	validated_guint = validate_value(COLOUR_COMP16, V_INT_UNSIGNED, &guint_val);
	if (NULL == validated_guint)
	{
		g_string_printf(message, "%s ED195: %s", _("Error"), _("There was something wrong with the default background color value stored in the preferences.  Using default preferences instead."));
		display_warning(message->str);
		usable_input = FALSE;
	} else
	{
		valid_bg_colour.green = *validated_guint;
		g_free(validated_guint);
	}

	// Retrieve the new default background colour, blue component
	guint_val = gconf_engine_get_int(gconf_engine, "/apps/salasaga/defaults/default_bg_colour_blue", NULL);
	validated_guint = validate_value(COLOUR_COMP16, V_INT_UNSIGNED, &guint_val);
	if (NULL == validated_guint)
	{
		g_string_printf(message, "%s ED196: %s", _("Error"), _("There was something wrong with the default background color value stored in the preferences.  Using default preferences instead."));
		display_warning(message->str);
		usable_input = FALSE;
	} else
	{
		valid_bg_colour.blue = *validated_guint;
		g_free(validated_guint);
	}

	// Retrieve the new preview width input
	guint_val = gconf_engine_get_int(gconf_engine, "/apps/salasaga/defaults/thumbnail_width", NULL);
	if (0 != guint_val)
	{
		validated_guint = validate_value(PREVIEW_WIDTH, V_INT_UNSIGNED, &guint_val);
		if (NULL == validated_guint)
		{
			g_string_printf(message, "%s ED197: %s", _("Error"), _("There was something wrong with the film strip thumbnail width value stored in the preferences.  Using default preferences instead."));
			display_warning(message->str);
			usable_input = FALSE;
		} else
		{
			valid_preview_width = *validated_guint;
			g_free(validated_guint);
		}
	}
	else
	{
		// No thumbnail width stored, so we use a reasonable default
		valid_preview_width = 300;
	}

	// Retrieve the new default frames per second input
	guint_val = gconf_engine_get_int(gconf_engine, "/apps/salasaga/defaults/frames_per_second", NULL);
	validated_guint = validate_value(PROJECT_FPS, V_INT_UNSIGNED, &guint_val);
	if (NULL == validated_guint)
	{
		g_string_printf(message, "%s ED198: %s", _("Error"), _("There was something wrong with the default frames per second value stored in the preferences.  Using default preferences instead."));
		display_warning(message->str);
		usable_input = FALSE;
	} else
	{
		valid_default_fps = *validated_guint;
		g_free(validated_guint);
	}

	// Retrieve the new icon height input
	guint_val = gconf_engine_get_int(gconf_engine, "/apps/salasaga/defaults/icon_height", NULL);
	if (0 != guint_val)
	{
		validated_guint = validate_value(ICON_HEIGHT, V_INT_UNSIGNED, &guint_val);
		if (NULL == validated_guint)
		{
			g_string_printf(message, "%s ED199: %s", _("Error"), _("There was something wrong with the icon height value stored in the preferences.  Using default preferences instead."));
			display_warning(message->str);
			usable_input = FALSE;
		} else
		{
			valid_icon_height = *validated_guint;
			g_free(validated_guint);
		}
	}
	else
	{
		// No thumbnail width stored, so we use a reasonable default
		valid_icon_height = 30;
	}

	// Retrieve the new screenshot delay input
	guint_val = gconf_engine_get_int(gconf_engine, "/apps/salasaga/defaults/screenshot_delay", NULL);
	if (0 != guint_val)
	{
		validated_guint = validate_value(SCREENSHOT_DELAY, V_INT_UNSIGNED, &guint_val);
		if (NULL == validated_guint)
		{
			g_string_printf(message, "%s ED428: %s", _("Error"), _("There was something wrong with the screenshot delay value stored in the preferences.  Using default preferences instead."));
			display_warning(message->str);
			usable_input = FALSE;
		} else
		{
			valid_screenshot_delay = *validated_guint;
			g_free(validated_guint);
		}
	}
	else
	{
		// No screenshot delay value stored, so we use a reasonable default
		valid_screenshot_delay = 5;
	}

	// Retrieve the default text foreground colour, red component
	guint_val = gconf_engine_get_int(gconf_engine, "/apps/salasaga/defaults/default_text_fg_colour_red", NULL);
	validated_guint = validate_value(COLOUR_COMP16, V_INT_UNSIGNED, &guint_val);
	if (NULL == validated_guint)
	{
		g_string_printf(message, "%s ED454: %s  %s", _("Error"), _("There was something wrong with the default text foreground color value stored in the preferences."), _("Using default preferences instead."));
		display_warning(message->str);
		usable_input = FALSE;
	} else
	{
		valid_default_text_fg_colour.red = *validated_guint;
		g_free(validated_guint);
	}

	// Retrieve the default text foreground colour, green component
	guint_val = gconf_engine_get_int(gconf_engine, "/apps/salasaga/defaults/default_text_fg_colour_green", NULL);
	validated_guint = validate_value(COLOUR_COMP16, V_INT_UNSIGNED, &guint_val);
	if (NULL == validated_guint)
	{
		g_string_printf(message, "%s ED455: %s  %s", _("Error"), _("There was something wrong with the default text foreground color value stored in the preferences."), _("Using default preferences instead."));
		display_warning(message->str);
		usable_input = FALSE;
	} else
	{
		valid_default_text_fg_colour.green = *validated_guint;
		g_free(validated_guint);
	}

	// Retrieve the default text foreground colour, blue component
	guint_val = gconf_engine_get_int(gconf_engine, "/apps/salasaga/defaults/default_text_fg_colour_blue", NULL);
	validated_guint = validate_value(COLOUR_COMP16, V_INT_UNSIGNED, &guint_val);
	if (NULL == validated_guint)
	{
		g_string_printf(message, "%s ED456: %s  %s", _("Error"), _("There was something wrong with the default text foreground color value stored in the preferences."), _("Using default preferences instead."));
		display_warning(message->str);
		usable_input = FALSE;
	} else
	{
		valid_default_text_fg_colour.blue = *validated_guint;
		g_free(validated_guint);
	}

	// Retrieve the new default text layer font size
	gfloat_val = gconf_engine_get_float(gconf_engine, "/apps/salasaga/defaults/default_text_font_size", NULL);
	if (0.0 != gfloat_val)
	{
		// A value was returned
		validated_gfloat = validate_value(FONT_SIZE, V_FLOAT_UNSIGNED, &gfloat_val);
		if (NULL == validated_gfloat)
		{
			g_string_printf(message, "%s ED457: %s  %s", _("Error"), _("There was something wrong with the default text layer font size value stored in the preferences."), _("Using default preferences instead."));
			display_warning(message->str);
			usable_input = FALSE;
		} else
		{
			valid_default_text_font_size = *validated_gfloat;
			g_free(validated_gfloat);
		}
	} else
	{
		// No value is set in the saved preferences, so we go with a sensible default
		valid_default_text_font_size = 10.0;
	}

	// Retrieve the new default text layer font face
	guint_val = gconf_engine_get_int(gconf_engine, "/apps/salasaga/defaults/default_text_font_face", NULL);
	validated_guint = validate_value(FONT_FACE, V_INT_UNSIGNED, &guint_val);
	if (NULL == validated_guint)
	{
		g_string_printf(message, "%s ED458: %s  %s", _("Error"), _("There was something wrong with the default text layer font face value stored in the preferences."), _("Using default preferences instead."));
		display_warning(message->str);
		usable_input = FALSE;
	} else
	{
		valid_default_text_font_face = *validated_guint;
		g_free(validated_guint);
	}

	// Check if the application should start maximised or not
	should_maximise = gconf_engine_get_bool(gconf_engine, "/apps/salasaga/defaults/window_maximised", NULL);

	// Check if the help test should be displayed or not
	help_text_key_data = gconf_engine_get(gconf_engine, "/apps/salasaga/defaults/display_help_text", NULL);
	if (NULL != help_text_key_data)
	{
		should_display_help_text = gconf_engine_get_bool(gconf_engine, "/apps/salasaga/defaults/display_help_text", NULL);
		gconf_value_free(help_text_key_data);
	} else
	{
		should_display_help_text = TRUE;
	}

	// Find out if the warning about not being able to set the screenshot key should be displayed
	should_keybind_warn = gconf_engine_get_bool(gconf_engine, "/apps/salasaga/defaults/screenshot_key_warning", NULL);

	// Check if all the values were validated ok
	if (FALSE == usable_input)
	{
		// Some of the saved preferences were invalid, so we won't use any of them (might be a bit overkill)
		g_string_free(valid_output_folder, TRUE);
		g_string_free(valid_project_folder, TRUE);
		g_string_free(valid_screenshot_folder, TRUE);
		if (NULL != error)
			g_error_free(error);
		return FALSE;
	}

	// * We only get here after all input is considered valid *

	// Set the project folder preference
	g_string_printf(default_project_folder, "%s", valid_project_folder->str);
	g_string_free(valid_project_folder, TRUE);

	// Set the screenshots folder preference
	g_string_printf(screenshots_folder, "%s", valid_screenshot_folder->str);
	g_string_free(valid_screenshot_folder, TRUE);

	// Set the default output folder preference
	g_string_printf(default_output_folder, "%s", valid_output_folder->str);
	g_string_free(valid_output_folder, TRUE);

	// Set the default zoom level preference
	if (0 != valid_zoom_level->len)
	{
		g_string_printf(default_zoom_level, "%s", valid_zoom_level->str);
		g_string_free(valid_zoom_level, TRUE);
	}

	// Set the default project width
	project_width = valid_project_width;

	// Set the default project height
	project_height = valid_project_height;

	// Set the default output width
	default_output_width = valid_output_width;

	// Set the default output height
	default_output_height = valid_output_height;

	// Set the default slide duration (in seconds)
	default_slide_duration = valid_slide_duration;

	// Set the default layer duration (in seconds)
	default_layer_duration = valid_layer_duration;

	// Set the default background colour
	default_bg_colour.red = valid_bg_colour.red;
	default_bg_colour.green = valid_bg_colour.green;
	default_bg_colour.blue = valid_bg_colour.blue;

	// Set the default text layer foreground colour
	default_text_fg_colour.red = valid_default_text_fg_colour.red;
	default_text_fg_colour.green = valid_default_text_fg_colour.green;
	default_text_fg_colour.blue = valid_default_text_fg_colour.blue;

	// Set the default text layer font size
	default_text_font_size = valid_default_text_font_size;

	// Set the default text layer font face
	default_text_font_face = valid_default_text_font_face;

	// Set the default film strip thumbnail width
	preview_width = valid_preview_width;

	// Set the default frames per second
	default_fps = frames_per_second = valid_default_fps;

	// Set the icon height
	icon_height = valid_icon_height;

	// Set the screenshot delay
	screenshot_delay_time = valid_screenshot_delay;

	// Set whether help text and dialogs should be displayed or not
	display_help_text = should_display_help_text;

	// Set the screenshot key bind warning
	screenshot_key_warning = should_keybind_warn;

	// Free our GConf engine
	gconf_engine_unref(gconf_engine);

#else

	// * Registry related code (windows only) *

	// Check if we have a saved configuration in the windows registry
	HKEY				hkey;
	guchar				buffer_data[1000];
	LPSTR				buffer_ptr = &buffer_data[0];
	glong				buffer_size;
	gboolean			missing_keys = FALSE;
	glong				return_code;


	// Check if the Salasaga registry keys exist
	if (!RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Salasaga", 0, KEY_QUERY_VALUE, &hkey))
	{
		// They do, so load the default values
		RegCloseKey(hkey);

		// Retrieve the value for the project folder
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Salasaga\\defaults", 0, KEY_QUERY_VALUE, &hkey))
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
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Salasaga\\defaults", 0, KEY_QUERY_VALUE, &hkey))
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
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Salasaga\\defaults", 0, KEY_QUERY_VALUE, &hkey))
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
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Salasaga\\defaults", 0, KEY_QUERY_VALUE, &hkey))
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
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Salasaga\\defaults", 0, KEY_QUERY_VALUE, &hkey))
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
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Salasaga\\defaults", 0, KEY_QUERY_VALUE, &hkey))
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
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Salasaga\\defaults", 0, KEY_QUERY_VALUE, &hkey))
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
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Salasaga\\defaults", 0, KEY_QUERY_VALUE, &hkey))
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

		// Retrieve the value for the default thumbnail width
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Salasaga\\defaults", 0, KEY_QUERY_VALUE, &hkey))
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

		// Retrieve the value for the default slide duration
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Salasaga\\defaults", 0, KEY_QUERY_VALUE, &hkey))
		{
			// Value is missing, so warn the user and set a sensible default
			missing_keys = TRUE;
			default_slide_duration = 60;
		} else
		{
			// Retrieve the value
			buffer_size = sizeof(buffer_data);
			return_code = RegQueryValueExA(hkey, "slide_duration", NULL, NULL, buffer_ptr, &buffer_size);
			if (ERROR_SUCCESS == return_code)
			{
				default_slide_duration = strtof(buffer_ptr, NULL);
			}

			// Close the registry key
			RegCloseKey(hkey);
		}

		// Retrieve the value for the default layer duration
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Salasaga\\defaults", 0, KEY_QUERY_VALUE, &hkey))
		{
			// Value is missing, so warn the user and set a sensible default
			missing_keys = TRUE;
			default_layer_duration = 60;
		} else
		{
			// Retrieve the value
			buffer_size = sizeof(buffer_data);
			return_code = RegQueryValueExA(hkey, "layer_duration", NULL, NULL, buffer_ptr, &buffer_size);
			if (ERROR_SUCCESS == return_code)
			{
				default_layer_duration = strtof(buffer_ptr, NULL);
			}

			// Close the registry key
			RegCloseKey(hkey);
		}

		// Retrieve the value for the default frames per second
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Salasaga\\defaults", 0, KEY_QUERY_VALUE, &hkey))
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
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Salasaga\\defaults", 0, KEY_QUERY_VALUE, &hkey))
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
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Salasaga\\defaults", 0, KEY_QUERY_VALUE, &hkey))
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
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Salasaga\\defaults", 0, KEY_QUERY_VALUE, &hkey))
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

		// Retrieve the value for the screenshot delay
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Salasaga\\defaults", 0, KEY_QUERY_VALUE, &hkey))
		{
			// Value is missing, so warn the user and set a sensible default
			missing_keys = TRUE;
			screenshot_delay_time = 5;
		} else
		{
			// Retrieve the value
			buffer_size = sizeof(buffer_data);
			return_code = RegQueryValueExA(hkey, "screenshot_delay", NULL, NULL, buffer_ptr, &buffer_size);
			if (ERROR_SUCCESS == return_code)
			{
				screenshot_delay_value = atoi(buffer_ptr);
			}

			// Close the registry key
			RegCloseKey(hkey);
		}

		// Retrieve the value for the initial window maximisation
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Salasaga\\defaults", 0, KEY_QUERY_VALUE, &hkey))
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
			display_warning(_("Some of the application preference registry keys are missing.  Using defaults."));
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

	// Free the memory used in this function
	g_string_free(message, TRUE);
	if (NULL != error)
		g_error_free(error);

	return TRUE;
}
