/*
 * $Id$
 *
 * Salasaga: Invalidates the widget area onscreen that's occupied by a specific row
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


gboolean time_line_internal_invalidate_layer_area(GtkWidget *widget, gint layer_number)
{
	// Local variables
	GtkAllocation		layer_area;
	TimeLinePrivate		*priv;
	TimeLine			*this_time_line;


	// Initialisation
	this_time_line = TIME_LINE(widget);
	priv = TIME_LINE_GET_PRIVATE(this_time_line);

	// Set the height related variables
	layer_area.x = 0;
	layer_area.y = priv->top_border_height + (layer_number * priv->row_height) + 2;
	layer_area.height = priv->row_height - 3;
	layer_area.width = widget->allocation.width;

	// Invalidate the selected area
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &layer_area, TRUE);

	return TRUE;
}
