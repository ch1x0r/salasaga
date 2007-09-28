/*
 * $Id$
 *
 * Flame Project: Function called when the drawing area is resized 
 * 
 * Copyright (C) 2007 Justin Clift <justin@postgresql.org>
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
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

// GTK includes
#include <gtk/gtk.h>

// GConf include (not for windows)
#ifndef _WIN32
	#include <gconf/gconf.h>
#else
	// Windows only code
	#include <windows.h>
#endif

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"
#include "../gui-functions.h"


gint event_size_allocate_received(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	// Local variables
	static gint			old_width = 0;


	// Check if the width of the drawing area has changed
	if (old_width != right_side->allocation.width)
	{
		// The width has been changed, so recalculate the zoom and redraw the drawing area
		zoom_selector_changed(GTK_WIDGET(zoom_selector), NULL, (gpointer) NULL);
		old_width = right_side->allocation.width;

		// Resize the drawing area so it draws properly
		gtk_widget_set_size_request(GTK_WIDGET(main_drawing_area), working_width, working_height);

	}

	// Return FALSE to continue event propagation
	return FALSE;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.1  2007/09/28 12:05:08  vapour
 * Broke callbacks.c and callbacks.h into its component functions.
 *
 */
