/*
 * $Id$
 *
 * Salasaga: Function called when the user chooses moves the film strip handle 
 * 
 * Copyright (C) 2007-2008 Justin Clift <justin@salasaga.org>
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

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"


void film_strip_handle_changed(GObject *paned, GParamSpec *pspec, gpointer data)
{
	// Temporary variables
	gint				new_position;

	// Get the new position of the film strip seperator
	new_position = gtk_paned_get_position(GTK_PANED(paned));

	// If the handle has moved, set the new thumbnail width in the application preferences
	if (new_position != preview_width)
	{
		// Set a toggle to indicate the film strip width is being changed
		film_strip_being_resized = TRUE;
		preview_width = new_position;
	}
}
