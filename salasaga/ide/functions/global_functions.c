/*
 * $Id$
 *
 * Salasaga: Functions for accessing and setting globally available variables
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
#include "../salasaga_types.h"
#include "../externs.h"

// Global variables
static guint			capture_height;				// Height for screen captures
static guint			capture_width;				// Width for screen captures
static guint			capture_x;
static guint			capture_y;
static gboolean			changes_made = FALSE;		// Have there been changes to the project since the last save?


// Functions to get and set the variables
guint get_capture_height()
{
	return capture_height;
}

guint get_capture_width()
{
	return capture_width;
}

guint get_capture_x()
{
	return capture_x;
}

guint get_capture_y()
{
	return capture_y;
}

gboolean get_changes_made()
{
	return changes_made;
}

void set_capture_height(guint new_capture_height)
{
	capture_height = new_capture_height;
}

void set_capture_width(guint new_capture_width)
{
	capture_width = new_capture_width;
}

void set_capture_x(guint new_capture_x)
{
	capture_x = new_capture_x;
}

void set_capture_y(guint new_capture_y)
{
	capture_y = new_capture_y;
}

void set_changes_made(gboolean new_changes_made)
{
	changes_made = new_changes_made;
}
