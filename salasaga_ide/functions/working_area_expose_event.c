/*
 * $Id$
 *
 * Salasaga: Redraw the working area from the front store 
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


gboolean working_area_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	// Only do this function if we have a front store available
	if (NULL == front_store)
	{
		return FALSE;
	}

	// Draw the invalidated front store area onto the working area
	gdk_draw_drawable(GDK_DRAWABLE(widget->window), GDK_GC(widget->style->fg_gc[GTK_WIDGET_STATE(widget)]),
		GDK_PIXMAP(front_store),
		event->area.x, event->area.y,
		event->area.x, event->area.y,
		event->area.width, event->area.height);

	return FALSE;
}
