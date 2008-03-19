/*
 * $Id$
 *
 * Salasaga: Detect when the user moves a pressed down mouse button on the drawing area 
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


// Turn on C99 compatibility - needed for roundf() to work
#define _ISOC99_SOURCE

// Math include
#include <math.h>

// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"


gboolean working_area_motion_notify_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	// Local variables
	GdkModifierType		button_state;				// Mouse button states
	static GdkGC		*line_gc = NULL;
	GdkSegment			lines[4];
	gint				mouse_x;					// Unscaled mouse x position
	gint				mouse_y;					// Unscaled mouse x position
	GdkRectangle		tmp_rectangle;


	// Find out where the mouse is positioned, and which buttons and modifier keys are down (active)
	gdk_window_get_pointer(event->window, &mouse_x, &mouse_y, &button_state);

	// If we're creating a new highlight layer, draw a bounding box
	if (TYPE_HIGHLIGHT == new_layer_selected)
	{
		// Ensure the invalidation (redraw) area is set to the maximum size that has been selected for this highlight
		if (mouse_x < invalidation_start_x)
		{
			invalidation_start_x = mouse_x - 1;
		}
		if (mouse_y < invalidation_start_y)
		{
			invalidation_start_y = mouse_y - 1;
		}
		if (mouse_x > invalidation_end_x)
		{
			invalidation_end_x = mouse_x;
		}
		if (mouse_y > invalidation_end_y)
		{
			invalidation_end_y = mouse_y;
		}

		// Ensure the invalidation area can't go out of bounds
		if (1 > invalidation_start_x)
		{
			invalidation_start_x = 1;
		}
		if ((main_drawing_area->allocation.width - 1) < invalidation_start_x)
		{
			invalidation_start_x = main_drawing_area->allocation.width - 1;
		}
		if (1 > invalidation_start_y)
		{
			invalidation_start_y = 1;
		}
		if ((main_drawing_area->allocation.height - 1) < invalidation_start_y)
		{
			invalidation_start_y = main_drawing_area->allocation.height - 1;
		}
		if (1 > invalidation_end_x)
		{
			invalidation_end_x = 1;
		}
		if ((main_drawing_area->allocation.width - 1) < invalidation_end_x)
		{
			invalidation_end_x = main_drawing_area->allocation.width - 1;
		}
		if (1 > invalidation_end_y)
		{
			invalidation_end_y = 1;
		}
		if ((main_drawing_area->allocation.height - 1) < invalidation_end_y)
		{
			invalidation_end_y = main_drawing_area->allocation.height - 1;
		}

		// Restore the front store area we're going over from the backing store
		gdk_draw_pixbuf(GDK_DRAWABLE(front_store), NULL, GDK_PIXBUF(backing_store),
				invalidation_start_x - 1, invalidation_start_y - 1,
				invalidation_start_x, invalidation_start_y,
				(invalidation_end_x - invalidation_start_x) + 1, (invalidation_end_y - invalidation_start_y) + 1,
				GDK_RGB_DITHER_NONE, 0, 0);

		// Draw a bounding box on the front store
		if (NULL == line_gc)
		{
			line_gc = gdk_gc_new(GDK_DRAWABLE(front_store));
		}
		gdk_gc_set_function(line_gc, GDK_INVERT);
		lines[0].x1 = stored_x;
		lines[0].y1 = stored_y;
		lines[0].x2 = mouse_x;
		lines[0].y2 = stored_y;
		lines[1].x1 = mouse_x;
		lines[1].y1 = stored_y;
		lines[1].x2 = mouse_x;
		lines[1].y2 = mouse_y;
		lines[2].x1 = mouse_x;
		lines[2].y1 = mouse_y;
		lines[2].x2 = stored_x;
		lines[2].y2 = mouse_y;
		lines[3].x1 = stored_x;
		lines[3].y1 = mouse_y;
		lines[3].x2 = stored_x;
		lines[3].y2 = stored_y;
		gdk_draw_segments(GDK_DRAWABLE(front_store), line_gc, lines, 4);

		// Tell the window system to display the updated front store area
		tmp_rectangle.x = invalidation_start_x;
		tmp_rectangle.y = invalidation_start_y;
		tmp_rectangle.width = (invalidation_end_x - invalidation_start_x) + 1;
		tmp_rectangle.height = (invalidation_end_y - invalidation_start_y) + 1;
		gdk_window_invalidate_rect(main_drawing_area->window, &tmp_rectangle, TRUE);

		return TRUE;
	}

	// If we're already aware of a mouse drag operation going on, then return
	if (TRUE == mouse_dragging)
	{
		return TRUE;
	}

	// * To get here, this must be the first time we've heard of this particular mouse drag *

	// Check if the primary mouse button is down
	if (GDK_BUTTON1_MASK & button_state)
	{
		// It is, so we take notice of the mouse drag operation and return
		mouse_dragging = TRUE;
	}

	return TRUE;
}
