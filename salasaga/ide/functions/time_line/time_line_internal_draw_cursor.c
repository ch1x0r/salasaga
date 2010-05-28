/*
 * $Id$
 *
 * Salasaga: Draw the time line cursor directly on the widget
 *
 * Copyright (C) 2005-2010 Digital Distribution Global Training Solutions Pty. Ltd.
 * <justin@salasaga.org>
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
#include "../../salasaga_types.h"
#include "../global_functions.h"
#include "time_line.h"
#include "time_line_get_type.h"


gboolean time_line_internal_draw_cursor(GtkWidget *widget, gint pixel_num)
{
	// Local variables
	const GdkColor		colour_black = { 0, 0, 0, 0 };
	const GdkColor		colour_blue = { 0, 0, 0, 65535 };
	GdkPoint			cursor_points[3];			// Holds the corner points for the (triangular) cursor head
	TimeLinePrivate		*priv;
	static GdkGC		*this_gc_top_right = NULL;
	static GdkGC		*this_gc_bot_right = NULL;
	TimeLine			*this_time_line;
	guint				height;

	// Safety check
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(IS_TIME_LINE(widget), FALSE);

	// Initialisation
	this_time_line = TIME_LINE(widget);
	priv = TIME_LINE_GET_PRIVATE(this_time_line);

	// Create a graphic context if we don't have one already
	if (NULL == this_gc_bot_right)
	{
		this_gc_bot_right = gdk_gc_new(GDK_DRAWABLE(priv->display_buffer_bot_right));
	}
	if (NULL == this_gc_top_right)
	{
		this_gc_top_right = gdk_gc_new(GDK_DRAWABLE(priv->display_buffer_top_right));
	}
	height = get_current_slide_num_layers()*priv->row_height + 10;
	if(height<widget->allocation.height)
		height = widget->allocation.height;
	// Draw the line part of the time line cursor
	gdk_gc_set_rgb_fg_color(GDK_GC(this_gc_bot_right), &colour_blue);
	gdk_gc_set_line_attributes(GDK_GC(this_gc_bot_right), 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_MITER);
	gdk_draw_line(GDK_DRAWABLE(priv->display_buffer_bot_right), GDK_GC(this_gc_bot_right), pixel_num, 0, pixel_num, height);

	// Draw the top part of the time line cursor
	cursor_points[0].x = pixel_num - (CURSOR_HEAD_WIDTH / 2);
	cursor_points[0].y = CURSOR_HEAD_TOP;
	cursor_points[1].x = pixel_num + (CURSOR_HEAD_WIDTH / 2);
	cursor_points[1].y = CURSOR_HEAD_TOP;
	cursor_points[2].x = pixel_num;
	cursor_points[2].y = priv->top_border_height - 1;
	gdk_draw_polygon(GDK_DRAWABLE(priv->display_buffer_top_right), GDK_GC(this_gc_top_right), TRUE, cursor_points, 3);
	gdk_gc_set_rgb_fg_color(GDK_GC(this_gc_top_right), &colour_black);
	gdk_draw_polygon(GDK_DRAWABLE(priv->display_buffer_top_right), GDK_GC(this_gc_top_right), FALSE, cursor_points, 3);

	return TRUE;
}
