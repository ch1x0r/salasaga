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
#include "widgets/time_line.h"


gboolean draw_handle_box(void)
{
	// Local variables
	slide				*current_slide_data;		// Alias to make things easier
	gint				height;
	layer				*layer_data;
	gint				onscreen_bottom;			// Y coordinate of bounding box bottom
	gint				onscreen_left;				// X coordinate of bounding box left
	gint				onscreen_right;				// X coordinate of bounding box right
	gint				onscreen_top;				// Y coordinate of bounding box top
	gint				present_x;
	gint				present_y;
	guint				required_size_for_handles;	// Minimum size we need in order to draw any resize handles
	gfloat				scaled_height_ratio;		// Used to calculate a vertical scaling ratio 
	gfloat				scaled_width_ratio;			// Used to calculate a horizontal scaling ratio
	guint				selected_layer;				// Holds the number of the layer that is selected
	gint				width;


	// Only do this function if we have a front store available and a project loaded
	if ((NULL == front_store) || (FALSE == project_active))
	{
		return TRUE;
	}

	// Initialise some things
	current_slide_data = current_slide->data;

	// Determine which layer the user has selected in the timeline
	selected_layer = time_line_get_selected_layer_num(current_slide_data->timeline_widget);

	// Retrieve the dimensions of the selected object
	current_slide_data->layers = g_list_first(current_slide_data->layers);
	layer_data = g_list_nth_data(current_slide_data->layers, selected_layer);
	switch (layer_data->object_type)
	{
		case TYPE_EMPTY:
			// This is an empty layer, so clear any existing handle box
			gdk_draw_drawable(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(main_drawing_area->style->fg_gc[GTK_WIDGET_STATE(main_drawing_area)]),
					GDK_PIXMAP(front_store), 0, 0, 0, 0, -1, -1);
			return TRUE;

		case TYPE_HIGHLIGHT:
			present_x = layer_data->x_offset_start;
			present_y = layer_data->y_offset_start;
			width = ((layer_highlight *) layer_data->object_data)->width;
			height = ((layer_highlight *) layer_data->object_data)->height;
			break;

		case TYPE_GDK_PIXBUF:
			// If this is the background layer, then we clear any existing handle box
			if (TRUE == layer_data->background)
			{
				// Clear any existing handle box
				gdk_draw_drawable(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(main_drawing_area->style->fg_gc[GTK_WIDGET_STATE(main_drawing_area)]),
						GDK_PIXMAP(front_store), 0, 0, 0, 0, -1, -1);
				return TRUE;
			}

			// No it's not, so process it
			present_x = layer_data->x_offset_start;
			present_y = layer_data->y_offset_start;
			width = ((layer_image *) layer_data->object_data)->width;
			height = ((layer_image *) layer_data->object_data)->height;
			break;

		case TYPE_MOUSE_CURSOR:
			present_x = layer_data->x_offset_start;
			present_y = layer_data->y_offset_start;
			width = ((layer_mouse *) layer_data->object_data)->width;
			height = ((layer_mouse *) layer_data->object_data)->height;
			break;

		case TYPE_TEXT:
			present_x = layer_data->x_offset_start - 8;
			present_y = layer_data->y_offset_start;
			width = ((layer_text *) layer_data->object_data)->rendered_width - 6;
			height = ((layer_text *) layer_data->object_data)->rendered_height - 4;
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
	onscreen_left = (present_x + 1) / scaled_width_ratio;
	onscreen_top = (present_y + 1) / scaled_height_ratio;
	onscreen_right = (present_x + width) / scaled_width_ratio;
	onscreen_bottom = (present_y + height) / scaled_height_ratio;

	// Ensure the bounding box doesn't go out of bounds
	onscreen_left = CLAMP(onscreen_left, 2, main_drawing_area->allocation.width - (width / scaled_width_ratio) - 2);
	onscreen_top = CLAMP(onscreen_top, 2, main_drawing_area->allocation.height - (height / scaled_height_ratio) - 2);
	onscreen_right = CLAMP(onscreen_right, 2 + (width / scaled_width_ratio), main_drawing_area->allocation.width - 2);
	onscreen_bottom = CLAMP(onscreen_bottom, 2 + (height / scaled_height_ratio), main_drawing_area->allocation.height - 2);

	// Draw a bounding box onscreen
	draw_bounding_box(onscreen_left, onscreen_top, onscreen_right, onscreen_bottom);

	// If this is a highlight layer and it's not too small, then draw the handle box handles onscreen and mark them as active
	if (TYPE_HIGHLIGHT == layer_data->object_type)
	{
		required_size_for_handles = (resize_handle_size * 2) + 1;
		if ((required_size_for_handles < width) && (required_size_for_handles < height))
		{
			draw_resize_handles(onscreen_left, onscreen_top, onscreen_right, onscreen_bottom);
			resize_handles_status = RESIZE_HANDLES_WAITING;
		}
	} else
	{
		resize_handles_status = RESIZE_HANDLES_INACTIVE;
	}

	return TRUE;
}
