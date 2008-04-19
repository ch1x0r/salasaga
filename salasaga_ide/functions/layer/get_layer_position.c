/*
 * $Id$
 *
 * Salasaga: Returns the offsets and dimensions of a layer at a given point in time
 *
 * Copyright (C) 2005-2008 Justin Clift <justin@salasaga.org>
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


// GTK includes
#include <gtk/gtk.h>

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../display_warning.h"


gboolean get_layer_position(GtkAllocation *position, layer *this_layer_data, gfloat time_position)
{
	// Local variables
	gfloat				end_time;					// Time in seconds of the layer objects finish time
	gint				finish_x;					// X position at the layer objects finish time
	gint				finish_y;					// Y position at the layer objects finish time 
	gfloat				start_time;					// Time in seconds of the layer objects start time
	gint				start_x;					// X position at the layer objects start time
	gint				start_y;					// Y position at the layer objects start time
	gfloat				time_offset;
	gfloat				time_diff;					// Used when calculating the object position at the desired point in time
	gfloat				x_diff;						// The X distance the object was dragged, after scaling
	gfloat				x_scale;					// Used when calculating the object position at the desired point in time
	gfloat				y_diff;						// The Y distance the object was dragged, after scaling
	gfloat				y_scale;					// Used when calculating the object position at the desired point in time


	// Simplify pointers
	finish_x = this_layer_data->x_offset_finish;
	finish_y = this_layer_data->y_offset_finish;
	start_time = this_layer_data->start_time;
	start_x = this_layer_data->x_offset_start;
	start_y = this_layer_data->y_offset_start;
	end_time = this_layer_data->start_time + this_layer_data->duration;
	if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
		end_time += this_layer_data->transition_in_duration;
	if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
		end_time += this_layer_data->transition_out_duration;

	// Calculate how far into the layer movement we are
	if ((time_position >= start_time) && (time_position <= end_time))
	{
		// The time line cursor is in the layers visible range
		time_offset = time_position - start_time;
		time_diff = end_time - start_time;
		x_diff = finish_x - start_x;
		x_scale = (((gfloat) x_diff) / time_diff);
		position->x = start_x + (x_scale * time_offset);
		y_diff = finish_y - start_y;
		y_scale = (((gfloat) y_diff) / time_diff);
		position->y = start_y + (y_scale * time_offset);
	} else
	{
		// The time line cursor is outside the layers visible range, so figure out where
		if (time_position < start_time)
		{
			// Time line cursor is before the layers visible range
			position->x = start_x;
			position->y = start_y;
		} else
		{
			// Time line cursor is after the layers visible range
			position->x = finish_x;
			position->y = finish_y;
		}
	}

	// Retrieve the layer size information
	switch (this_layer_data->object_type)
	{
		case TYPE_EMPTY:
			// This is an empty layer, so reset things and return
			stored_x = -1;
			stored_y = -1;
			return FALSE;

		case TYPE_HIGHLIGHT:
			position->width = ((layer_highlight *) this_layer_data->object_data)->width;
			position->height = ((layer_highlight *) this_layer_data->object_data)->height;
			break;

		case TYPE_GDK_PIXBUF:
			// If this is the background layer, then we ignore it
			if (TRUE == this_layer_data->background)
			{
				stored_x = -1;
				stored_y = -1;
				return FALSE;
			}

			// No it's not, so process it
			position->width = ((layer_image *) this_layer_data->object_data)->width;
			position->height = ((layer_image *) this_layer_data->object_data)->height;
			break;

		case TYPE_MOUSE_CURSOR:
			position->width = ((layer_mouse *) this_layer_data->object_data)->width;
			position->height = ((layer_mouse *) this_layer_data->object_data)->height;
			break;

		case TYPE_TEXT:
			position->width = ((layer_text *) this_layer_data->object_data)->rendered_width;
			position->height = ((layer_text *) this_layer_data->object_data)->rendered_height;
			break;

		default:
			display_warning("Error ED381: Unknown layer type");

			return FALSE;
	}

	return TRUE;
}
