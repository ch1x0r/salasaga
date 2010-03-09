/*
 * $Id$
 *
 * Salasaga: Draw a horizontal guide line directly on the time line widget
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
#include "../../salasaga_types.h"
#include "../global_functions.h"
#include "time_line.h"
#include "time_line_get_type.h"


gboolean time_line_internal_draw_guide_line(GtkWidget *widget, gint pixel_num)
{
	// Local variables
	const GdkColor		colour_black = {0, 0, 0, 0 };
	static GdkColormap	*colourmap = NULL;			// Colormap used for drawing
	static gint8		dash_list[2] = { 3, 3 };
	static GdkGC		*widget_gc = NULL;
	GtkAllocation		guide_area;					// The area we need to invalidate
	gint				pixmap_height;				// Height of the pixmap in pixels
	TimeLinePrivate		*priv;
	TimeLine			*this_time_line;


	// Safety check
	if (NULL == widget)
		return FALSE;
	if (FALSE == IS_TIME_LINE(widget))
		return FALSE;

	// Initialisation
	this_time_line = TIME_LINE(widget);
	priv = TIME_LINE_GET_PRIVATE(this_time_line);
	if (NULL == colourmap)
	{
		colourmap = gdk_colormap_get_system();
		gdk_drawable_set_colormap(GDK_DRAWABLE(widget->window), GDK_COLORMAP(colourmap));
	}
	if (NULL == widget_gc)
	{
		widget_gc = gdk_gc_new(GDK_DRAWABLE(widget->window));
	}
	gdk_drawable_get_size(GDK_DRAWABLE(widget->window), NULL, &pixmap_height);

	// Set the height related variables
	guide_area.x = pixel_num;
	guide_area.y = 0;
	guide_area.height = pixmap_height;
	guide_area.width = 1;

	// Draw guide line
	gdk_gc_set_rgb_fg_color(GDK_GC(widget_gc), &colour_black);
	gdk_gc_set_line_attributes(GDK_GC(widget_gc), 1, GDK_LINE_ON_OFF_DASH, GDK_CAP_BUTT, GDK_JOIN_MITER);
	gdk_gc_set_dashes(GDK_GC(widget_gc), 1, dash_list, 2);
	gdk_draw_line(GDK_DRAWABLE(widget->window), GDK_GC(widget_gc),
			guide_area.x,
			priv->top_border_height + 1,
			guide_area.x,
			guide_area.height);

	return TRUE;
}
