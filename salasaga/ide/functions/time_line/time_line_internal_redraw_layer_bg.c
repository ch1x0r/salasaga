/*
 * $Id$
 *
 * Salasaga: Refresh the area of the display buffer covered by a layer, from the cached background image
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
#include "time_line_internal_redraw_bg_area.h"


gboolean time_line_internal_redraw_layer_bg(TimeLinePrivate *priv, gint layer_number)
{
	// Local variables
	gint				layer_height;
	gint				layer_width;
	gint				layer_x;
	gint				layer_y;


	// Set the height related variables
	layer_x = 0;
	layer_y = priv->top_border_height + (layer_number * priv->row_height) + 2;
	layer_height = priv->row_height - 3;
	layer_width = -1;

	// Refresh the display buffer for the selected layer
	time_line_internal_redraw_bg_area(priv, layer_x, layer_y, layer_width, layer_height);

	return TRUE;
}
