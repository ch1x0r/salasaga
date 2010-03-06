/*
 * $Id$
 *
 * Salasaga: Draws resize handles on a bounding box
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
	GdkRectangle		line_clip_region;			// Used as a clip mask region
	guint				mid_point_horizontal;
	guint				mid_point_vertical;
	gint				pixmap_height;				// Height of the front store
	gint				pixmap_width;				// Width of the front store
	guint				required_size_mid_point;	// Minimum size we need in order to draw mid point handles
	GtkWidget			*temp_widget;				// Temporarily holds a pointer to the main drawing area widget
	guint				width;						// Width of the bounding box


	// Initialise some things
	temp_widget = get_main_drawing_area();
	gdk_drawable_get_size(GDK_PIXMAP(front_store), &pixmap_width, &pixmap_height);

	// Draw the handles directly onto the widget
	if (NULL == handle_gc)
	{
		handle_gc = gdk_gc_new(GDK_DRAWABLE(temp_widget->window));
	}
	gdk_gc_set_function(GDK_GC(handle_gc), GDK_INVERT);

	// Set a clip mask
	line_clip_region.x = 1;
	line_clip_region.y = 1;
	line_clip_region.width = (gint) pixmap_width - 2;
	line_clip_region.height = (gint) pixmap_height - 2;
	gdk_gc_set_clip_rectangle(GDK_GC(handle_gc), &line_clip_region);


	// * Generate the list of corner resize handle positions *

	// Top left
	resize_handles_rect[0].x = left + 1;
	resize_handles_rect[0].y = top + 1;
	resize_handles_rect[0].width = get_resize_handle_size() - 1;
	resize_handles_rect[0].height = get_resize_handle_size() - 1;

	// Top right
	resize_handles_rect[2].x = right - get_resize_handle_size() + 1;
	resize_handles_rect[2].y = top + 1;
	resize_handles_rect[2].width = get_resize_handle_size() - 1;
	resize_handles_rect[2].height = get_resize_handle_size() - 1;

	// Bottom right
	resize_handles_rect[4].x = right - get_resize_handle_size() + 1;
	resize_handles_rect[4].y = bottom - get_resize_handle_size() + 1;
	resize_handles_rect[4].width = get_resize_handle_size() - 1;
	resize_handles_rect[4].height = get_resize_handle_size() - 1;

	// Bottom left
	resize_handles_rect[6].x = left + 1;
	resize_handles_rect[6].y = bottom - get_resize_handle_size() + 1;
	resize_handles_rect[6].width = get_resize_handle_size() - 1;
	resize_handles_rect[6].height = get_resize_handle_size() - 1;


	// * Draw the corner handles *

	// Top left
	gdk_draw_rectangle(GDK_DRAWABLE(temp_widget->window), GDK_GC(handle_gc), TRUE,
		resize_handles_rect[0].x, resize_handles_rect[0].y,
		resize_handles_rect[0].width, resize_handles_rect[0].height);

	// Top right
	gdk_draw_rectangle(GDK_DRAWABLE(temp_widget->window), GDK_GC(handle_gc), TRUE,
		resize_handles_rect[2].x, resize_handles_rect[2].y,
		resize_handles_rect[2].width, resize_handles_rect[2].height);

	// Bottom right
	gdk_draw_rectangle(GDK_DRAWABLE(temp_widget->window), GDK_GC(handle_gc), TRUE,
		resize_handles_rect[4].x, resize_handles_rect[4].y,
		resize_handles_rect[4].width, resize_handles_rect[4].height);

	// Bottom left
	gdk_draw_rectangle(GDK_DRAWABLE(temp_widget->window), GDK_GC(handle_gc), TRUE,
		resize_handles_rect[6].x, resize_handles_rect[6].y,
		resize_handles_rect[6].width, resize_handles_rect[6].height);

	// * If there is enough room, draw the mid point handles *
	required_size_mid_point = (get_resize_handle_size() * 3) + 2;

	// Draw the horizontal resize handles
	width = bottom - top;
	if (required_size_mid_point < width)
	{
		// Calculate where to place the horizontal resize handles
		mid_point_vertical = top + roundf(((gfloat) width - get_resize_handle_size()) / 2);

		// Right middle
		resize_handles_rect[3].x = right - get_resize_handle_size() + 1;
		resize_handles_rect[3].y = mid_point_vertical;
		resize_handles_rect[3].width = get_resize_handle_size() - 1;
		resize_handles_rect[3].height = get_resize_handle_size();

		// Left middle
		resize_handles_rect[7].x = left + 1;
		resize_handles_rect[7].y = mid_point_vertical;
		resize_handles_rect[7].width = get_resize_handle_size() - 1;
		resize_handles_rect[7].height = get_resize_handle_size();

		// Right middle
		gdk_draw_rectangle(GDK_DRAWABLE(temp_widget->window), GDK_GC(handle_gc), TRUE,
			resize_handles_rect[3].x, resize_handles_rect[3].y,
			resize_handles_rect[3].width, resize_handles_rect[3].height);

		// Left middle
		gdk_draw_rectangle(GDK_DRAWABLE(temp_widget->window), GDK_GC(handle_gc), TRUE,
			resize_handles_rect[7].x, resize_handles_rect[7].y,
			resize_handles_rect[7].width, resize_handles_rect[7].height);
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
		mid_point_horizontal = left + roundf(((gfloat) height - get_resize_handle_size()) / 2);

		// Top Middle
		resize_handles_rect[1].x = mid_point_horizontal;
		resize_handles_rect[1].y = top + 1;
		resize_handles_rect[1].width = get_resize_handle_size();
		resize_handles_rect[1].height = get_resize_handle_size() - 1;

		// Bottom middle
		resize_handles_rect[5].x = mid_point_horizontal;
		resize_handles_rect[5].y = bottom - get_resize_handle_size() + 1;
		resize_handles_rect[5].width = get_resize_handle_size();
		resize_handles_rect[5].height = get_resize_handle_size() - 1;

		// Top middle
		gdk_draw_rectangle(GDK_DRAWABLE(temp_widget->window), GDK_GC(handle_gc), TRUE,
			resize_handles_rect[1].x, resize_handles_rect[1].y,
			resize_handles_rect[1].width, resize_handles_rect[1].height);

		// Bottom middle
		gdk_draw_rectangle(GDK_DRAWABLE(temp_widget->window), GDK_GC(handle_gc), TRUE,
			resize_handles_rect[5].x, resize_handles_rect[5].y,
			resize_handles_rect[5].width, resize_handles_rect[5].height);
	} else
	{
		// Mark the mid point handle width's as 0, to indicate they're unused
		resize_handles_rect[1].width = 0;
		resize_handles_rect[5].width = 0;
	}

	return TRUE;
}
