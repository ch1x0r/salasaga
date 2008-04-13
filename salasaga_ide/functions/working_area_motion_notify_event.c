/*
 * $Id$
 *
 * Salasaga: Called when the user is dragging the mouse on the drawing area 
 * 
 * Copyright (C) 2007-2008 Justin Clift <justin@salasaga.org>
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
#include "draw_bounding_box.h"
#include "display_warning.h"
#include "widgets/time_line.h"


gboolean working_area_motion_notify_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	// Local variables
	GdkModifierType		button_state;				// Mouse button states
	slide				*current_slide_data;		// Alias to make things easier
	gfloat				end_time;					// Time in seconds of the layer objects finish time
	gint				finish_x;					// X position at the layer objects finish time
	gint				finish_y;					// Y position at the layer objects finish time 
	gint				height;
	layer				*layer_data;				// Data for the layer we're working on
	GdkRectangle		mouse_pointer_rect;			// Rectangle holding the mouse pointer position
	gint				mouse_x;					// Unscaled mouse x position
	gint				mouse_y;					// Unscaled mouse x position
	gint				onscreen_bottom;			// Y coordinate of bounding box bottom
	gint				onscreen_left;				// X coordinate of bounding box left
	gint				onscreen_right;				// X coordinate of bounding box right
	gint				onscreen_top;				// Y coordinate of bounding box top
	gint				present_x;
	gint				present_y;
	gfloat				scaled_height_ratio;		// Used to calculate a vertical scaling ratio 
	gfloat				scaled_width_ratio;			// Used to calculate a horizontal scaling ratio
	gint				selected_row;				// Holds the number of the row that is selected
	gfloat				start_time;					// Time in seconds of the layer objects start time
	gint				start_x;					// X position at the layer objects start time
	gint				start_y;					// Y position at the layer objects start time
	gfloat				time_offset;
	gint				time_x;						// Unscaled X position of the layer at our desired point in time
	gint				time_y;						// Unscaled Y position of the layer at our desired point in time
	gfloat				time_diff;					// Used when calculating the object position at the desired point in time
	gfloat				time_position;
	gint				width;
	gfloat				x_diff;						// The X distance the object was dragged, after scaling
	gfloat				x_scale;					// Used when calculating the object position at the desired point in time
	gfloat				y_diff;						// The Y distance the object was dragged, after scaling
	gfloat				y_scale;					// Used when calculating the object position at the desired point in time


	// If the current slide hasn't been initialised, or there is no project active don't run this function
	if ((NULL == current_slide) || (FALSE == project_active))
	{
		return TRUE;
	}

	// Find out where the mouse is positioned, and which buttons and modifier keys are down (active)
	gdk_window_get_pointer(event->window, &mouse_x, &mouse_y, &button_state);

	// If we're creating a new highlight layer, draw a bounding box
	if (TYPE_HIGHLIGHT == new_layer_selected)
	{
		// Draw the updated bounding box
		draw_bounding_box(stored_x, stored_y, mouse_x, mouse_y);
		return TRUE;
	}

	// If we're already aware of a resize operation going on, then draw the appropriate bounding box
	if (FALSE != (RESIZE_HANDLES_RESIZING & resize_handles_status))
	{
		// Initialise some things
		current_slide_data = current_slide->data;

		// Calculate the height and width scaling values for the main drawing area at its present size
		scaled_height_ratio = (gfloat) project_height / (gfloat) main_drawing_area->allocation.height;
		scaled_width_ratio = (gfloat) project_width / (gfloat) main_drawing_area->allocation.width;

		// Determine which layer is selected in the timeline
		selected_row = time_line_get_selected_layer_num(current_slide_data->timeline_widget);

		// Get its present X and Y offsets
		current_slide_data->layers = g_list_first(current_slide_data->layers);
		layer_data = g_list_nth_data(current_slide_data->layers, selected_row);
		switch (layer_data->object_type)
		{
			case TYPE_HIGHLIGHT:
				present_x = layer_data->x_offset_start;
				present_y = layer_data->y_offset_start;
				width = ((layer_highlight *) layer_data->object_data)->width;
				height = ((layer_highlight *) layer_data->object_data)->height;
				break;

			default:
				return FALSE;  // Unknown layer type, so ignore this event
		}

		// Calculate the distance the object has been dragged onscreen
		x_diff = mouse_x - stored_x;
		y_diff = mouse_y - stored_y;

		// Work out the bounding box boundaries
		switch (resize_handles_status & RESIZE_HANDLES_RESIZING_ALL)
		{
			case RESIZE_HANDLES_RESIZING_TL:
				// Top left resize
				onscreen_left = (present_x / scaled_width_ratio) + x_diff;
				onscreen_top = (present_y / scaled_height_ratio) + y_diff;
				onscreen_right = ((present_x + width) / scaled_width_ratio);
				onscreen_bottom = ((present_y + height) / scaled_height_ratio);
				break;

			case RESIZE_HANDLES_RESIZING_TM:
				// Top middle resize
				onscreen_left = (present_x / scaled_width_ratio);
				onscreen_top = (present_y / scaled_height_ratio) + y_diff;
				onscreen_right = ((present_x + width) / scaled_width_ratio);
				onscreen_bottom = ((present_y + height) / scaled_height_ratio);
				break;

			case RESIZE_HANDLES_RESIZING_TR:
				// Top right resize
				onscreen_left = (present_x / scaled_width_ratio);
				onscreen_top = (present_y / scaled_height_ratio) + y_diff;
				onscreen_right = ((present_x + width) / scaled_width_ratio) + x_diff;
				onscreen_bottom = ((present_y + height) / scaled_height_ratio);
				break;

			case RESIZE_HANDLES_RESIZING_RM:
				// Middle right resize
				onscreen_left = (present_x / scaled_width_ratio);
				onscreen_top = (present_y / scaled_height_ratio);
				onscreen_right = ((present_x + width) / scaled_width_ratio) + x_diff;
				onscreen_bottom = ((present_y + height) / scaled_height_ratio);
				break;

			case RESIZE_HANDLES_RESIZING_BR:
				// Bottom right resize
				onscreen_left = (present_x / scaled_width_ratio);
				onscreen_top = (present_y / scaled_height_ratio);
				onscreen_right = ((present_x + width) / scaled_width_ratio) + x_diff;
				onscreen_bottom = ((present_y + height) / scaled_height_ratio) + y_diff;
				break;

			case RESIZE_HANDLES_RESIZING_BM:
				// Bottom middle resize
				onscreen_left = (present_x / scaled_width_ratio);
				onscreen_top = (present_y / scaled_height_ratio);
				onscreen_right = ((present_x + width) / scaled_width_ratio);
				onscreen_bottom = ((present_y + height) / scaled_height_ratio) + y_diff;
				break;

			case RESIZE_HANDLES_RESIZING_BL:
				// Bottom left resize
				onscreen_left = (present_x / scaled_width_ratio) + x_diff;
				onscreen_top = (present_y / scaled_height_ratio);
				onscreen_right = ((present_x + width) / scaled_width_ratio);
				onscreen_bottom = ((present_y + height) / scaled_height_ratio) + y_diff;
				break;

			case RESIZE_HANDLES_RESIZING_LM:
				// Left middle resize
				onscreen_left = (present_x / scaled_width_ratio) + x_diff;
				onscreen_top = (present_y / scaled_height_ratio);
				onscreen_right = ((present_x + width) / scaled_width_ratio);
				onscreen_bottom = ((present_y + height) / scaled_height_ratio);
				break;

			default:
				display_warning("Error ED295: Unknown resizing direction");
				return FALSE;
		}

		// Ensure the bounding box doesn't go out of bounds
		onscreen_left = CLAMP(onscreen_left, 2, main_drawing_area->allocation.width - 2);
		onscreen_top = CLAMP(onscreen_top, 2, main_drawing_area->allocation.height - 2);
		onscreen_right = CLAMP(onscreen_right, 2, main_drawing_area->allocation.width - 2);
		onscreen_bottom = CLAMP(onscreen_bottom, 2, main_drawing_area->allocation.height - 2);

		// Draw a bounding box onscreen
		draw_bounding_box(onscreen_left, onscreen_top, onscreen_right, onscreen_bottom);

		return TRUE;
	}

	// If we're already aware of a mouse drag operation going on, then draw a bounding box
	if (TRUE == mouse_dragging)
	{
		// Initialise some things
		current_slide_data = current_slide->data;

		// Determine which layer is selected in the timeline
		selected_row = time_line_get_selected_layer_num(current_slide_data->timeline_widget);

		// Find out where the time line cursor is
		time_position = time_line_get_cursor_position(current_slide_data->timeline_widget);

		// Get its present X and Y offsets
		current_slide_data->layers = g_list_first(current_slide_data->layers);
		layer_data = g_list_nth_data(current_slide_data->layers, selected_row);

		// Simplify pointers
		finish_x = layer_data->x_offset_finish;
		finish_y = layer_data->y_offset_finish;
		start_time = layer_data->start_time;
		start_x = layer_data->x_offset_start;
		start_y = layer_data->y_offset_start;
		end_time = layer_data->start_time + layer_data->duration;
		if (TRANS_LAYER_NONE != layer_data->transition_in_type)
			end_time += layer_data->transition_in_duration;
		if (TRANS_LAYER_NONE != layer_data->transition_out_type)
			end_time += layer_data->transition_out_duration;

		// Calculate how far into the layer movement we are
		time_offset = time_position - start_time;
		time_diff = end_time - start_time;
		x_diff = finish_x - start_x;
		x_scale = (((gfloat) x_diff) / time_diff);
		time_x = start_x + (x_scale * time_offset);
		y_diff = finish_y - start_y;
		y_scale = (((gfloat) y_diff) / time_diff);
		time_y = start_y + (y_scale * time_offset);

		// Calculate the height and width scaling values for the main drawing area at its present size
		scaled_height_ratio = (gfloat) project_height / (gfloat) main_drawing_area->allocation.height;
		scaled_width_ratio = (gfloat) project_width / (gfloat) main_drawing_area->allocation.width;
		switch (layer_data->object_type)
		{
			case TYPE_EMPTY:
				// We can't drag an empty layer, so reset things and return
				mouse_dragging = FALSE;
				stored_x = -1;
				stored_y = -1;
				return TRUE;

			case TYPE_HIGHLIGHT:
				present_x = time_x;
				present_y = time_y;
				width = ((layer_highlight *) layer_data->object_data)->width;
				height = ((layer_highlight *) layer_data->object_data)->height;
				break;

			case TYPE_GDK_PIXBUF:
				// If this is the background layer, then we ignore it
				if (TRUE == layer_data->background)
				{
					mouse_dragging = FALSE;
					stored_x = -1;
					stored_y = -1;
					return TRUE;
				}

				// No it's not, so process it
				present_x = time_x;
				present_y = time_y;
				width = ((layer_image *) layer_data->object_data)->width;
				height = ((layer_image *) layer_data->object_data)->height;
				break;

			case TYPE_MOUSE_CURSOR:
				present_x = time_x;
				present_y = time_y;
				width = ((layer_mouse *) layer_data->object_data)->width;
				height = ((layer_mouse *) layer_data->object_data)->height;
				break;

			case TYPE_TEXT:
				present_x = time_x - 12;
				present_y = time_y - 4;
				width = ((layer_text *) layer_data->object_data)->rendered_width + 2;
				height = ((layer_text *) layer_data->object_data)->rendered_height + 2;
				break;

			default:
				display_warning("Error ED284: Unknown layer type");

				return TRUE;  // Unknown layer type, so no idea how to extract the needed data for the next code
		}

		// Calculate the distance the object has been dragged onscreen
		x_diff = mouse_x - stored_x;
		y_diff = mouse_y - stored_y;

		// Work out the bounding box boundaries
		onscreen_left = (present_x / scaled_width_ratio) + x_diff;
		onscreen_top = (present_y / scaled_height_ratio) + y_diff;
		onscreen_right = ((present_x + width) / scaled_width_ratio) + x_diff;
		onscreen_bottom = ((present_y + height) / scaled_height_ratio) + y_diff;

		// Ensure the bounding box doesn't go out of bounds
		onscreen_left = CLAMP(onscreen_left, 2, main_drawing_area->allocation.width - (width / scaled_width_ratio) - 2);
		onscreen_top = CLAMP(onscreen_top, 2, main_drawing_area->allocation.height - (height / scaled_height_ratio) - 2);
		onscreen_right = CLAMP(onscreen_right, 2 + (width / scaled_width_ratio), main_drawing_area->allocation.width - 2);
		onscreen_bottom = CLAMP(onscreen_bottom, 2 + (height / scaled_height_ratio), main_drawing_area->allocation.height - 2);

		// Draw a bounding box onscreen
		draw_bounding_box(onscreen_left, onscreen_top, onscreen_right, onscreen_bottom);

		return TRUE;
	}

	// * To get here, this must be the first time we've heard of this particular drag operation *

	// Check if the primary mouse button is down and we're open to being clicked on the resize handles
	if ((GDK_BUTTON1_MASK & button_state) && (RESIZE_HANDLES_WAITING == resize_handles_status))
	{
		// Create the mouse pointer rectangle
		mouse_pointer_rect.x = event->x;
		mouse_pointer_rect.y = event->y;
		mouse_pointer_rect.width = 10;  // I think that the bigger this is, the more likely for the handle click to happen 
		mouse_pointer_rect.height = 10;  // I think that the bigger this is, the more likely for the handle click to happen

		// * Check if the mouse is over a corner resize handle *

		// Top left
		if (TRUE == gdk_rectangle_intersect(&resize_handles_rect[0], &mouse_pointer_rect, NULL))
		{
			// We're resizing from the top left
			resize_handles_status = RESIZE_HANDLES_RESIZING | RESIZE_HANDLES_RESIZING_TL;
		}

		// Top right
		if (TRUE == gdk_rectangle_intersect(&resize_handles_rect[2], &mouse_pointer_rect, NULL))
		{
			// We're resizing from the top right
			resize_handles_status = RESIZE_HANDLES_RESIZING | RESIZE_HANDLES_RESIZING_TR;
		}

		// Bottom right
		if (TRUE == gdk_rectangle_intersect(&resize_handles_rect[4], &mouse_pointer_rect, NULL))
		{
			// We're resizing from the bottom right
			resize_handles_status = RESIZE_HANDLES_RESIZING | RESIZE_HANDLES_RESIZING_BR;
		}

		// Bottom left
		if (TRUE == gdk_rectangle_intersect(&resize_handles_rect[6], &mouse_pointer_rect, NULL))
		{
			// We're resizing from the bottom left
			resize_handles_status = RESIZE_HANDLES_RESIZING | RESIZE_HANDLES_RESIZING_BL;
		}

		// If the horizontal resize handles are in use, check them
		if (0 != resize_handles_rect[3].width)
		{
			// Right middle
			if (TRUE == gdk_rectangle_intersect(&resize_handles_rect[3], &mouse_pointer_rect, NULL))
			{
				// We're resizing from the right middle
				resize_handles_status = RESIZE_HANDLES_RESIZING | RESIZE_HANDLES_RESIZING_RM;
			}

			// Left middle
			if (TRUE == gdk_rectangle_intersect(&resize_handles_rect[7], &mouse_pointer_rect, NULL))
			{
				// We're resizing from the left middle
				resize_handles_status = RESIZE_HANDLES_RESIZING | RESIZE_HANDLES_RESIZING_LM;
			}
		}

		// If the vertical resize handles are in use, check them
		if (0 != resize_handles_rect[1].width)
		{
			// Top middle
			if (TRUE == gdk_rectangle_intersect(&resize_handles_rect[1], &mouse_pointer_rect, NULL))
			{
				// We're resizing from the top middle
				resize_handles_status = RESIZE_HANDLES_RESIZING | RESIZE_HANDLES_RESIZING_TM;
			}

			// Bottom middle
			if (TRUE == gdk_rectangle_intersect(&resize_handles_rect[5], &mouse_pointer_rect, NULL))
			{
				// We're resizing from the bottom middle
				resize_handles_status = RESIZE_HANDLES_RESIZING | RESIZE_HANDLES_RESIZING_BM;
			}
		}

		// Store the mouse coordinates so we know where to resize from
		stored_x = event->x;
		stored_y = event->y;

		// Reset the invalidation area
		invalidation_end_x = event->x;
		invalidation_end_y = event->y;
		invalidation_start_x = event->x - 1;
		invalidation_start_y = event->y - 1;

		// If we're resizing, then return, else drop through to the next check
		if (FALSE != (RESIZE_HANDLES_RESIZING & resize_handles_status))
			return TRUE;
	}

	// Check if the primary mouse button is down and we're not resizing a layer
	if ((GDK_BUTTON1_MASK & button_state) && (RESIZE_HANDLES_RESIZING != resize_handles_status))
	{
		// We're commencing a drag, so note this
		mouse_dragging = TRUE;

		// Store the mouse coordinates so we know where to drag from
		stored_x = event->x;
		stored_y = event->y;

		// Reset the invalidation area
		invalidation_end_x = event->x;
		invalidation_end_y = event->y;
		invalidation_start_x = event->x - 1;
		invalidation_start_y = event->y - 1;

		return TRUE;
	}

	return TRUE;
}
