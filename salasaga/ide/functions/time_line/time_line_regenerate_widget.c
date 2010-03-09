/*
 * $Id$
 *
 * Salasaga: Redraw the images in an already existing timeline widget
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
#include "time_line_internal_draw_layer_info.h"
#include "time_line_internal_draw_selection_highlight.h"
#include "time_line_internal_initialise_bg_image.h"
#include "time_line_internal_initialise_display_buffer.h"


gboolean time_line_regenerate_widget(GtkWidget *widget)
{
	// Local variables
	gint				height;
	TimeLinePrivate		*priv;
	TimeLine			*this_time_line;
	gint				width;


	// Safety check
	if (NULL == widget)
	{
		return TRUE;
	}
	if (FALSE == IS_TIME_LINE(widget))
	{
		return TRUE;
	}

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

	// If the cached background image isn't valid, we need to recreate it
	if (FALSE == priv->cached_bg_valid)
	{
		time_line_internal_initialise_bg_image(priv, width, height);
	}

	// Recreate the display buffer
	time_line_internal_initialise_display_buffer(priv, width, height);

	// Draw the layer information
	time_line_internal_draw_layer_info(priv);

	// Highlight the selected row
	time_line_internal_draw_selection_highlight(priv, width);

	return TRUE;
}
