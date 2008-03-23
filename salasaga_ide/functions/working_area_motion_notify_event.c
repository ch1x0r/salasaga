/*
 * $Id$
 *
 * Salasaga: Detect when the user moves a pressed down mouse button on the drawing area 
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


gboolean working_area_motion_notify_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	// Local variables
	GdkModifierType		button_state;				// Mouse button states
	slide				*current_slide_data;		// Alias to make things easier
	gint				height;
	layer				*layer_data;
	GtkWidget			*list_widget;				// Alias to the timeline widget to make things easier
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
	gchar				*selected_row;				// Holds the number of the row that is selected
	GtkTreePath			*tmp_path;					// Temporary path
	gint				width;
	gfloat				x_diff;						// The X distance the object was dragged, after scaling
	gfloat				y_diff;						// The Y distance the object was dragged, after scaling


	// Find out where the mouse is positioned, and which buttons and modifier keys are down (active)
	gdk_window_get_pointer(event->window, &mouse_x, &mouse_y, &button_state);

	// If we're creating a new highlight layer, draw a bounding box
	if (TYPE_HIGHLIGHT == new_layer_selected)
	{
		// Draw the updated bounding box
		draw_bounding_box(stored_x, stored_y, mouse_x, mouse_y);
		return TRUE;
	}

	// If we're already aware of a mouse drag operation going on, then draw a bounding box
	if (TRUE == mouse_dragging)
	{
		// Initialise some things
		current_slide_data = current_slide->data;
		list_widget = current_slide_data->timeline_widget;

		// Calculate the height and width scaling values for the main drawing area at its present size
		scaled_height_ratio = (gfloat) project_height / (gfloat) main_drawing_area->allocation.height;
		scaled_width_ratio = (gfloat) project_width / (gfloat) main_drawing_area->allocation.width;

		// Determine which layer is selected in the timeline
		tmp_path = gtk_tree_path_new();
		gtk_tree_view_get_cursor(GTK_TREE_VIEW(list_widget), &tmp_path, NULL);
		selected_row = gtk_tree_path_to_string(tmp_path);

		// Get its present X and Y offsets
		current_slide_data->layers = g_list_first(current_slide_data->layers);
		layer_data = g_list_nth_data(current_slide_data->layers, atoi(selected_row));
		switch (layer_data->object_type)
		{
			case TYPE_EMPTY:
				// We can't drag an empty layer, so reset things and return
				mouse_dragging = FALSE;
				stored_x = -1;
				stored_y = -1;
				return TRUE;

			case TYPE_HIGHLIGHT:
				present_x = layer_data->x_offset_start;
				present_y = layer_data->y_offset_start;
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
				present_x = layer_data->x_offset_start - 12;
				present_y = layer_data->y_offset_start - 4;
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

	// * To get here, this must be the first time we've heard of this particular mouse drag *

	// Check if the primary mouse button is down
	if (GDK_BUTTON1_MASK & button_state)
	{
		// It is, so we take notice of the mouse drag operation
		mouse_dragging = TRUE;

		// Store the mouse coordinates so we know where to drag from
		stored_x = event->x;
		stored_y = event->y;

		// Reset the invalidation area
		invalidation_end_x = event->x;
		invalidation_end_y = event->y;
		invalidation_start_x = event->x - 1;
		invalidation_start_y = event->y - 1;
	}

	return TRUE;
}
