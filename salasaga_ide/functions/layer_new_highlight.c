/*
 * $Id$
 *
 * Flame Project: Function called when the user clicks the Add Highlight Layer toolbar button 
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

// Flame Edit includes
#include "../flame-types.h"
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


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.11  2008/03/05 10:11:00  vapour
 * Moved the code for the highlight, mouse, and text layers into an inner version of each function.
 *
 * Revision 1.10  2008/03/03 02:51:10  vapour
 * Added status bar feedback message.
 *
 * Revision 1.9  2008/02/16 11:12:53  vapour
 * Replaced our sound beep function with the inbuilt gdk sound beep one.
 *
 * Revision 1.8  2008/02/12 14:11:51  vapour
 * Updated to use the new visibility and background fields in the layer structure.
 *
 * Revision 1.7  2008/02/12 05:21:52  vapour
 * Adjusted to work with the new, slightly simplified layer structure.
 *
 * Revision 1.6  2008/02/04 16:56:57  vapour
 *  + Removed unnecessary includes.
 *
 * Revision 1.5  2008/02/01 10:42:40  vapour
 * Added a new field, the target window to open the external link in, defaulting to _self.
 *
 * Revision 1.4  2008/01/21 10:27:56  vapour
 *  + Fixed a bug, so that new layers longer than the existing slide duration will extend out the slide duration.
 *  + Delegated creation of timeline duration image to main duration image regen function.
 *  + Simplified things somewhat.
 *
 * Revision 1.3  2008/01/15 16:19:07  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.2  2007/10/06 11:39:27  vapour
 * Continued adjusting function include definitions.
 *
 * Revision 1.1  2007/09/29 04:22:17  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */
