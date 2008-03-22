/*
 * $Id$
 *
 * Salasaga: Function to draw a bounding box in the drawing area, via the front store 
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


gboolean draw_bounding_box(gint left, gint top, gint right, gint bottom)
{
	// Local variables
	static GdkGC		*line_gc = NULL;
	GdkSegment			lines[4];
	GdkRectangle		tmp_rectangle;				// Temporary rectangle used for invalidating part of the front store


	// Ensure the invalidation (redraw) area is set to the maximum size that has been selected for this highlight
	if (right < invalidation_start_x)
	{
		invalidation_start_x = right - 1;
	}
	if (bottom < invalidation_start_y)
	{
		invalidation_start_y = bottom - 1;
	}
	if (right > invalidation_end_x)
	{
		invalidation_end_x = right;
	}
	if (bottom > invalidation_end_y)
	{
		invalidation_end_y = bottom;
	}

	// Ensure the invalidation area can't go out of bounds
	invalidation_start_x = CLAMP(invalidation_start_x, 1, main_drawing_area->allocation.width - 1);
	invalidation_start_y = CLAMP(invalidation_start_y, 1, main_drawing_area->allocation.height - 1);
	invalidation_end_x = CLAMP(invalidation_end_x, 1, main_drawing_area->allocation.width - 1);
	invalidation_end_y = CLAMP(invalidation_end_y, 1, main_drawing_area->allocation.height - 1);

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
	lines[0].x1 = left;
	lines[0].y1 = top;
	lines[0].x2 = right;
	lines[0].y2 = top;
	lines[1].x1 = right;
	lines[1].y1 = top;
	lines[1].x2 = right;
	lines[1].y2 = bottom;
	lines[2].x1 = right;
	lines[2].y1 = bottom;
	lines[2].x2 = left;
	lines[2].y2 = bottom;
	lines[3].x1 = left;
	lines[3].y1 = bottom;
	lines[3].x2 = left;
	lines[3].y2 = top;
	gdk_draw_segments(GDK_DRAWABLE(front_store), line_gc, lines, 4);

	// Tell the window system to display the updated front store area
	tmp_rectangle.x = invalidation_start_x;
	tmp_rectangle.y = invalidation_start_y;
	tmp_rectangle.width = (invalidation_end_x - invalidation_start_x) + 1;
	tmp_rectangle.height = (invalidation_end_y - invalidation_start_y) + 1;
	gdk_window_invalidate_rect(main_drawing_area->window, &tmp_rectangle, TRUE);

	return TRUE;
}
