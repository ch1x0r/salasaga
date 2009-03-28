/*
 * $Id$
 *
 * Salasaga: Draws resize handles on a bounding box
 *
 * Copyright (C) 2005-2008 Justin Clift <justin@salasaga.org>
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


// Turn on C99 compatibility - needed for roundf() to work
#define _ISOC99_SOURCE

// Math include
#include <math.h>

// GTK includes
#include <gtk/gtk.h>

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"


gboolean draw_resize_handles(gint left, gint top, gint right, gint bottom)
{
	// Local variables
	static GdkGC		*handle_gc = NULL;
	guint				height;						// Height of the bounding box
	guint				mid_point_horizontal;
	guint				mid_point_vertical;
	gint				pixmap_height;				// Height of the front store
	gint				pixmap_width;				// Width of the front store
	guint				required_size_mid_point;	// Minimum size we need in order to draw mid point handles
	guint				width;						// Width of the bounding box


	// Initialise some things
	gdk_drawable_get_size(GDK_PIXMAP(front_store), &pixmap_width, &pixmap_height);

	// Draw the handles directly onto the widget
	if (NULL == handle_gc)
	{
		handle_gc = gdk_gc_new(GDK_DRAWABLE(main_drawing_area->window));
	}
	gdk_gc_set_function(GDK_GC(handle_gc), GDK_INVERT);

	// * Generate the list of corner resize handle positions *

	// Top left
	resize_handles_rect[0].x = left + 1;
	resize_handles_rect[0].y = top + 1;
	resize_handles_rect[0].width = resize_handle_size - 1;
	resize_handles_rect[0].height = resize_handle_size - 1;

	// Top right
	resize_handles_rect[2].x = right - resize_handle_size + 1;
	resize_handles_rect[2].y = top + 1;
	resize_handles_rect[2].width = resize_handle_size - 1;
	resize_handles_rect[2].height = resize_handle_size - 1;

	// Bottom right
	resize_handles_rect[4].x = right - resize_handle_size + 1;
	resize_handles_rect[4].y = bottom - resize_handle_size + 1;
	resize_handles_rect[4].width = resize_handle_size - 1;
	resize_handles_rect[4].height = resize_handle_size - 1;

	// Bottom left
	resize_handles_rect[6].x = left + 1;
	resize_handles_rect[6].y = bottom - resize_handle_size + 1;
	resize_handles_rect[6].width = resize_handle_size - 1;
	resize_handles_rect[6].height = resize_handle_size - 1;


	// * Draw the corner handles *

	// Top left
	if (((resize_handles_rect[0].x + resize_handle_size) < pixmap_width) && (resize_handles_rect[0].x >= 1)
			&& (resize_handles_rect[0].y >= 1)
			&& ((resize_handles_rect[0].y + resize_handle_size) < pixmap_height))
	{
		gdk_draw_rectangle(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(handle_gc), TRUE,
			resize_handles_rect[0].x, resize_handles_rect[0].y,
			resize_handles_rect[0].width, resize_handles_rect[0].height);
	}

	// Top right
	if (((resize_handles_rect[2].x + resize_handle_size) < pixmap_width) && (resize_handles_rect[2].x >= 1)
			&& (resize_handles_rect[2].y >= 1)
			&& ((resize_handles_rect[2].y + resize_handle_size) < pixmap_height))
	{
		gdk_draw_rectangle(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(handle_gc), TRUE,
			resize_handles_rect[2].x, resize_handles_rect[2].y,
			resize_handles_rect[2].width, resize_handles_rect[2].height);
	}

	// Bottom right
	if (((resize_handles_rect[4].x + resize_handle_size) < pixmap_width) && (resize_handles_rect[4].x >= 1)
			&& (resize_handles_rect[4].y >= 1)
			&& ((resize_handles_rect[4].y + resize_handle_size) < pixmap_height))
	{
		gdk_draw_rectangle(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(handle_gc), TRUE,
			resize_handles_rect[4].x, resize_handles_rect[4].y,
			resize_handles_rect[4].width, resize_handles_rect[4].height);
	}

	// Bottom left
	if (((resize_handles_rect[6].x + resize_handle_size) < pixmap_width) && (resize_handles_rect[6].x >= 1)
			&& (resize_handles_rect[6].y >= 1)
			&& ((resize_handles_rect[6].y + resize_handle_size) < pixmap_height))
	{
		gdk_draw_rectangle(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(handle_gc), TRUE,
			resize_handles_rect[6].x, resize_handles_rect[6].y,
			resize_handles_rect[6].width, resize_handles_rect[6].height);
	}

	// * If there is enough room, draw the mid point handles *
	required_size_mid_point = (resize_handle_size * 3) + 2;

	// Draw the horizontal resize handles
	width = bottom - top;
	if (required_size_mid_point < width)
	{
		// Calculate where to place the horizontal resize handles
		mid_point_vertical = top + roundf(((gfloat) width - resize_handle_size) / 2);

		// Right middle
		resize_handles_rect[3].x = right - resize_handle_size + 1;
		resize_handles_rect[3].y = mid_point_vertical;
		resize_handles_rect[3].width = resize_handle_size - 1;
		resize_handles_rect[3].height = resize_handle_size;

		// Left middle
		resize_handles_rect[7].x = left + 1;
		resize_handles_rect[7].y = mid_point_vertical;
		resize_handles_rect[7].width = resize_handle_size - 1;
		resize_handles_rect[7].height = resize_handle_size;

		// Right middle
		if (((resize_handles_rect[3].x + resize_handle_size) < pixmap_width) && (resize_handles_rect[3].x >= 1)
				&& (resize_handles_rect[3].y >= 1)
				&& ((resize_handles_rect[3].y + resize_handle_size) < pixmap_height))
		{
			gdk_draw_rectangle(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(handle_gc), TRUE,
				resize_handles_rect[3].x, resize_handles_rect[3].y,
				resize_handles_rect[3].width, resize_handles_rect[3].height);
		}

		// Left middle
		if (((resize_handles_rect[7].x + resize_handle_size) < pixmap_width) && (resize_handles_rect[7].x >= 1)
				&& (resize_handles_rect[7].y >= 1)
				&& ((resize_handles_rect[7].y + resize_handle_size) < pixmap_height))
		{
			gdk_draw_rectangle(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(handle_gc), TRUE,
				resize_handles_rect[7].x, resize_handles_rect[7].y,
				resize_handles_rect[7].width, resize_handles_rect[7].height);
		}
	} else
	{
		// Mark the mid point handle width's as 0, to indicate they're unused
		resize_handles_rect[3].width = 0;
		resize_handles_rect[7].width = 0;
	}

	// Draw the vertical resize handles
	height = right - left;
	if (required_size_mid_point < height)
	{
		// Calculate where to place the vertical resize handles
		mid_point_horizontal = left + roundf(((gfloat) height - resize_handle_size) / 2);

		// Top Middle
		resize_handles_rect[1].x = mid_point_horizontal;
		resize_handles_rect[1].y = top + 1;
		resize_handles_rect[1].width = resize_handle_size;
		resize_handles_rect[1].height = resize_handle_size - 1;

		// Bottom middle
		resize_handles_rect[5].x = mid_point_horizontal;
		resize_handles_rect[5].y = bottom - resize_handle_size + 1;
		resize_handles_rect[5].width = resize_handle_size;
		resize_handles_rect[5].height = resize_handle_size - 1;

		// Top middle
		if (((resize_handles_rect[1].x + resize_handle_size) < pixmap_width) && (resize_handles_rect[1].x >= 1)
				&& (resize_handles_rect[1].y >= 1)
				&& ((resize_handles_rect[1].y + resize_handle_size) < pixmap_height))
		{
			gdk_draw_rectangle(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(handle_gc), TRUE,
				resize_handles_rect[1].x, resize_handles_rect[1].y,
				resize_handles_rect[1].width, resize_handles_rect[1].height);
		}

		// Bottom middle
		if (((resize_handles_rect[5].x + resize_handle_size) < pixmap_width) && (resize_handles_rect[5].x >= 1)
				&& (resize_handles_rect[5].y >= 1)
				&& ((resize_handles_rect[5].y + resize_handle_size) < pixmap_height))
		{
			gdk_draw_rectangle(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(handle_gc), TRUE,
				resize_handles_rect[5].x, resize_handles_rect[5].y,
				resize_handles_rect[5].width, resize_handles_rect[5].height);
		}
	} else
	{
		// Mark the mid point handle width's as 0, to indicate they're unused
		resize_handles_rect[1].width = 0;
		resize_handles_rect[5].width = 0;
	}

	return TRUE;
}
