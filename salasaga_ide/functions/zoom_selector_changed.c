/*
 * $Id$
 *
 * Flame Project: Function called when the user chooses a new zoom level 
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


// Standard includes
#include <stdlib.h>

// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"
#include "draw_workspace.h"


gint zoom_selector_changed(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	// Temporary variables
	gint32		tmp_int;
	GString		*tmp_string;

	// Get the new zoom level
	tmp_string = g_string_new(NULL);
	g_string_printf(tmp_string, "%s", gtk_combo_box_get_active_text(GTK_COMBO_BOX(zoom_selector)));

	// Parse and store the new zoom level
	tmp_int = g_ascii_strncasecmp(tmp_string->str, "F", 1);
	if (0 == tmp_int)
	{
		// "Fit to width" is selected, so work out a new zoom level by figuring out how much space the widget really has
		//  (Look at the alloation of it's parent widget)
		//  Reduce the width calculated by 24 pixels (guessed) to give space for widget borders and such
		zoom = (guint) (((float) (right_side->allocation.width - 24) / (float) project_width) * 100);
	} else
	{
		tmp_string = g_string_truncate(tmp_string, tmp_string->len - 1);
		zoom = atoi(tmp_string->str);
	}

	// Free the memory allocated in this function
	g_string_free(tmp_string, TRUE);

	// Calculate and set the display size of the working area
	working_width = (project_width * zoom) / 100;
	working_height = (project_height * zoom) / 100;

	// Resize the drawing area so it draws properly
	gtk_widget_set_size_request(GTK_WIDGET(main_drawing_area), working_width, working_height);

	// Free the existing backing store for the workspace
	if (NULL != backing_store)
	{
		g_object_unref(backing_store);
		backing_store = NULL;
	}

	// Redraw the workspace area
	draw_workspace();

	// Indicate to the calling routine that this function finished fine
	return TRUE;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.7  2008/02/27 13:46:46  vapour
 * Fixed a small bug where the drawing area wasn't always being sized correctly at startup until the window had been resized.
 *
 * Revision 1.6  2008/02/07 12:40:03  vapour
 * Updated to free the backing store when needed.
 *
 * Revision 1.5  2008/02/04 17:23:41  vapour
 *  + Removed unnecessary includes.
 *
 * Revision 1.4  2008/01/15 16:18:59  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.3  2007/10/06 11:36:39  vapour
 * Continued adjusting function include definitions.
 *
 * Revision 1.2  2007/09/29 04:22:12  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 * Revision 1.1  2007/09/28 12:05:06  vapour
 * Broke callbacks.c and callbacks.h into its component functions.
 *
 */
