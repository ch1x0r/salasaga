/*
 * $Id$
 *
 * Salasaga: Returns the offsets and dimensions of a layer at a given point in time
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


// GTK includes
#include <gtk/gtk.h>

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../cairo/render_text_string.h"
#include "../dialog/display_warning.h"


gboolean get_layer_position(GtkAllocation *position, layer *this_layer_data, gfloat time_position, gfloat *time_alpha)
{
	// Local variables
	gfloat				end_time;					// Time in seconds of the layer objects finish time
	gint				finish_x;					// X position at the layer objects finish time
	gint				finish_y;					// Y position at the layer objects finish time
	gfloat				full_end_time;				// Time in seconds when the layer finishes being fully visible
	gfloat				full_start_time;			// Time in seconds when the layer starts being fully visible
	GString				*message;					// Used to construct message strings
	gint				pixmap_height;				// Receives the height of a given pixmap
	gint				pixmap_width;				// Receives the width of a given pixmap
	gfloat				scaled_height_ratio;		// Used to calculate a vertical scaling ratio
	gfloat				scaled_width_ratio;			// Used to calculate a horizontal scaling ratio
	gfloat				start_time;					// Time in seconds of the layer objects start time
	gint				start_x;					// X position at the layer objects start time
	gint				start_y;					// Y position at the layer objects start time
	gfloat				time_offset;
	gfloat				time_diff;					// Used when calculating the object position at the desired point in time
	gfloat				x_diff;						// The X distance the object was dragged, after scaling
	gfloat				x_scale;					// Used when calculating the object position at the desired point in time
	gfloat				y_diff;						// The Y distance the object was dragged, after scaling
	gfloat				y_scale;					// Used when calculating the object position at the desired point in time


	// Initialisation
	message = g_string_new(NULL);

	// Simplify pointers
	finish_x = this_layer_data->x_offset_finish;
	finish_y = this_layer_data->y_offset_finish;
	start_time = this_layer_data->start_time;
	time_offset = time_position - start_time;
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
		// Work out the full opacity start and end times
		full_start_time = this_layer_data->start_time;
		if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
			full_start_time += this_layer_data->transition_in_duration;
		full_end_time = full_start_time + this_layer_data->duration;

		// If there's a transition in, check if the time point is during it
		if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
		{
			if (time_position < full_start_time)
			{
				// Yes.  The time position is during the transition in
				time_diff = full_start_time - start_time;
				*time_alpha = time_offset / time_diff;

				// We also reset the position to be at the start point
				position->x = start_x;
				position->y = start_y;
			}
		}

		// If there's a transition out, check if the time point is during it
		if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
		{
			// There is a transition out, so work out if the time position is during it
			if (time_position > full_end_time)
			{
				// Yes. The time position is during a transition out
				time_diff = end_time - full_end_time;
				time_offset = time_position - full_end_time;
				*time_alpha = 1 - (time_offset / time_diff);

				// We also reset the position to be at the end point
				position->x = finish_x;
				position->y = finish_y;
			}
		}

		// Check if the time position is during the layer's fully visible time
		if ((time_position >= full_start_time) && (time_position <= full_end_time))
		{
			// Yes.  The time position is during the fully visible time
			time_offset = time_position - full_start_time;
			time_diff = full_end_time - full_start_time;
			x_diff = finish_x - start_x;
			x_scale = (((gfloat) x_diff) / time_diff);
			position->x = start_x + (x_scale * time_offset);
			y_diff = finish_y - start_y;
			y_scale = (((gfloat) y_diff) / time_diff);
			position->y = start_y + (y_scale * time_offset);
			*time_alpha = 1;
		}
	} else
	{
		// Check if the desired time is before the layers visible range
		if (time_position < start_time)
		{
			// Time position is before the layers visible range
			position->x = start_x;
			position->y = start_y;
			*time_alpha = 0;
		}

		// Check if the desired time is after the layers visible range
		if (time_position > end_time)
		{
			// Time position is after the layers visible range
			position->x = finish_x;
			position->y = finish_y;
			*time_alpha = 0;
		}
	}

	// Retrieve the layer size information
	switch (this_layer_data->object_type)
	{
		case TYPE_EMPTY:
			// This is an empty layer, so reset things and return
			stored_x = -1;
			stored_y = -1;
			g_string_free(message, TRUE);
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
				g_string_free(message, TRUE);
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
			// If the text hasn't ever been rendered, we'll have to work out the size ourselves now
			if (0 == ((layer_text *) this_layer_data->object_data)->rendered_width)
			{
				// Calculate the height and width scaling values for the front pixmap
				gdk_drawable_get_size(GDK_PIXMAP(front_store), &pixmap_width, &pixmap_height);
				scaled_height_ratio = (gfloat) pixmap_height / (gfloat) project_height;
				scaled_width_ratio = (gfloat) pixmap_width / (gfloat) project_width;

				// Calculate the rendered size of the text layer
				render_text_string(NULL, (layer_text *) this_layer_data->object_data, scaled_width_ratio, scaled_height_ratio, 0, 0, *time_alpha, FALSE);
			}
			position->width = ((layer_text *) this_layer_data->object_data)->rendered_width;
			position->height = ((layer_text *) this_layer_data->object_data)->rendered_height;
			break;

		default:
			g_string_printf(message, "%s ED381: %s", _("Error"), _("Unknown layer type."));
			display_warning(message->str);
			g_string_free(message, TRUE);
			return FALSE;
	}

	// Free the memory used in this function
	g_string_free(message, TRUE);

	return TRUE;
}
