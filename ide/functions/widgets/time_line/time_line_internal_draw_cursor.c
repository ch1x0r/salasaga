/*
 * $Id$
 *
 * Salasaga: Draw the time line cursor directly on the widget
 *
 * Copyright (C) 2005-2009 Justin Clift <justin@salasaga.org>
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

// Standard include
#include <math.h>

// GTK includes
#include <gtk/gtk.h>

// Salasaga includes
#include "../../../salasaga_types.h"
#include "../../../externs.h"
#include "time_line.h"


gboolean time_line_internal_draw_cursor(GtkWidget *widget, gint pixel_num)
{
	// Local variables
	const GdkColor		colour_black = { 0, 0, 0, 0 };
	const GdkColor		colour_blue = { 0, 0, 0, 65535 };
	GdkPoint			cursor_points[3];			// Holds the corner points for the (triangular) cursor head
	TimeLinePrivate		*priv;
	static GdkGC		*this_gc = NULL;
	TimeLine			*this_time_line;


	// Safety check
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(IS_TIME_LINE(widget), FALSE);

	// Initialisation
	this_time_line = TIME_LINE(widget);
	priv = TIME_LINE_GET_PRIVATE(this_time_line);

	// Create a graphic context if we don't have one already
	if (NULL == this_gc)
	{
		this_gc = gdk_gc_new(GDK_DRAWABLE(widget->window));
	}

	// Draw the line part of the time line cursor
	gdk_gc_set_rgb_fg_color(GDK_GC(this_gc), &colour_blue);
	gdk_gc_set_line_attributes(GDK_GC(this_gc), 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_MITER);
	gdk_draw_line(GDK_DRAWABLE(widget->window), GDK_GC(this_gc), pixel_num, 0, pixel_num, widget->allocation.height);

	// Draw the top part of the time line cursor
	cursor_points[0].x = pixel_num - (CURSOR_HEAD_WIDTH / 2);
	cursor_points[0].y = CURSOR_HEAD_TOP;
	cursor_points[1].x = pixel_num + (CURSOR_HEAD_WIDTH / 2);
	cursor_points[1].y = CURSOR_HEAD_TOP;
	cursor_points[2].x = pixel_num;
	cursor_points[2].y = priv->top_border_height - 1;
	gdk_draw_polygon(GDK_DRAWABLE(widget->window), GDK_GC(this_gc), TRUE, cursor_points, 3);
	gdk_gc_set_rgb_fg_color(GDK_GC(this_gc), &colour_black);
	gdk_draw_polygon(GDK_DRAWABLE(widget->window), GDK_GC(this_gc), FALSE, cursor_points, 3);

	return TRUE;
}