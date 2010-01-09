/*
 * $Id$
 *
 * Salasaga: Function to draw a bounding box directly onto the drawing area widget
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
#include "../working_area/draw_layer_start_and_end_points.h"


gboolean draw_bounding_box(gint left, gint top, gint right, gint bottom)
{
	// Local variables
	GdkRectangle		line_clip_region;			// Used as a clip mask region
	static GdkGC		*line_gc = NULL;
	GdkSegment			lines[4];					// Holds the lines used to draw the border
	gint				pixmap_height;				// Height of the front store
	gint				pixmap_width;				// Width of the front store
	gint				swap_value;					// Temp location while we swap galues

	// Initialise some things
	gdk_drawable_get_size(GDK_PIXMAP(front_store), &pixmap_width, &pixmap_height);

	// Swap around the top and side positions if we need to
	if (left > right)
	{
		swap_value = left;
		left = right;
		right = swap_value;
	}
	if (top > bottom)
	{
		swap_value = top;
		top = bottom;
		bottom = swap_value;
	}

	// If the bounding box is completely outside of the slide area onscreen, skip drawing it
	if ((left >= pixmap_width) || (right <= 1) || (top >= pixmap_height) || (bottom <= 1))
	{
		return TRUE;
	}

	// Ensure the invalidation (redraw) area is set to the maximum size that has been selected
	if (left - 2 < invalidation_start_x)
	{
		invalidation_start_x = left - 2;
	}
	if (top - 2 < invalidation_start_y)
	{
		invalidation_start_y = top - 2;
	}
	if (right + 2 > invalidation_end_x)
	{
		invalidation_end_x = right + 2;
	}
	if (bottom + 2 > invalidation_end_y)
	{
		invalidation_end_y = bottom + 2;
	}

	// Ensure the invalidation area can't go out of bounds
	invalidation_start_x = CLAMP(invalidation_start_x, 1, pixmap_width - 1);
	invalidation_start_y = CLAMP(invalidation_start_y, 1, pixmap_height - 1);
	invalidation_end_x = CLAMP(invalidation_end_x, 1, pixmap_width - 1);
	invalidation_end_y = CLAMP(invalidation_end_y, 1, pixmap_height - 1);

	// Restore the widget area we're going over from the front store
	gdk_draw_drawable(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(main_drawing_area->style->fg_gc[GTK_WIDGET_STATE(main_drawing_area)]),
		GDK_PIXMAP(front_store),
		invalidation_start_x, invalidation_start_y,
		invalidation_start_x, invalidation_start_y,
		(invalidation_end_x - invalidation_start_x) + 1, (invalidation_end_y - invalidation_start_y) + 1);

	// Draw a bounding box directly onto the widget
	if (NULL == line_gc)
	{
		line_gc = gdk_gc_new(GDK_DRAWABLE(main_drawing_area->window));
	}
	gdk_gc_set_function(GDK_GC(line_gc), GDK_INVERT);

	// Set a clip mask
	line_clip_region.x = 1;
	line_clip_region.y = 1;
	line_clip_region.width = (gint) pixmap_width - 2;
	line_clip_region.height = (gint) pixmap_height - 2;
	gdk_gc_set_clip_rectangle(GDK_GC(line_gc), &line_clip_region);

	// Draw as much of the top line as fits in the slide area onscreen
	lines[0].x1 = left;
	lines[0].y1 = top;
	lines[0].x2 = right;
	lines[0].y2 = top;
	gdk_draw_segments(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(line_gc), &lines[0], 1);

	// Draw as much of the right hand side line as fits in the slide area onscreen
	lines[1].x1 = right;
	lines[1].y1 = top;
	lines[1].x2 = right;
	lines[1].y2 = bottom;
	gdk_draw_segments(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(line_gc), &lines[1], 1);

	// Draw as much of the bottom line as fits in the slide area onscreen
	lines[2].x1 = left;
	lines[2].y1 = bottom;
	lines[2].x2 = right;
	lines[2].y2 = bottom;
	gdk_draw_segments(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(line_gc), &lines[2], 1);

	// Draw as much of the left hand side line as fits in the slide area onscreen
	lines[3].x1 = left;
	lines[3].y1 = top;
	lines[3].x2 = left;
	lines[3].y2 = bottom;
	gdk_draw_segments(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(line_gc), &lines[3], 1);

	// Draw the start and end points for the layer
	draw_layer_start_and_end_points();

	return TRUE;
}
