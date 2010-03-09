/*
 * $Id$
 *
 * Salasaga: Function to set and retrieve project preference values
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
#include "../global_functions.h"


// Project preference variables
static GString				*info_link;						// The URL to open when the user clicks the "Information" box
static GString				*info_link_target;				// The target frame to open the URL in when the user clicks the "Information" box
static GString				*output_folder = NULL;			// Where to export output files too
static guint				output_height;					// How high to create project output
static guint				output_width;					// How wide to create project output
static GString				*project_folder = NULL;			// The path to the project folder
static guint				project_height;					// The height of the project in pixels
static GString				*project_name = NULL;			// The name of the project
static guint				project_width;					// The width of the project in pixels


// Functions to get and set the variables
gchar *get_info_link()
{
	return info_link->str;
}

gsize get_info_link_length()
{
	return info_link->len;
}

gchar *get_info_link_target()
{
	return info_link_target->str;
}

gsize get_info_link_target_length()
{
	return info_link_target->len;
}

gchar *get_output_folder()
{
	return output_folder->str;
}

gsize get_output_folder_length()
{
	return output_folder->len;
}

guint get_output_height()
{
	return output_height;
}

guint get_output_width()
{
	return output_width;
}

gchar *get_project_folder()
{
	return project_folder->str;
}

gsize get_project_folder_length()
{
	return project_folder->len;
}

guint get_project_height()
{
	return project_height;
}

gchar *get_project_name()
{
	return project_name->str;
}

gsize get_project_name_length()
{
	return project_name->len;
}

guint get_project_width()
{
	return project_width;
}

void set_info_link(gchar *new_info_link)
{
	if (NULL == info_link)
	{
		info_link = g_string_new(new_info_link);
	} else
	{
		info_link = g_string_assign(info_link, new_info_link);
	}
}

void set_info_link_target(gchar *new_info_link_target)
{
	if (NULL == info_link_target)
	{
		info_link_target = g_string_new(new_info_link_target);
	} else
	{
		info_link_target = g_string_assign(info_link_target, new_info_link_target);
	}
}

void set_output_folder(gchar *new_output_folder)
{
	if (NULL == output_folder)
	{
		output_folder = g_string_new(new_output_folder);
	} else
	{
		output_folder = g_string_assign(output_folder, new_output_folder);
	}
}

void set_output_height(guint new_output_height)
{
	output_height = new_output_height;
}

void set_output_width(guint new_output_width)
{
	output_width = new_output_width;
}

void set_project_folder(gchar *new_project_folder)
{
	if (NULL == project_folder)
	{
		project_folder = g_string_new(new_project_folder);
	} else
	{
		project_folder = g_string_assign(project_folder, new_project_folder);
	}
}

void set_project_height(guint new_project_height)
{
	project_height = new_project_height;
}

void set_project_name(gchar *new_project_name)
{
	if (NULL == project_name)
	{
		project_name = g_string_new(new_project_name);
	} else
	{
		project_name = g_string_assign(project_name, new_project_name);
	}
}

void set_project_width(guint new_project_width)
{
	project_width = new_project_width;
}
