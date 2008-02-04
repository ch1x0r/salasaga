/*
 * $Id$
 *
 * Flame Project: Redraw the working area from the backing store 
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


gboolean working_area_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	// Only do this function if we have a backing store available
	if (NULL == backing_store)
	{
		return FALSE;
	}

	// Draw the invalidated backing store area onto the working area
	gdk_draw_drawable(GDK_DRAWABLE(widget->window), GDK_GC(widget->style->fg_gc[GTK_WIDGET_STATE(widget)]),
		GDK_PIXMAP(backing_store),
		event->area.x, event->area.y,
		event->area.x, event->area.y,
		event->area.width, event->area.height);

	return FALSE;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.4  2008/02/04 17:22:38  vapour
 *  + Removed unnecessary includes.
 *
 * Revision 1.3  2008/01/15 16:18:59  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.2  2007/09/29 04:22:12  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 * Revision 1.1  2007/09/28 12:05:05  vapour
 * Broke callbacks.c and callbacks.h into its component functions.
 *
 */
