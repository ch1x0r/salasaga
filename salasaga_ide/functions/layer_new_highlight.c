/*
 * $Id$
 *
 * Salasaga: Function called when the user clicks the Add Highlight Layer toolbar button 
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

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"


void layer_new_highlight(void)
{
	// Local variables
	GdkCursor			*selection_cursor;			// Holds the new temporary selection cursor


	// If no project is loaded then don't run this function
	if (NULL == current_slide)
	{
		// Make a beep, then return
		gdk_beep();
		return;
	}

	// Change the cursor to something suitable
	selection_cursor = gdk_cursor_new(GDK_CROSSHAIR);
	gdk_window_set_cursor(main_drawing_area->window, selection_cursor);
	gdk_cursor_unref(selection_cursor);

	// Set a toggle so the release button callback will know to do the rest
	new_layer_selected = TYPE_HIGHLIGHT;

	// Update the status bar
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, " Please draw the new layer");
	gdk_flush();
}
