/*
 * $Id$
 *
 * Salasaga: Draws resize handles on a bounding box
 * 
 * Copyright (C) 2008 Justin Clift <justin@salasaga.org>
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

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"

gboolean draw_resize_handles(gint left, gint top, gint right, gint bottom, guint handle_size)
{
	// Local variables
	static GdkGC		*handle_gc = NULL;
	guint				height;						// Height of the bounding box
	guint				mid_point_horizontal;
	guint				mid_point_vertical;
	guint				required_size_mid_point;	// Minimum size we need in order to draw mid point handles
	guint				width;						// Width of the bounding box


	// Draw the handles directly onto the widget
	if (NULL == handle_gc)
	{
		handle_gc = gdk_gc_new(GDK_DRAWABLE(main_drawing_area->window));
	}
	gdk_gc_set_function(GDK_GC(handle_gc), GDK_INVERT);

	// * Draw the corner handles *

	// Top left
	gdk_draw_rectangle(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(handle_gc), TRUE, left + 1, top + 1, handle_size - 1, handle_size - 1);

	// Top right
	gdk_draw_rectangle(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(handle_gc), TRUE, right - handle_size + 1, top + 1, handle_size - 1, handle_size - 1);

	// Bottom left
	gdk_draw_rectangle(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(handle_gc), TRUE, left + 1, bottom - handle_size + 1, handle_size - 1, handle_size - 1);

	// Bottom right
	gdk_draw_rectangle(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(handle_gc), TRUE, right - handle_size + 1, bottom - handle_size + 1, handle_size - 1, handle_size - 1);

	// * If there is enough room, draw the mid point handles *
	required_size_mid_point = (handle_size * 3) + 2;

	// Draw the vertical mid point handles
	width = bottom - top;
	if (required_size_mid_point < width)
	{
		// Calculate where to place the vertical mid point handles
		mid_point_vertical = top + roundf(((gfloat) width - handle_size) / 2);
	
		// Left
		gdk_draw_rectangle(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(handle_gc), TRUE, left + 1, mid_point_vertical, handle_size - 1, handle_size);

		// Right
		gdk_draw_rectangle(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(handle_gc), TRUE, right - handle_size + 1, mid_point_vertical, handle_size - 1, handle_size);
	}

	// Draw the horizontal mid point handles
	height = right - left;
	if (required_size_mid_point < height)
	{
		// Calculate where to place the horizontal mid point handles
		mid_point_horizontal = left + roundf(((gfloat) height - handle_size) / 2);

		// Top
		gdk_draw_rectangle(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(handle_gc), TRUE, mid_point_horizontal, top + 1, handle_size, handle_size - 1);

		// Bottom
		gdk_draw_rectangle(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(handle_gc), TRUE, mid_point_horizontal, bottom - handle_size + 1, handle_size, handle_size - 1);
	}

	return TRUE;
}
