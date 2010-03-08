/*
 * $Id$
 *
 * Salasaga: Function to set and retrieve application preference values
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


// GTK includes
#include <gtk/gtk.h>

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"


// Application default preferences
static GdkColor				default_bg_colour = {0, 0, 0, 0};  // Default background colour for slides
static guint				default_fps;					// Default number of frames per second
static gfloat				default_layer_duration;			// Length of all new layers, in frames
static GString				*default_output_folder;			// Application default save path for exporting animations
static guint				default_output_height;			// Application default for how high to create project output
static guint				default_output_width;			// Application default for how wide to create project output
static GString				*default_project_folder;		// Application default save path for project folders
static gfloat				default_slide_duration;			// Default length of all new slides, in frames
static GString				*default_zoom_level;			// Default zoom level to use
static guint				icon_height = 30;				// Height in pixels for the toolbar icons (they're scalable SVG's)
static guint				preview_width;					// Width in pixel for the film strip preview (might turn into a structure later)
static GString				*screenshots_folder;			// Application default for where to store screenshots


// Functions to get and set the variables
GdkColor *get_default_bg_colour()
{
	GdkColor			*copied_colour;			// Holds a newly allocated copy of the colour


	// Allocate memory for a new copy of the default background colour structure
	copied_colour = g_new0(GdkColor, 1);

	// Copy the values from the default background structure to the newly allocated memory
	copied_colour->pixel = default_bg_colour.pixel;
	copied_colour->red = default_bg_colour.red;
	copied_colour->green = default_bg_colour.green;
	copied_colour->blue = default_bg_colour.blue;

	// Return the address of the newly populated colour structure
	return copied_colour;
}

guint32 get_default_bg_colour_pixel()
{
	return default_bg_colour.pixel;
}

guint16 get_default_bg_colour_red()
{
	return default_bg_colour.red;
}

guint16 get_default_bg_colour_green()
{
	return default_bg_colour.green;
}

guint16 get_default_bg_colour_blue()
{
	return default_bg_colour.blue;
}

guint get_default_fps()
{
	return default_fps;
}

gfloat get_default_layer_duration()
{
	return default_layer_duration;
}

gchar *get_default_output_folder()
{
	return default_output_folder->str;
}

gsize get_default_output_folder_length()
{
	return default_output_folder->len;
}

guint get_default_output_height()
{
	return default_output_height;
}

guint get_default_output_width()
{
	return default_output_width;
}

gchar *get_default_project_folder()
{
	return default_project_folder->str;
}

gsize get_default_project_folder_length()
{
	return default_project_folder->len;
}

gfloat get_default_slide_duration()
{
	return default_slide_duration;
}

gchar *get_default_zoom_level()
{
	return default_zoom_level->str;
}

gsize get_default_zoom_level_length()
{
	return default_zoom_level->len;
}

guint get_icon_height()
{
	return icon_height;
}

guint get_preview_width()
{
	return preview_width;
}

gchar *get_screenshots_folder()
{
	return screenshots_folder->str;
}

gsize get_screenshots_folder_length()
{
	return screenshots_folder->len;
}

void set_default_bg_colour(GdkColor *new_default_bg_colour)
{
	default_bg_colour.pixel = new_default_bg_colour->pixel;
	default_bg_colour.red = new_default_bg_colour->red;
	default_bg_colour.green = new_default_bg_colour->green;
	default_bg_colour.blue = new_default_bg_colour->blue;
}

void set_default_bg_colour_pixel(guint32 new_pixel)
{
	default_bg_colour.pixel = new_pixel;
}

void set_default_bg_colour_red(guint16 new_red)
{
	default_bg_colour.red = new_red;
}

void set_default_bg_colour_green(guint16 new_green)
{
	default_bg_colour.green = new_green;
}

void set_default_bg_colour_blue(guint16 new_blue)
{
	default_bg_colour.blue = new_blue;
}

void set_default_fps(guint new_default_fps)
{
	default_fps = new_default_fps;
}

void set_default_layer_duration(gfloat new_default_layer_duration)
{
	default_layer_duration = new_default_layer_duration;
}

void set_default_output_folder(gchar *new_default_output_folder)
{
	if (NULL == default_output_folder)
	{
		default_output_folder = g_string_new(new_default_output_folder);
	} else
	{
		default_output_folder = g_string_assign(default_output_folder, new_default_output_folder);
	}
}

void set_default_output_height(guint new_default_output_height)
{
	default_output_height = new_default_output_height;
}

void set_default_output_width(guint new_default_output_width)
{
	default_output_width = new_default_output_width;
}

void set_default_project_folder(gchar *new_default_project_folder)
{
	if (NULL == default_project_folder)
	{
		default_project_folder = g_string_new(new_default_project_folder);
	} else
	{
		default_project_folder = g_string_assign(default_project_folder, new_default_project_folder);
	}
}

void set_default_slide_duration(gfloat new_default_slide_duration)
{
	default_slide_duration = new_default_slide_duration;
}

void set_default_zoom_level(gchar *new_default_zoom_level)
{
	if (NULL == default_zoom_level)
	{
		default_zoom_level = g_string_new(new_default_zoom_level);
	} else
	{
		default_zoom_level = g_string_assign(default_zoom_level, new_default_zoom_level);
	}
}

void set_icon_height(guint new_icon_height)
{
	icon_height = new_icon_height;
}

void set_preview_width(guint new_preview_width)
{
	preview_width = new_preview_width;
}

void set_screenshots_folder(gchar *new_screenshots_folder)
{
	if (NULL == screenshots_folder)
	{
		screenshots_folder = g_string_new(new_screenshots_folder);
	} else
	{
		screenshots_folder = g_string_assign(screenshots_folder, new_screenshots_folder);
	}
}
