/*
 * $Id$
 *
 * Salasaga: Changes the presently selected time line layer to the given one
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
#include "time_line_get_type.h"
#include "time_line_internal_draw_selection_highlight.h"
#include "time_line_internal_redraw_bg_area.h"


// fixme3: Should this be done as a property instead?
gboolean time_line_set_selected_layer_num(GtkWidget *widget, gint selected_row)
{
	// Local variables
	gint				height;
	GtkAllocation		new_allocation;
	GtkAllocation		old_allocation;
	TimeLinePrivate		*priv;
	TimeLine			*this_time_line;
	gint				width;
	gint				x1;
	gint				x2;
	gint				y1;
	gint				y2;


	// Safety check
	g_return_val_if_fail(widget != NULL, -1);
	g_return_val_if_fail(IS_TIME_LINE(widget), -1);

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

	// * Restore the background underneath the existing selection box *

	// Calculate the corner points
	x1 = 0;
	y1 = priv->top_border_height + 1 + (priv->selected_layer_num * priv->row_height);
	x2 = width;
	y2 = priv->row_height - 2;

	// Restore top line segment
	old_allocation.x = x1;
	old_allocation.y = y1;
	old_allocation.width = x2;
	old_allocation.height = 1;
	time_line_internal_redraw_bg_area(priv, old_allocation.x, old_allocation.y, old_allocation.width, old_allocation.height);
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &old_allocation, TRUE);

	// Restore bottom line segment
	old_allocation.y = y1 + y2;
	time_line_internal_redraw_bg_area(priv, old_allocation.x, old_allocation.y, old_allocation.width, old_allocation.height);
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &old_allocation, TRUE);

	// Restore left line segment
	old_allocation.y = y1;
	old_allocation.width = 1;
	old_allocation.height = y2;
	time_line_internal_redraw_bg_area(priv, old_allocation.x, old_allocation.y, old_allocation.width, old_allocation.height);
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &old_allocation, TRUE);

	// Restore right line segment
	old_allocation.x = width - 1;
	time_line_internal_redraw_bg_area(priv, old_allocation.x, old_allocation.y, old_allocation.width, old_allocation.height);
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &old_allocation, TRUE);

	// Set the internal variable, as requested
	priv->selected_layer_num = selected_row;

	// Draw a selection box around the newly selected layer
	time_line_internal_draw_selection_highlight(priv, width);

	// * Refresh the widget where the new lines were just drawn  *

	// Update the corner points
	y1 = priv->top_border_height + 1 + (selected_row * priv->row_height);

	// Refresh top line segment
	new_allocation.x = x1;
	new_allocation.y = y1;
	new_allocation.width = x2;
	new_allocation.height = 1;
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &new_allocation, TRUE);

	// Refresh bottom line segment
	new_allocation.y = y1 + y2;
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &new_allocation, TRUE);

	// Refresh left line segment
	new_allocation.y = y1;
	new_allocation.width = 1;
	new_allocation.height = y2;
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &new_allocation, TRUE);

	// Refresh right line segment
	new_allocation.x = width - 1;
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &new_allocation, TRUE);

	return TRUE;
}
