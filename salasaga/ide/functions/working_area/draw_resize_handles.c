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
#include "../global_functions.h"


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
	gdk_drawable_get_size(GDK_PIXMAP(get_front_store()), &pixmap_width, &pixmap_height);

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
	set_resize_handles_rect_x(0, left + 1);
	set_resize_handles_rect_y(0, top + 1);
	set_resize_handles_rect_width(0, get_resize_handle_size() - 1);
	set_resize_handles_rect_height(0, get_resize_handle_size() - 1);

	// Top right
	set_resize_handles_rect_x(2, right - get_resize_handle_size() + 1);
	set_resize_handles_rect_y(2, top + 1);
	set_resize_handles_rect_width(2, get_resize_handle_size() - 1);
	set_resize_handles_rect_height(2, get_resize_handle_size() - 1);

	// Bottom right
	set_resize_handles_rect_x(4, right - get_resize_handle_size() + 1);
	set_resize_handles_rect_y(4, bottom - get_resize_handle_size() + 1);
	set_resize_handles_rect_width(4, get_resize_handle_size() - 1);
	set_resize_handles_rect_height(4, get_resize_handle_size() - 1);

	// Bottom left
	set_resize_handles_rect_x(6, left + 1);
	set_resize_handles_rect_y(6, bottom - get_resize_handle_size() + 1);
	set_resize_handles_rect_width(6, get_resize_handle_size() - 1);
	set_resize_handles_rect_height(6, get_resize_handle_size() - 1);


	// * Draw the corner handles *

	// Top left
	gdk_draw_rectangle(GDK_DRAWABLE(temp_widget->window), GDK_GC(handle_gc), TRUE,
		get_resize_handles_rect_x(0), get_resize_handles_rect_y(0),
		get_resize_handles_rect_width(0), get_resize_handles_rect_height(0));

	// Top right
	gdk_draw_rectangle(GDK_DRAWABLE(temp_widget->window), GDK_GC(handle_gc), TRUE,
		get_resize_handles_rect_x(2), get_resize_handles_rect_y(2),
		get_resize_handles_rect_width(2), get_resize_handles_rect_height(2));

	// Bottom right
	gdk_draw_rectangle(GDK_DRAWABLE(temp_widget->window), GDK_GC(handle_gc), TRUE,
		get_resize_handles_rect_x(4), get_resize_handles_rect_y(4),
		get_resize_handles_rect_width(4), get_resize_handles_rect_height(4));

	// Bottom left
	gdk_draw_rectangle(GDK_DRAWABLE(temp_widget->window), GDK_GC(handle_gc), TRUE,
		get_resize_handles_rect_x(6), get_resize_handles_rect_y(6),
		get_resize_handles_rect_width(6), get_resize_handles_rect_height(6));

	// * If there is enough room, draw the mid point handles *
	required_size_mid_point = (get_resize_handle_size() * 3) + 2;

	// Draw the horizontal resize handles
	width = bottom - top;
	if (required_size_mid_point < width)
	{
		// Calculate where to place the horizontal resize handles
		mid_point_vertical = top + roundf(((gfloat) width - get_resize_handle_size()) / 2);

		// Right middle
		set_resize_handles_rect_x(3, right - get_resize_handle_size() + 1);
		set_resize_handles_rect_y(3, mid_point_vertical);
		set_resize_handles_rect_width(3, get_resize_handle_size() - 1);
		set_resize_handles_rect_height(3, get_resize_handle_size());

		// Left middle
		set_resize_handles_rect_x(7, left + 1);
		set_resize_handles_rect_y(7, mid_point_vertical);
		set_resize_handles_rect_width(7, get_resize_handle_size() - 1);
		set_resize_handles_rect_height(7, get_resize_handle_size());

		// Right middle
		gdk_draw_rectangle(GDK_DRAWABLE(temp_widget->window), GDK_GC(handle_gc), TRUE,
			get_resize_handles_rect_x(3), get_resize_handles_rect_y(3),
			get_resize_handles_rect_width(3), get_resize_handles_rect_height(3));

		// Left middle
		gdk_draw_rectangle(GDK_DRAWABLE(temp_widget->window), GDK_GC(handle_gc), TRUE,
			get_resize_handles_rect_x(7), get_resize_handles_rect_y(7),
			get_resize_handles_rect_width(7), get_resize_handles_rect_height(7));
	} else
	{
		// Mark the mid point handle width's as 0, to indicate they're unused
		set_resize_handles_rect_width(3, 0);
		set_resize_handles_rect_width(7, 0);
	}

	// Draw the vertical resize handles
	height = right - left;
	if (required_size_mid_point < height)
	{
		// Calculate where to place the vertical resize handles
		mid_point_horizontal = left + roundf(((gfloat) height - get_resize_handle_size()) / 2);

		// Top Middle
		set_resize_handles_rect_x(1, mid_point_horizontal);
		set_resize_handles_rect_y(1, top + 1);
		set_resize_handles_rect_width(1, get_resize_handle_size());
		set_resize_handles_rect_height(1, get_resize_handle_size() - 1);

		// Bottom middle
		set_resize_handles_rect_x(5, mid_point_horizontal);
		set_resize_handles_rect_y(5, bottom - get_resize_handle_size() + 1);
		set_resize_handles_rect_width(5, get_resize_handle_size());
		set_resize_handles_rect_height(5, get_resize_handle_size() - 1);

		// Top middle
		gdk_draw_rectangle(GDK_DRAWABLE(temp_widget->window), GDK_GC(handle_gc), TRUE,
			get_resize_handles_rect_x(1), get_resize_handles_rect_y(1),
			get_resize_handles_rect_width(1), get_resize_handles_rect_height(1));

		// Bottom middle
		gdk_draw_rectangle(GDK_DRAWABLE(temp_widget->window), GDK_GC(handle_gc), TRUE,
			get_resize_handles_rect_x(5), get_resize_handles_rect_y(5),
			get_resize_handles_rect_width(5), get_resize_handles_rect_height(5));
	} else
	{
		// Mark the mid point handle width's as 0, to indicate they're unused
		set_resize_handles_rect_width(1, 0);
		set_resize_handles_rect_width(5, 0);
	}

	return TRUE;
}
