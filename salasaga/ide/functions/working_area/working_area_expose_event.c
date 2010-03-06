/*
 * $Id$
 *
 * Salasaga: Redraw the working area from the front store 
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

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "draw_handle_box.h"
#include "draw_layer_start_and_end_points.h"


gboolean working_area_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	// Only do this function if we have a front store available and a project loaded
	if ((NULL == front_store) || (FALSE == get_project_active()))
	{
		return TRUE;
	}

	// Draw the invalidated front store area onto the working area
	gdk_draw_drawable(GDK_DRAWABLE(widget->window), GDK_GC(widget->style->fg_gc[GTK_WIDGET_STATE(widget)]),
		GDK_PIXMAP(front_store),
		event->area.x, event->area.y,
		event->area.x, event->area.y,
		event->area.width, event->area.height);

	// If we're creating a new layer, we don't need to draw a handle box nor layer start and end points
	if (TYPE_NONE == get_new_layer_selected())
	{
		// Draw a handle box around the selected layer
		draw_handle_box();

		// Draw the start and end points for the layer
		draw_layer_start_and_end_points();
	}

	return FALSE;
}
