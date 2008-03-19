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


void draw_bounding_box(GtkWidget *widget, GdkRegion *region)
{
	// Local variables
	GdkPoint			point_array[5];				// Holds the boundary points of the lines to draw

	GdkRectangle			*tmp_rectangle_array[1];
	gint				tmp_gint;
	

	gdk_region_offset(region, 1, 2);
	gdk_region_get_rectangles(region, tmp_rectangle_array, &tmp_gint);

//g_printerr("Number of rectangles in region: %d\n", tmp_gint);	
//g_printerr("Rectangle X: %d\n", tmp_rectangle_array[0]->x);
//g_printerr("Rectangle Y: %d\n", tmp_rectangle_array[0]->y);
//g_printerr("Rectangle Width: %d\n", tmp_rectangle_array[0]->width);
//g_printerr("Rectangle Height: %d\n", tmp_rectangle_array[0]->height);

	// Fill in the values for the point array
	point_array[0].x = tmp_rectangle_array[0]->x;
	point_array[0].y = tmp_rectangle_array[0]->y;

	point_array[1].x = tmp_rectangle_array[0]->x + tmp_rectangle_array[0]->width - 1;
	point_array[1].y = tmp_rectangle_array[0]->y;

	point_array[2].x = tmp_rectangle_array[0]->x + tmp_rectangle_array[0]->width - 1;
	point_array[2].y = tmp_rectangle_array[0]->y + tmp_rectangle_array[0]->height - 1;

	point_array[3].x = tmp_rectangle_array[0]->x;
	point_array[3].y = tmp_rectangle_array[0]->y + tmp_rectangle_array[0]->height - 1;

	point_array[4].x = tmp_rectangle_array[0]->x;
	point_array[4].y = tmp_rectangle_array[0]->y;

//draw_workspace();

	// Draw the bounding line around the object
	gdk_draw_lines(front_store, widget->style->black_gc, &point_array[0], 5);

	// Cause an expose event to happen, writing the (updated) front store to the screen
	gdk_window_invalidate_rect(widget->window, tmp_rectangle_array[0], FALSE);

	// Free the memory allocated in this function
	g_free(tmp_rectangle_array[0]);
}
