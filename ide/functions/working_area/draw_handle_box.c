/*
 * $Id$
 *
 * Salasaga: Draws a handle box (outline) on the workspace, around the selected layer
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


// Standard includes
#include <stdlib.h>

// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../dialog/display_warning.h"
#include "../cairo/calculate_text_object_size.h"
#include "../layer/get_layer_position.h"
#include "../widgets/time_line.h"
#include "draw_bounding_box.h"
#include "draw_resize_handles.h"


gboolean draw_handle_box(void)
{
	// Local variables
	slide				*current_slide_data;		// Alias to make things easier
	gint				finish_x;					// X position at the layer objects finish time
	gint				finish_y;					// Y position at the layer objects finish time
	GtkAllocation		layer_positions;			// Offset and dimensions for a given layer object
	gint				onscreen_bottom;			// Y coordinate of bounding box bottom
	gint				onscreen_left;				// X coordinate of bounding box left
	gint				onscreen_right;				// X coordinate of bounding box right
	gint				onscreen_top;				// Y coordinate of bounding box top
	gint				pixmap_height;				// Height of the front store
	gint				pixmap_width;				// Width of the front store
	guint				required_size_for_handles;	// Minimum size we need in order to draw any resize handles
	gboolean			return_code_gbool;			// Receives gboolean return codes
	gfloat				scaled_height_ratio;		// Used to calculate a vertical scaling ratio
	gfloat				scaled_width_ratio;			// Used to calculate a horizontal scaling ratio
	gint				selected_layer;				// Holds the number of the layer that is selected
	gfloat				start_time;					// Time in seconds of the layer objects start time
	gint				start_x;					// X position at the layer objects start time
	gint				start_y;					// Y position at the layer objects start time
	layer				*this_layer_data;
	gfloat				time_alpha;
	gfloat 				time_position;				// The point in time we need the handle box for


	// Only do this function if we have a front store available and a project loaded
	if ((NULL == front_store) || (FALSE == project_active))
	{
		return TRUE;
	}

	// Initialise some things
	current_slide_data = current_slide->data;
	gdk_drawable_get_size(GDK_PIXMAP(front_store), &pixmap_width, &pixmap_height);

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

	// Retrieve the layer position and alpha for the given point in time
	return_code_gbool = get_layer_position(&layer_positions, this_layer_data, time_position, &time_alpha);
	if (FALSE == return_code_gbool)
		return TRUE;

	// If this is a layer that isn't selectable, then clear any existing handle box and return
	switch (this_layer_data->object_type)
	{
		case TYPE_EMPTY:
			// This is an empty layer, so clear any existing handle box then return
			gdk_draw_drawable(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(main_drawing_area->style->fg_gc[GTK_WIDGET_STATE(main_drawing_area)]),
					GDK_PIXMAP(front_store), 0, 0, 0, 0, -1, -1);
			return TRUE;

		case TYPE_GDK_PIXBUF:
			// If this is the background layer, then we clear any existing handle box then return
			if (TRUE == this_layer_data->background)
			{
				// Clear any existing handle box then return
				gdk_draw_drawable(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(main_drawing_area->style->fg_gc[GTK_WIDGET_STATE(main_drawing_area)]),
						GDK_PIXMAP(front_store), 0, 0, 0, 0, -1, -1);
				return TRUE;
			}
	}

	// Calculate the height and width scaling values for the main drawing area at its present size
	scaled_height_ratio = (gfloat) project_height / (gfloat) pixmap_height;
	scaled_width_ratio = (gfloat) project_width / (gfloat) pixmap_width;

	// Work out the bounding box boundaries
	onscreen_left = (layer_positions.x + 1) / scaled_width_ratio;
	onscreen_top = (layer_positions.y + 1) / scaled_height_ratio;
	onscreen_right = (layer_positions.x + layer_positions.width) / scaled_width_ratio;
	onscreen_bottom = (layer_positions.y + layer_positions.height) / scaled_height_ratio;

	// Draw a bounding box onscreen
	draw_bounding_box(onscreen_left, onscreen_top, onscreen_right, onscreen_bottom);

	// If this is a highlight layer and it's not too small, then draw the handle box handles onscreen and mark them as active
	if (TYPE_HIGHLIGHT == this_layer_data->object_type)
	{
		required_size_for_handles = (resize_handle_size * 2) + 1;
		if ((required_size_for_handles < layer_positions.width) && (required_size_for_handles < layer_positions.height))
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
