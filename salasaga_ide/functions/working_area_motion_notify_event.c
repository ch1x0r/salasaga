/*
 * $Id$
 *
 * Flame Project: Detect when the user moves a pressed down mouse button on the drawing area 
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


gboolean working_area_motion_notify_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	// Local variables
	GdkModifierType		button_state;
	gint					mouse_x;
	gint					mouse_y;


	// If we're already aware of a mouse drag operation going on, then return
	if (TRUE == mouse_dragging)
	{
		return TRUE;
	}

	// * To get here, this must be the first time we've heard of this particular mouse drag *

	// Find out where the mouse is positioned, and which buttons and modifier keys are down (active)
	gdk_window_get_pointer(event->window, &mouse_x, &mouse_y, &button_state);

	// Check if the primary mouse button is down
	if (GDK_BUTTON1_MASK & button_state)
	{
		// It is, so we take notice of the mouse drag operation and return
		mouse_dragging = TRUE;
	}

	return TRUE;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.2  2007/09/29 04:22:12  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 * Revision 1.1  2007/09/28 12:05:06  vapour
 * Broke callbacks.c and callbacks.h into its component functions.
 *
 */
