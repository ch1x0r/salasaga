/*
 * $Id$
 *
 * Salasaga:
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
#include "time_line_get_type.h"
#include "time_line_internal_draw_layer_info.h"
#include "time_line_internal_draw_selection_highlight.h"
#include "time_line_internal_initialise_bg_image.h"
#include "time_line_internal_initialise_display_buffer.h"


void time_line_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
	// Local variables
	gint				height;
	TimeLinePrivate		*priv;
	TimeLine			*this_time_line;
	gint				width;


	// Safety check
	g_return_if_fail(allocation != NULL || widget != NULL);
	g_return_if_fail(TIME_LINE(widget));

	// Initialisation
	this_time_line = TIME_LINE(widget);
	priv = TIME_LINE_GET_PRIVATE(this_time_line);

	// Ensure we have at least the minimum required widget size
	if (WIDGET_MINIMUM_HEIGHT > allocation->height)
	{
		height = WIDGET_MINIMUM_HEIGHT;
	} else
	{
		height = allocation->height;
	}
	if (WIDGET_MINIMUM_WIDTH > allocation->width)
	{
		width = WIDGET_MINIMUM_WIDTH;
	} else
	{
		width = allocation->width;
	}

	// Create the background buffer
	time_line_internal_initialise_bg_image(priv, width, height);

	// Create the display buffer
	time_line_internal_initialise_display_buffer(priv, width, height);

	// Draw the layer information
	time_line_internal_draw_layer_info(priv);

	// Highlight the selected row
	time_line_internal_draw_selection_highlight(priv, width);

	// Set the widget position and size
	widget->allocation = *allocation;
	if (GTK_WIDGET_REALIZED(widget))
	{
		gdk_window_move_resize(widget->window, allocation->x, allocation->y, width, height);
	}
}
