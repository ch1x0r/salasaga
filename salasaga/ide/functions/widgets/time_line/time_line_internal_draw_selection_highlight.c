/*
 * $Id$
 *
 * Salasaga: Highlight the layer in a specific row
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

// Standard include
#include <math.h>

// GTK includes
#include <gtk/gtk.h>

// Salasaga includes
#include "../../../salasaga_types.h"
#include "../../../externs.h"
#include "time_line.h"


void time_line_internal_draw_selection_highlight(TimeLinePrivate *priv, gint width)
{
	// Local variables
	const GdkColor		colour_red = {0, 65535, 0, 0 };
	gint8				dash_list[2] = { 3, 3 };
	static GdkGC		*display_buffer_gc = NULL;
	gint				selected_row;
	gint				x1, y1, x2, y2;


	// Create a graphic context for the display buffer image if we don't have one already
	if (NULL == display_buffer_gc)
	{
		display_buffer_gc = gdk_gc_new(GDK_DRAWABLE(priv->display_buffer));
	}

	// Draw the selection
	selected_row = priv->selected_layer_num;
	x1 = 0;
	y1 = priv->top_border_height + 1 + (selected_row * priv->row_height);
	x2 = width - 1;
	y2 = priv->row_height - 2;
	gdk_gc_set_rgb_fg_color(GDK_GC(display_buffer_gc), &colour_red);
	gdk_gc_set_line_attributes(GDK_GC(display_buffer_gc), 1, GDK_LINE_ON_OFF_DASH, GDK_CAP_BUTT, GDK_JOIN_MITER);
	gdk_gc_set_dashes(GDK_GC(display_buffer_gc), 1, dash_list, 2);
	gdk_draw_rectangle(GDK_DRAWABLE(priv->display_buffer), GDK_GC(display_buffer_gc), FALSE, x1, y1, x2, y2);
}
