/*
 * $Id$
 *
 * Salasaga: Draw the layer information onto the display buffer
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
#include "time_line_internal_draw_layer_duration.h"
#include "time_line_internal_draw_layer_name.h"


gboolean time_line_internal_draw_layer_info(TimeLinePrivate *priv)
{
	// Local variables
	gint				loop_counter;				// Simple counter used in loops
	gint				num_layers;					// The number of layers in the select slide


	// Draw the layer names and durations
	num_layers = get_current_slide_num_layers();
	for (loop_counter = 0; loop_counter < num_layers; loop_counter++)
	{
		time_line_internal_draw_layer_name(priv, loop_counter);
		time_line_internal_draw_layer_duration(priv, loop_counter);
	}

	return TRUE;
}
