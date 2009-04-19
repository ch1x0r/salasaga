/*
 * $Id$
 *
 * Salasaga: Function called when the user presses the mouse button on the drawing area
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
#include "../layer/layer_edit.h"
#include "../widgets/time_line/time_line_get_selected_layer_num.h"
#include "../widgets/time_line/time_line_set_selected_layer_num.h"
#include "calculate_object_boundaries.h"
#include "detect_collisions.h"
#include "draw_bounding_box.h"
#include "draw_handle_box.h"


gboolean working_area_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	// Local variables
	gint				box_height;					// Height of the bounding box
	gint				box_width;					// Width of the bounding box
	GList				*collision_list = NULL;
	guint				count_int;
	gint				finish_x;					// X position at the layer objects finish time
	gint				finish_y;					// Y position at the layer objects finish time
	GList				*layer_pointer;
	GString				*message;					// Used to construct message strings
	guint				num_collisions;
	gint				onscreen_bottom;			// Y coordinate of bounding box bottom
	gint				onscreen_left;				// X coordinate of bounding box left
	gint				onscreen_right;				// X coordinate of bounding box right
	gint				onscreen_top;				// Y coordinate of bounding box top
	gint				pixmap_height;				// Height of the front store
	gint				pixmap_width;				// Width of the front store
	gfloat				scaled_height_ratio;		// Used to calculate a vertical scaling ratio
	gfloat				scaled_width_ratio;			// Used to calculate a horizontal scaling ratio
	gint				selected_row;				// Holds the number of the row that is selected
	gint				start_x;					// X position at the layer objects start time
	gint				start_y;					// Y position at the layer objects start time
	layer 				*this_layer_data;			// Pointer to the data for the selected layer
	slide				*this_slide_data;			// Alias to make things easier
	guint				tmp_int;					// Temporary integer


	// Only do this function if we have a front store available and a project loaded
	if ((NULL == front_store) || (FALSE == project_active))
	{
		return TRUE;
	}

	// Initialise some things
	this_slide_data = current_slide->data;
	gdk_drawable_get_size(GDK_PIXMAP(front_store), &pixmap_width, &pixmap_height);

	// Check for primary mouse button click
	if (1 != event->button)
	{
		// Not a primary mouse click, so return
		return TRUE;
	}

	// Reset the mouse drag toggle
	mouse_dragging = FALSE;

	// Check if this was a double mouse click.  If it was, open an edit dialog
	if (GDK_2BUTTON_PRESS == event->type)
	{
		// Open an edit dialog
		layer_edit();

		return TRUE;
	}

	// Check if this was a triple mouse click.  If it was, ignore it
	if (GDK_3BUTTON_PRESS == event->type)
	{
		return TRUE;
	}

	// If we're presently creating a new highlight layer, store the mouse coordinates
	if (TYPE_HIGHLIGHT == new_layer_selected)
	{
		// Save the mouse coordinates
		stored_x = event->x;
		stored_y = event->y;

		// Reset the invalidation area
		invalidation_end_x = event->x;
		invalidation_end_y = event->y;
		invalidation_start_x = event->x - 1;
		invalidation_start_y = event->y - 1;

		return TRUE;
	}

	// If the user has clicked on the start or end points for the selected layer, we
	// don't want to do the collision detection below that changes layers
	if (END_POINTS_INACTIVE == end_point_status)
	{
		// * Check if the user is clicking on the layer start or end points *

		// Calculate the height and width scaling values for the main drawing area at its present size
		scaled_height_ratio = (gfloat) project_height / (gfloat) pixmap_height;
		scaled_width_ratio = (gfloat) project_width / (gfloat) pixmap_width;

		// Determine which row is selected in the time line
		selected_row = time_line_get_selected_layer_num(this_slide_data->timeline_widget);
		layer_pointer = g_list_first(this_slide_data->layers);
		this_layer_data = g_list_nth_data(layer_pointer, selected_row);

		// Calculate start and end points
		finish_x = (this_layer_data->x_offset_finish / scaled_width_ratio) + END_POINT_HORIZONTAL_OFFSET;
		finish_y = (this_layer_data->y_offset_finish / scaled_height_ratio) + END_POINT_VERTICAL_OFFSET;
		start_x = (this_layer_data->x_offset_start / scaled_width_ratio) + END_POINT_HORIZONTAL_OFFSET;
		start_y = (this_layer_data->y_offset_start / scaled_height_ratio) + END_POINT_VERTICAL_OFFSET;

		// Is the user clicking on an end point?
		if (((event->x >= start_x)							// Start point
			&& (event->x <= start_x + END_POINT_WIDTH)
			&& (event->y >= start_y)
			&& (event->y <= start_y + END_POINT_HEIGHT)) ||
			((event->x >= finish_x)							// End point
			&& (event->x <= finish_x + END_POINT_WIDTH)
			&& (event->y >= finish_y)
			&& (event->y <= finish_y + END_POINT_HEIGHT)))
		{
			// Retrieve the layer size information
			switch (this_layer_data->object_type)
			{
				case TYPE_EMPTY:
					// We can't drag an empty layer, so reset things and return
					mouse_dragging = FALSE;
					end_point_status = END_POINTS_INACTIVE;
					stored_x = -1;
					stored_y = -1;
					return TRUE;

				case TYPE_HIGHLIGHT:
					box_width = ((layer_highlight *) this_layer_data->object_data)->width;
					box_height = ((layer_highlight *) this_layer_data->object_data)->height;
					break;

				case TYPE_GDK_PIXBUF:
					// If this is the background layer, then we ignore it
					if (TRUE == this_layer_data->background)
					{
						mouse_dragging = FALSE;
						end_point_status = END_POINTS_INACTIVE;
						stored_x = -1;
						stored_y = -1;
						return TRUE;
					}

					// No it's not, so process it
					box_width = ((layer_image *) this_layer_data->object_data)->width;
					box_height = ((layer_image *) this_layer_data->object_data)->height;
					break;

				case TYPE_MOUSE_CURSOR:
					box_width = ((layer_mouse *) this_layer_data->object_data)->width;
					box_height = ((layer_mouse *) this_layer_data->object_data)->height;
					break;

				case TYPE_TEXT:
					box_width = ((layer_text *) this_layer_data->object_data)->rendered_width;
					box_height = ((layer_text *) this_layer_data->object_data)->rendered_height;
					break;

				default:
					message = g_string_new(NULL);
					g_string_printf(message, "%s ED377: %s", _("Error"), _("Unknown layer type."));
					display_warning(message->str);
					g_string_free(message, TRUE);

					return TRUE;  // Unknown layer type, so no idea how to extract the needed data for the next code
			}

			// Work out the bounding box boundaries (scaled)
			if ((event->x >= start_x)							// Left
				&& (event->x <= start_x + END_POINT_WIDTH)		// Right
				&& (event->y >= start_y)						// Top
				&& (event->y <= start_y + END_POINT_HEIGHT))	// Bottom
			{
				// Start point clicked
				onscreen_left = this_layer_data->x_offset_start / scaled_width_ratio;
				onscreen_top = this_layer_data->y_offset_start / scaled_width_ratio;;
				onscreen_right = (this_layer_data->x_offset_start + box_width) / scaled_height_ratio;
				onscreen_bottom = (this_layer_data->y_offset_start + box_height) / scaled_height_ratio;
			} else
			{
				// End point clicked
				onscreen_left = this_layer_data->x_offset_finish / scaled_width_ratio;
				onscreen_top = this_layer_data->y_offset_finish / scaled_width_ratio;;
				onscreen_right = (this_layer_data->x_offset_finish + box_width) / scaled_height_ratio;
				onscreen_bottom = (this_layer_data->y_offset_finish + box_height) / scaled_height_ratio;
			}

			// Ensure the bounding box doesn't go out of bounds
			onscreen_left = CLAMP(onscreen_left, 2, pixmap_width - 2);
			onscreen_top = CLAMP(onscreen_top, 2, pixmap_height - 2);
			onscreen_right = CLAMP(onscreen_right, 2, pixmap_width - 2);
			onscreen_bottom = CLAMP(onscreen_bottom, 2, pixmap_height - 2);

			// Draw a bounding box onscreen
			draw_bounding_box(onscreen_left, onscreen_top, onscreen_right, onscreen_bottom);

			// End point clicked, so we return in order to avoid the collision detection
			return TRUE;
		}
	}

	// * Do collision detection here to determine if the user has clicked on a layer's object *

	this_slide_data = current_slide->data;
	calculate_object_boundaries();
	collision_list = detect_collisions(collision_list, event->x, event->y);
	if (NULL == collision_list)
	{
		// If there was no collision, then select the background layer
		time_line_set_selected_layer_num(this_slide_data->timeline_widget, this_slide_data->num_layers - 1);  // *Needs* the -1, don't remove

		// Clear any existing handle box
		gdk_draw_drawable(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(main_drawing_area->style->fg_gc[GTK_WIDGET_STATE(main_drawing_area)]),
				GDK_PIXMAP(front_store), 0, 0, 0, 0, -1, -1);

		// Reset the stored mouse coordinates
		stored_x = -1;
		stored_y = -1;

		// Free the memory allocated during the collision detection
		g_list_free(collision_list);
		collision_list = NULL;

		return TRUE;
	}

	// * To get here there must have been at least one collision *

	// Save the mouse coordinates
	stored_x = event->x;
	stored_y = event->y;

	// Determine which layer the user has selected in the timeline
	selected_row = time_line_get_selected_layer_num(this_slide_data->timeline_widget);

	// Is the presently selected layer in the collision list?
	collision_list = g_list_first(collision_list);
	num_collisions = g_list_length(collision_list);
	for (count_int = 0; count_int < num_collisions; count_int++)
	{
		collision_list = g_list_first(collision_list);
		collision_list = g_list_nth(collision_list, count_int);
		layer_pointer = g_list_first(this_slide_data->layers);
		tmp_int = g_list_position(layer_pointer, ((boundary_box *) collision_list->data)->layer_ptr);
		if (tmp_int == selected_row)
		{
			// Return if the presently selected row is in the collision list, as we don't want to change our selected layer
			return TRUE;
		}
	}

	// * To get here, the presently selected layer wasn't in the collision list *

	// The presently selected row is not in the collision list, so move the selection row to the first collision
	collision_list = g_list_first(collision_list);
	selected_row = g_list_position(this_slide_data->layers, ((boundary_box *) collision_list->data)->layer_ptr);
	time_line_set_selected_layer_num(this_slide_data->timeline_widget, selected_row);

	// Draw a handle box around the new selected object
	draw_handle_box();

	// Free the memory allocated during the collision detection
	g_list_free(collision_list);
	collision_list = NULL;

	return TRUE;
}
