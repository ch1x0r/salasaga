/*
 * $Id$
 *
 * Salasaga: Draws a handle box (outline) on the workspace, around the selected layer 
 * 
 * Copyright (C) 2008 Justin Clift <justin@salasaga.org>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 */


// Standard includes
#include <stdlib.h>

// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"
#include "display_warning.h"
#include "draw_bounding_box.h"
#include "draw_resize_handles.h"
#include "cairo/calculate_text_object_size.h"
#include "widgets/time_line.h"


gboolean draw_handle_box(void)
{
	// Local variables
	slide				*current_slide_data;		// Alias to make things easier
	gfloat				end_time;					// Time in seconds of the layer objects finish time
	gint				finish_x;					// X position at the layer objects finish time
	gint				finish_y;					// Y position at the layer objects finish time 
	gint				height;
	gint				onscreen_bottom;			// Y coordinate of bounding box bottom
	gint				onscreen_left;				// X coordinate of bounding box left
	gint				onscreen_right;				// X coordinate of bounding box right
	gint				onscreen_top;				// Y coordinate of bounding box top
	guint				required_size_for_handles;	// Minimum size we need in order to draw any resize handles
	gfloat				scaled_height_ratio;		// Used to calculate a vertical scaling ratio 
	gfloat				scaled_width_ratio;			// Used to calculate a horizontal scaling ratio
	gint				selected_layer;				// Holds the number of the layer that is selected
	gfloat				start_time;					// Time in seconds of the layer objects start time
	gint				start_x;					// X position at the layer objects start time
	gint				start_y;					// Y position at the layer objects start time
	layer				*this_layer_data;
	gfloat				time_offset;
	gfloat 				time_position;				// The point in time we need the handle box for
	gint				time_x;						// Unscaled X position of the layer at our desired point in time
	gint				time_y;						// Unscaled Y position of the layer at our desired point in time
	gfloat				time_diff;					// Used when calculating the object position at the desired point in time
	gint				width;
	gint				x_diff;						// Used when calculating the object position at the desired point in time
	gfloat				x_scale;					// Used when calculating the object position at the desired point in time
	gint				y_diff;						// Used when calculating the object position at the desired point in time
	gfloat				y_scale;					// Used when calculating the object position at the desired point in time



	// Only do this function if we have a front store available and a project loaded
	if ((NULL == front_store) || (FALSE == project_active))
	{
		return TRUE;
	}

	// Initialise some things
	current_slide_data = current_slide->data;

	// Determine which layer the user has selected in the timeline
	selected_layer = time_line_get_selected_layer_num(current_slide_data->timeline_widget);
	if (0 > selected_layer)
	{
		// If we're received an invalid layer number, we can't proceed
		return TRUE;
	}

	// Get the current time line cursor position
	time_position = time_line_get_cursor_position(current_slide_data->timeline_widget);

	// Simplify pointers
	current_slide_data->layers = g_list_first(current_slide_data->layers);
	this_layer_data = g_list_nth_data(current_slide_data->layers, selected_layer);
	finish_x = this_layer_data->x_offset_finish;
	finish_y = this_layer_data->y_offset_finish;
	start_time = this_layer_data->start_time;
	start_x = this_layer_data->x_offset_start;
	start_y = this_layer_data->y_offset_start;

	// Calculate the position of the handle box
	end_time = this_layer_data->start_time + this_layer_data->duration;
	if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
		end_time += this_layer_data->transition_in_duration;
	if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
		end_time += this_layer_data->transition_out_duration;
	if ((time_position >= start_time) && (time_position <= end_time))
	{
		// Calculate how far into the layer movement we are
		time_offset = time_position - start_time;
		time_diff = end_time - start_time;
		x_diff = finish_x - start_x;
		x_scale = (((gfloat) x_diff) / time_diff);
		time_x = start_x + (x_scale * time_offset);
		y_diff = finish_y - start_y;
		y_scale = (((gfloat) y_diff) / time_diff);
		time_y = start_y + (y_scale * time_offset);
	} else
	{
		if (time_position < start_time)
		{
			// The desired point in time is before the layer is visible, so we use the object start position
			time_x = start_x;
			time_y = start_y;
		} else
		{
			// The desired point in time after layer is visible, so we use the object finish position
			time_x = finish_x;
			time_y = finish_y;
		}
	}

	// Retrieve the dimensions of the selected object
	switch (this_layer_data->object_type)
	{
		case TYPE_EMPTY:
			// This is an empty layer, so clear any existing handle box
			gdk_draw_drawable(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(main_drawing_area->style->fg_gc[GTK_WIDGET_STATE(main_drawing_area)]),
					GDK_PIXMAP(front_store), 0, 0, 0, 0, -1, -1);
			return TRUE;

		case TYPE_HIGHLIGHT:
			width = ((layer_highlight *) this_layer_data->object_data)->width;
			height = ((layer_highlight *) this_layer_data->object_data)->height;
			break;

		case TYPE_GDK_PIXBUF:
			// If this is the background layer, then we clear any existing handle box
			if (TRUE == this_layer_data->background)
			{
				// Clear any existing handle box
				gdk_draw_drawable(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(main_drawing_area->style->fg_gc[GTK_WIDGET_STATE(main_drawing_area)]),
						GDK_PIXMAP(front_store), 0, 0, 0, 0, -1, -1);
				return TRUE;
			}

			// No it's not, so process it
			width = ((layer_image *) this_layer_data->object_data)->width;
			height = ((layer_image *) this_layer_data->object_data)->height;
			break;

		case TYPE_MOUSE_CURSOR:
			width = ((layer_mouse *) this_layer_data->object_data)->width;
			height = ((layer_mouse *) this_layer_data->object_data)->height;
			break;

		case TYPE_TEXT:
			// If the text hasn't ever been rendered, we'll have to work out the size ourselves now
			if (0 == ((layer_text *) this_layer_data->object_data)->rendered_width)
			{
				calculate_text_object_size((layer_text *) this_layer_data->object_data);
			}
			width = ((layer_text *) this_layer_data->object_data)->rendered_width;
			height = ((layer_text *) this_layer_data->object_data)->rendered_height;
			break;

		default:
			// Unknown layer type, so no idea how to extract the needed data for the next code
			display_warning("Error ED285: Unknown layer type");
			return TRUE;
	}

	// Calculate the height and width scaling values for the main drawing area at its present size
	scaled_height_ratio = (gfloat) project_height / (gfloat) main_drawing_area->allocation.height;
	scaled_width_ratio = (gfloat) project_width / (gfloat) main_drawing_area->allocation.width;

	// Work out the bounding box boundaries
	onscreen_left = (time_x + 1) / scaled_width_ratio;
	onscreen_top = (time_y + 1) / scaled_height_ratio;
	onscreen_right = (time_x + width) / scaled_width_ratio;
	onscreen_bottom = (time_y + height) / scaled_height_ratio;

	// Ensure the bounding box doesn't go out of bounds
	onscreen_left = CLAMP(onscreen_left, 2, main_drawing_area->allocation.width - (width / scaled_width_ratio) - 2);
	onscreen_top = CLAMP(onscreen_top, 2, main_drawing_area->allocation.height - (height / scaled_height_ratio) - 2);
	onscreen_right = CLAMP(onscreen_right, 2 + (width / scaled_width_ratio), main_drawing_area->allocation.width - 2);
	onscreen_bottom = CLAMP(onscreen_bottom, 2 + (height / scaled_height_ratio), main_drawing_area->allocation.height - 2);

	// Draw a bounding box onscreen
	draw_bounding_box(onscreen_left, onscreen_top, onscreen_right, onscreen_bottom);

	// If this is a highlight layer and it's not too small, then draw the handle box handles onscreen and mark them as active
	if (TYPE_HIGHLIGHT == this_layer_data->object_type)
	{
		required_size_for_handles = (resize_handle_size * 2) + 1;
		if ((required_size_for_handles < width) && (required_size_for_handles < height))
		{
			draw_resize_handles(onscreen_left, onscreen_top, onscreen_right, onscreen_bottom);
			resize_handles_status |= RESIZE_HANDLES_WAITING;
		}
	} else
	{
		// Either wrong layer type or it's too small
		resize_handles_status = RESIZE_HANDLES_INACTIVE;
	}

	return TRUE;
}
