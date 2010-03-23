/*
 * $Id$
 *
 * Salasaga: Drawing the time line widget onscreen
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
#include "time_line_get_cursor_position.h"
#include "time_line_get_left_border_width.h"
#include "time_line_get_type.h"
#include "time_line_internal_draw_cursor.h"
#include "time_line_internal_draw_layer_info.h"
#include "time_line_internal_draw_selection_highlight.h"
#include "time_line_internal_initialise_display_buffer.h"


gint time_line_expose(GtkWidget *widget, GdkEventExpose *event)
{
	// Local variables
	gint				height;
	gint				new_cursor_pos;
	TimeLinePrivate		*priv;
	static GdkGC		*this_gc = NULL;
	TimeLine			*this_time_line;
	gint				width;


	// Safety check
	g_return_val_if_fail(widget != NULL || event != NULL, FALSE);
	g_return_val_if_fail(IS_TIME_LINE(widget), FALSE);

	// Initialisation
	this_time_line = TIME_LINE(widget);
	priv = TIME_LINE_GET_PRIVATE(this_time_line);

	// Ensure we have at least the minimum required widget size
	if (WIDGET_MINIMUM_HEIGHT > GTK_WIDGET(widget)->allocation.height)
	{
		height = WIDGET_MINIMUM_HEIGHT;
	} else
	{
		height = GTK_WIDGET(widget)->allocation.height;
	}
	if (WIDGET_MINIMUM_WIDTH > GTK_WIDGET(widget)->allocation.width)
	{
		width = WIDGET_MINIMUM_WIDTH;
	} else
	{
		width = GTK_WIDGET(widget)->allocation.width;
	}

	// Ensure we have a display buffer to refresh from
	if (NULL == priv->display_buffer)
	{
		// Create the display buffer
		time_line_internal_initialise_display_buffer(priv, width, height);

		// Draw the layer information
		time_line_internal_draw_layer_info(priv);

		// Highlight the selected row
		time_line_internal_draw_selection_highlight(priv, width);
	}

	// Create a graphic context if we don't have one already
	if (NULL == this_gc)
	{
		this_gc = gdk_gc_new(GDK_DRAWABLE(widget->window));
	}

	// Refresh the invalidated area from the local cached version
	gdk_draw_drawable(GDK_DRAWABLE(widget->window), GDK_GC(this_gc),
		GDK_PIXMAP(priv->display_buffer),
		event->area.x, event->area.y,
		event->area.x, event->area.y,
		event->area.width, event->area.height);

	// Draw the time line cursor
	new_cursor_pos = round(time_line_get_cursor_position(widget) * time_line_get_pixels_per_second()) + time_line_get_left_border_width(priv);
	time_line_internal_draw_cursor(widget, new_cursor_pos);

	return TRUE;
}
