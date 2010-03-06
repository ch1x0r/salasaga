/*
 * $Id$
 *
 * Salasaga: Called when the user is dragging the mouse on the drawing area
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
#include "../layer/get_layer_position.h"
#include "../widgets/time_line/time_line_get_cursor_position.h"
#include "../widgets/time_line/time_line_get_selected_layer_num.h"
#include "draw_bounding_box.h"


gboolean working_area_motion_notify_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	// Local variables
	GdkModifierType		button_state;				// Mouse button states
	gint				cursor_movement_x;
	gint				cursor_movement_y;
	gint				finish_x;					// X position at the layer objects finish time
	gint				finish_y;					// Y position at the layer objects finish time
	gint				height;
	layer				*layer_data;				// Data for the layer we're working on
	GtkAllocation		layer_positions;			// Offset and dimensions for a given layer object
	GString				*message;					// Used to construct message strings
	GdkRectangle		mouse_pointer_rect;			// Rectangle holding the mouse pointer position
	gint				mouse_x;					// Unscaled mouse x position
	gint				mouse_y;					// Unscaled mouse x position
	gint				new_x_val;
	gint				new_y_val;
	gint				onscreen_bottom;			// Y coordinate of bounding box bottom
	gint				onscreen_left;				// X coordinate of bounding box left
	gint				onscreen_right;				// X coordinate of bounding box right
	gint				onscreen_top;				// Y coordinate of bounding box top
	gint				pixmap_height;				// Height of the front store
	gint				pixmap_width;				// Width of the front store
	gboolean			return_code_gbool;			// Receives gboolean return codes
	gfloat				scaled_height_ratio;		// Used to calculate a vertical scaling ratio
	gfloat				scaled_width_ratio;			// Used to calculate a horizontal scaling ratio
	gint				selected_row;				// Holds the number of the row that is selected
	gint				start_x;					// X position at the layer objects start time
	gint				start_y;					// Y position at the layer objects start time
	static gint			stored_x_val;
	static gint			stored_y_val;
	slide				*this_slide_data;			// Alias to make things easier
	gfloat				time_alpha = 1.0;			// Alpha value to use at our desired point in time (defaulting to 1.0 = fall opacity)
	gint				time_x;						// Unscaled X position of the layer at our desired point in time
	gint				time_y;						// Unscaled Y position of the layer at our desired point in time
	gfloat				time_position;
	gint				width;
	gint				x_diff = 0;					// The X distance the object was dragged, after scaling
	gint				y_diff = 0;					// The Y distance the object was dragged, after scaling


	// If the current slide hasn't been initialised, or there is no project active don't run this function
	if ((NULL == current_slide) || (FALSE == get_project_active()))
	{
		return TRUE;
	}

	// Find out where the mouse is positioned, and which buttons and modifier keys are down (active)
	gdk_window_get_pointer(event->window, &mouse_x, &mouse_y, &button_state);

	// Initialise things
	gdk_drawable_get_size(GDK_PIXMAP(front_store), &pixmap_width, &pixmap_height);

	// If we're creating a new highlight layer, draw a bounding box
	if (TYPE_HIGHLIGHT == get_new_layer_selected())
	{
		// Draw the updated bounding box
		draw_bounding_box(get_stored_x(), get_stored_y(), mouse_x, mouse_y);
		return TRUE;
	}

	// If we're already aware of a resize operation going on, then draw the appropriate bounding box
	if (FALSE != (RESIZE_HANDLES_RESIZING & resize_handles_status))
	{
		// Initialise some things
		this_slide_data = current_slide->data;

		// Determine which layer is selected in the timeline
		selected_row = time_line_get_selected_layer_num(this_slide_data->timeline_widget);

		// Find out where the time line cursor is
		time_position = time_line_get_cursor_position(this_slide_data->timeline_widget);

		// Get its present X and Y offsets
		this_slide_data->layers = g_list_first(this_slide_data->layers);
		layer_data = g_list_nth_data(this_slide_data->layers, selected_row);
		return_code_gbool = get_layer_position(&layer_positions, layer_data, time_position, &time_alpha);
		if (FALSE == return_code_gbool)
			return TRUE;

		// Calculate the height and width scaling values for the main drawing area at its present size
		scaled_height_ratio = (gfloat) project_height / (gfloat) pixmap_height;
		scaled_width_ratio = (gfloat) project_width / (gfloat) pixmap_width;

		// Calculate the distance the object has been dragged onscreen
		x_diff = mouse_x - get_stored_x();
		y_diff = mouse_y - get_stored_y();

		// Work out the bounding box boundaries
		switch (resize_handles_status & RESIZE_HANDLES_RESIZING_ALL)
		{
			case RESIZE_HANDLES_RESIZING_TL:
				// Top left resize
				onscreen_left = (layer_positions.x / scaled_width_ratio) + x_diff;
				onscreen_top = (layer_positions.y / scaled_height_ratio) + y_diff;
				onscreen_right = ((layer_positions.x + layer_positions.width) / scaled_width_ratio);
				onscreen_bottom = ((layer_positions.y + layer_positions.height) / scaled_height_ratio);
				break;

			case RESIZE_HANDLES_RESIZING_TM:
				// Top middle resize
				onscreen_left = (layer_positions.x / scaled_width_ratio);
				onscreen_top = (layer_positions.y / scaled_height_ratio) + y_diff;
				onscreen_right = ((layer_positions.x + layer_positions.width) / scaled_width_ratio);
				onscreen_bottom = ((layer_positions.y + layer_positions.height) / scaled_height_ratio);
				break;

			case RESIZE_HANDLES_RESIZING_TR:
				// Top right resize
				onscreen_left = (layer_positions.x / scaled_width_ratio);
				onscreen_top = (layer_positions.y / scaled_height_ratio) + y_diff;
				onscreen_right = ((layer_positions.x + layer_positions.width) / scaled_width_ratio) + x_diff;
				onscreen_bottom = ((layer_positions.y + layer_positions.height) / scaled_height_ratio);
				break;

			case RESIZE_HANDLES_RESIZING_RM:
				// Middle right resize
				onscreen_left = (layer_positions.x / scaled_width_ratio);
				onscreen_top = (layer_positions.y / scaled_height_ratio);
				onscreen_right = ((layer_positions.x + layer_positions.width) / scaled_width_ratio) + x_diff;
				onscreen_bottom = ((layer_positions.y + layer_positions.height) / scaled_height_ratio);
				break;

			case RESIZE_HANDLES_RESIZING_BR:
				// Bottom right resize
				onscreen_left = (layer_positions.x / scaled_width_ratio);
				onscreen_top = (layer_positions.y / scaled_height_ratio);
				onscreen_right = ((layer_positions.x + layer_positions.width) / scaled_width_ratio) + x_diff;
				onscreen_bottom = ((layer_positions.y + layer_positions.height) / scaled_height_ratio) + y_diff;
				break;

			case RESIZE_HANDLES_RESIZING_BM:
				// Bottom middle resize
				onscreen_left = (layer_positions.x / scaled_width_ratio);
				onscreen_top = (layer_positions.y / scaled_height_ratio);
				onscreen_right = ((layer_positions.x + layer_positions.width) / scaled_width_ratio);
				onscreen_bottom = ((layer_positions.y + layer_positions.height) / scaled_height_ratio) + y_diff;
				break;

			case RESIZE_HANDLES_RESIZING_BL:
				// Bottom left resize
				onscreen_left = (layer_positions.x / scaled_width_ratio) + x_diff;
				onscreen_top = (layer_positions.y / scaled_height_ratio);
				onscreen_right = ((layer_positions.x + layer_positions.width) / scaled_width_ratio);
				onscreen_bottom = ((layer_positions.y + layer_positions.height) / scaled_height_ratio) + y_diff;
				break;

			case RESIZE_HANDLES_RESIZING_LM:
				// Left middle resize
				onscreen_left = (layer_positions.x / scaled_width_ratio) + x_diff;
				onscreen_top = (layer_positions.y / scaled_height_ratio);
				onscreen_right = ((layer_positions.x + layer_positions.width) / scaled_width_ratio);
				onscreen_bottom = ((layer_positions.y + layer_positions.height) / scaled_height_ratio);
				break;

			default:
				message = g_string_new(NULL);
				g_string_printf(message, "%s ED295: %s", _("Error"), _("Unknown resizing direction."));
				display_warning(message->str);
				g_string_free(message, TRUE);
				return FALSE;
		}

		// Draw a bounding box onscreen
		draw_bounding_box(onscreen_left, onscreen_top, onscreen_right, onscreen_bottom);

		return TRUE;
	}

	// * Start and end point movement detection code *
	if ((FALSE == get_mouse_dragging())							// Not dragging mouse already
		&& ((RESIZE_HANDLES_WAITING == resize_handles_status)
		|| (RESIZE_HANDLES_INACTIVE == resize_handles_status)))	// Not resizing a layer already
	{
		// Initialise some things
		this_slide_data = current_slide->data;
		this_slide_data->layers = g_list_first(this_slide_data->layers);

		// Determine which layer is selected in the timeline
		selected_row = time_line_get_selected_layer_num(this_slide_data->timeline_widget);

		// Find out where the time line cursor is
		time_position = time_line_get_cursor_position(this_slide_data->timeline_widget);

		// Calculate the height and width scaling values for the main drawing area at its present size
		scaled_height_ratio = (gfloat) project_height / (gfloat) pixmap_height;
		scaled_width_ratio = (gfloat) project_width / (gfloat) pixmap_width;

		// Calculate start and end points
		layer_data = g_list_nth_data(this_slide_data->layers, selected_row);
		finish_x = (layer_data->x_offset_finish / scaled_width_ratio) + END_POINT_HORIZONTAL_OFFSET;
		finish_y = (layer_data->y_offset_finish / scaled_height_ratio) + END_POINT_VERTICAL_OFFSET;
		start_x = (layer_data->x_offset_start / scaled_width_ratio) + END_POINT_HORIZONTAL_OFFSET;
		start_y = (layer_data->y_offset_start / scaled_height_ratio) + END_POINT_VERTICAL_OFFSET;

		// Check if the user is clicking on the layer start or end points
		if (END_POINTS_INACTIVE == get_end_point_status())
		{
			// Is the user clicking on a start or end point?
			if (((event->x >= start_x)							// Start point
				&& (event->x <= start_x + END_POINT_WIDTH)
				&& (event->y >= start_y)
				&& (event->y <= start_y + END_POINT_HEIGHT)) ||
				((event->x >= finish_x)							// End point
				&& (event->x <= finish_x + END_POINT_WIDTH)
				&& (event->y >= finish_y)
				&& (event->y <= finish_y + END_POINT_HEIGHT)))
			{
				// Is it the start point?
				if ((event->x >= start_x)							// Left
					&& (event->x <= start_x + END_POINT_WIDTH)		// Right
					&& (event->y >= start_y)						// Top
					&& (event->y <= start_y + END_POINT_HEIGHT))	// Bottom
				{
					// Start point clicked
					set_end_point_status(END_POINTS_START_ACTIVE);
					set_stored_x(event->x);
					set_stored_y(event->y);
					stored_x_val = layer_data->x_offset_start;
					stored_y_val = layer_data->y_offset_start;
					return TRUE;
				} else
				{
					// End point clicked
					set_end_point_status(END_POINTS_END_ACTIVE);
					set_stored_x(event->x);
					set_stored_y(event->y);
					stored_x_val = layer_data->x_offset_finish;
					stored_y_val = layer_data->y_offset_finish;
					return TRUE;
				}
			}
		}
	}

	// Check if we're already aware of an end point drag operation going on
	if (END_POINTS_INACTIVE != get_end_point_status())
	{
		// Initialise some things
		this_slide_data = current_slide->data;
		selected_row = time_line_get_selected_layer_num(this_slide_data->timeline_widget);
		this_slide_data->layers = g_list_first(this_slide_data->layers);
		layer_data = g_list_nth_data(this_slide_data->layers, selected_row);

		// Calculate the height and width scaling values for the main drawing area at its present size
		scaled_height_ratio = (gfloat) project_height / (gfloat) pixmap_height;
		scaled_width_ratio = (gfloat) project_width / (gfloat) pixmap_width;

		// Find out where the time line cursor is
		time_position = time_line_get_cursor_position(this_slide_data->timeline_widget);

		// Work out the distance the cursor has moved in screen pixels
		cursor_movement_x = event->x - get_stored_x();
		cursor_movement_y = event->y - get_stored_y();

		// Calculate the distance the object has been dragged
		x_diff = cursor_movement_x * scaled_width_ratio;
		y_diff = cursor_movement_y * scaled_height_ratio;

		// If the start or end point is being moved, we use the start or end time instead of time line cursor time
		if (END_POINTS_START_ACTIVE == get_end_point_status())
		{
			// The start point is being dragged
			time_x = stored_x_val;
			time_y = stored_y_val;
		}
		if (END_POINTS_END_ACTIVE == get_end_point_status())
		{
			// The end point is being dragged
			time_x = stored_x_val;
			time_y = stored_y_val;
		}

		// Retrieve the layer size information
		switch (layer_data->object_type)
		{
			case TYPE_EMPTY:
				// We can't drag an empty layer, so reset things and return
				set_mouse_dragging(FALSE);
				set_end_point_status(END_POINTS_INACTIVE);
				set_stored_x(-1);
				set_stored_y(-1);
				return TRUE;

			case TYPE_HIGHLIGHT:
				width = ((layer_highlight *) layer_data->object_data)->width;
				height = ((layer_highlight *) layer_data->object_data)->height;
				break;

			case TYPE_GDK_PIXBUF:
				// If this is the background layer, then we ignore it
				if (TRUE == layer_data->background)
				{
					set_mouse_dragging(FALSE);
					set_end_point_status(END_POINTS_INACTIVE);
					set_stored_x(-1);
					set_stored_y(-1);
					return TRUE;
				}

				// No it's not, so process it
				width = ((layer_image *) layer_data->object_data)->width;
				height = ((layer_image *) layer_data->object_data)->height;
				break;

			case TYPE_MOUSE_CURSOR:
				width = ((layer_mouse *) layer_data->object_data)->width;
				height = ((layer_mouse *) layer_data->object_data)->height;
				break;

			case TYPE_TEXT:
				width = ((layer_text *) layer_data->object_data)->rendered_width;
				height = ((layer_text *) layer_data->object_data)->rendered_height;
				break;

			default:
				message = g_string_new(NULL);
				g_string_printf(message, "%s ED284: %s", _("Error"), _("Unknown layer type."));
				display_warning(message->str);
				g_string_free(message, TRUE);

				return TRUE;  // Unknown layer type, so no idea how to extract the needed data for the next code
		}

		// Work out the bounding box boundaries (scaled)
		new_x_val = stored_x_val + x_diff;
		new_y_val = stored_y_val + y_diff;
		onscreen_right = new_x_val + width;
		onscreen_bottom = new_y_val + height;

		// Update the layer object positions
		if (END_POINTS_START_ACTIVE == get_end_point_status())
		{
			// Bounds check the starting x offset, then update the object with the new value
			layer_data->x_offset_start = new_x_val;

			// Bounds check the starting y offset, then update the object with the new value
			layer_data->y_offset_start = new_y_val;
		}
		if (END_POINTS_END_ACTIVE == get_end_point_status())
		{
			// Bounds check the finishing x offset, then update the object with the new value
			layer_data->x_offset_finish = new_x_val;

			// Bounds check the finishing y offset, then update the object with the new value
			layer_data->y_offset_finish = new_y_val;
		}

		// Scale the bounding box boundaries
		onscreen_left = new_x_val / scaled_width_ratio;
		onscreen_top = new_y_val / scaled_height_ratio;
		onscreen_right /= scaled_width_ratio;
		onscreen_bottom /= scaled_height_ratio;

		// Draw a bounding box onscreen
		draw_bounding_box(onscreen_left, onscreen_top, onscreen_right, onscreen_bottom);

		return TRUE;
	}

	// If we're already aware of a mouse drag operation going on, then draw a bounding box
	if (TRUE == get_mouse_dragging())
	{
		// Initialise some things
		this_slide_data = current_slide->data;

		// Calculate the height and width scaling values for the main drawing area at its present size
		scaled_height_ratio = (gfloat) project_height / (gfloat) pixmap_height;
		scaled_width_ratio = (gfloat) project_width / (gfloat) pixmap_width;

		// Determine which layer is selected in the timeline
		selected_row = time_line_get_selected_layer_num(this_slide_data->timeline_widget);

		// Find out where the time line cursor is
		time_position = time_line_get_cursor_position(this_slide_data->timeline_widget);

		// Get its present X and Y offsets
		this_slide_data->layers = g_list_first(this_slide_data->layers);
		layer_data = g_list_nth_data(this_slide_data->layers, selected_row);
		return_code_gbool = get_layer_position(&layer_positions, layer_data, time_position, &time_alpha);
		if (FALSE == return_code_gbool)
			return TRUE;

		// Calculate the distance the object has been dragged onscreen
		x_diff = mouse_x - get_stored_x();
		y_diff = mouse_y - get_stored_y();

		// Work out the bounding box boundaries
		onscreen_left = (layer_positions.x / scaled_width_ratio) + x_diff;
		onscreen_top = (layer_positions.y / scaled_height_ratio) + y_diff;
		onscreen_right = ((layer_positions.x + layer_positions.width) / scaled_width_ratio) + x_diff;
		onscreen_bottom = ((layer_positions.y + layer_positions.height) / scaled_height_ratio) + y_diff;

		// Draw a bounding box onscreen
		draw_bounding_box(onscreen_left, onscreen_top, onscreen_right, onscreen_bottom);

		return TRUE;
	}

	// * To get here, this must be the first time we've heard of this particular drag operation *

	// Check if the primary mouse button is down and we're open to being clicked on the resize handles
	if ((GDK_BUTTON1_MASK & button_state) && (RESIZE_HANDLES_WAITING == resize_handles_status))
	{
		// Create the mouse pointer rectangle
		mouse_pointer_rect.x = event->x - 5;
		mouse_pointer_rect.y = event->y - 5;
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

		// If we're resizing, then return, else drop through to the next check
		if (FALSE != (RESIZE_HANDLES_RESIZING & resize_handles_status))
		{
			// Store the mouse coordinates so we know where to resize from
			set_stored_x(event->x);
			set_stored_y(event->y);

			// Reset the invalidation area
			set_invalidation_end_x(event->x);
			set_invalidation_end_y(event->y);
			set_invalidation_start_x(event->x - 1);
			set_invalidation_start_y(event->y - 1);
		}
	}

	// Check if the primary mouse button is down and we're not resizing a layer
	if ((GDK_BUTTON1_MASK & button_state)
		&& ((RESIZE_HANDLES_WAITING == resize_handles_status)
		|| (RESIZE_HANDLES_INACTIVE == resize_handles_status)))	// Not resizing a layer already
	{
		// We're commencing a drag, so note this
		set_mouse_dragging(TRUE);

		// Initialise some things
		this_slide_data = current_slide->data;

		// Calculate the height and width scaling values for the main drawing area at its present size
		scaled_height_ratio = (gfloat) project_height / (gfloat) pixmap_height;
		scaled_width_ratio = (gfloat) project_width / (gfloat) pixmap_width;

		// Determine which layer is selected in the timeline
		selected_row = time_line_get_selected_layer_num(this_slide_data->timeline_widget);

		// Find out where the time line cursor is
		time_position = time_line_get_cursor_position(this_slide_data->timeline_widget);

		// Get its present X and Y offsets
		this_slide_data->layers = g_list_first(this_slide_data->layers);
		layer_data = g_list_nth_data(this_slide_data->layers, selected_row);
		return_code_gbool = get_layer_position(&layer_positions, layer_data, time_position, &time_alpha);
		if (FALSE == return_code_gbool)
			return TRUE;

		// Store the mouse coordinates so we know where to drag from
		set_stored_x(event->x);
		set_stored_y(event->y);

		// Reset the invalidation area to the size of the selected layer
		set_invalidation_end_x(((layer_positions.x + layer_positions.width) / scaled_width_ratio) + x_diff);
		set_invalidation_end_y(((layer_positions.y + layer_positions.height) / scaled_height_ratio) + y_diff);
		set_invalidation_start_x((layer_positions.x / scaled_width_ratio) + x_diff);
		set_invalidation_start_y((layer_positions.y / scaled_height_ratio) + y_diff);

		return TRUE;
	}

	return TRUE;
}
