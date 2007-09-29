/*
 * $Id$
 *
 * Flame Project: Function to draw a bounding box in the drawing area, via the backing store 
 * 
 * Copyright (C) 2007 Justin Clift <justin@postgresql.org>
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

// Standard includes
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

// GTK includes
#include <glib/gstdio.h>
#include <gtk/gtk.h>

// Gnome includes
#include <libgnome/gnome-url.h>

// XML includes
#include <libxml/xmlsave.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
	#include "flame-keycapture.h"
#endif

// Flame Edit includes
#include "../flame-types.h"
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
	gdk_draw_lines(backing_store, widget->style->black_gc, &point_array[0], 5);

	// Cause an expose event to happen, writing the (updated) backing store to the screen
	gdk_window_invalidate_rect(widget->window, tmp_rectangle_array[0], FALSE);

	// Free the memory allocated in this function
	g_free(tmp_rectangle_array[0]);
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.1  2007/09/29 04:22:16  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */
