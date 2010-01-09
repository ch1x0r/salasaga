/*
 * $Id$
 *
 * Salasaga: Detect when the user releases the mouse button on the drawing area
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
#include "../film_strip/film_strip_create_thumbnail.h"
#include "../layer/layer_new_highlight_inner.h"
#include "../layer/layer_new_image_inner.h"
#include "../layer/layer_new_mouse_inner.h"
#include "../layer/layer_new_text_inner.h"
#include "../undo_redo/undo_functions.h"
#include "../widgets/time_line/time_line_get_selected_layer_num.h"
#include "draw_workspace.h"


gboolean working_area_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	// Local variables
	GdkModifierType		button_state;
	gint				height;
	layer				*layer_data;
	GString				*message;					// Used to construct message strings
	gint				mouse_x;
	gint				mouse_y;
	gint				onscreen_bottom;			// New Y coordinate of layer
	gint				onscreen_left;				// New X coordinate of layer
	gint				onscreen_right;				// New X coordinate of layer
	gint				onscreen_top;				// New Y coordinate of layer
	gint				pixmap_height;				// Height of the front store
	gint				pixmap_width;				// Width of the front store
	gint				project_x_position;			// X position in the project image
	gint				project_y_position;			// Y position in the project image
	gfloat				scaled_height_ratio;		// Used to calculate a vertical scaling ratio
	gfloat				scaled_width_ratio;			// Used to calculate a horizontal scaling ratio
	gint				selected_row;				// Holds the number of the row that is selected
	guint				swap_value;					// Temporary value used when swapping border positions
	slide				*this_slide_data;			// Alias to make things easier
	undo_data			*undo_item_data = NULL;		// Memory structure undo history items are created in
	gint				width;
	gint				x_change;					// The X distance the layer object moves from start to finish
	gint				y_change;					// The Y distance the layer object moves from start to finish
	gfloat				x_diff;						// The X distance the object was dragged, after scaling
	gfloat				y_diff;						// The Y distance the object was dragged, after scaling


	// Only do this function if we have a front store available and a project loaded
	if ((NULL == front_store) || (FALSE == project_active))
	{
		return TRUE;
	}

	// Find out where the mouse is positioned, and which buttons and modifier keys are down (active)
	gdk_window_get_pointer(event->window, &mouse_x, &mouse_y, &button_state);

	// Calculate the height and width scaling values for the main drawing area at its present size
	gdk_drawable_get_size(GDK_PIXMAP(front_store), &pixmap_width, &pixmap_height);
	scaled_height_ratio = (gfloat) project_height / (gfloat) pixmap_height;
	scaled_width_ratio = (gfloat) project_width / (gfloat) pixmap_width;

	// Work out where the mouse is positioned
	project_x_position = mouse_x * scaled_width_ratio;
	project_y_position = mouse_y * scaled_height_ratio;

	// Code to run if this button release is for the creation of a new highlight layer
	if (TYPE_HIGHLIGHT == new_layer_selected)
	{
		// Reset the new layer toggle
		new_layer_selected = TYPE_NONE;

		// Ask the user for the rest of the required details
		layer_new_highlight_inner(mouse_x, mouse_y);  // Note that this one gets the unscaled mouse positions
		return TRUE;
	}

	// Code to run if this button release is for the creation of a new image layer
	if (TYPE_GDK_PIXBUF == new_layer_selected)
	{
		// Reset the new layer toggle
		new_layer_selected = TYPE_NONE;

		// Ask the user for the rest of the required details
		layer_new_image_inner(project_x_position, project_y_position);
		return TRUE;
	}

	// Code to run if this button release is for the creation of a new mouse layer
	if (TYPE_MOUSE_CURSOR == new_layer_selected)
	{
		// Reset the new layer toggle
		new_layer_selected = TYPE_NONE;

		// Ask the user for the rest of the required details
		layer_new_mouse_inner(project_x_position, project_y_position);
		return TRUE;
	}

	// Code to run if this button release is for the creation of a new text layer
	if (TYPE_TEXT == new_layer_selected)
	{
		// Reset the new layer toggle
		new_layer_selected = TYPE_NONE;

		// Ask the user for the rest of the required details
		layer_new_text_inner(project_x_position, project_y_position);
		return TRUE;
	}

	// If this release matches an end point moving operation, we process it
	if (END_POINTS_INACTIVE != end_point_status)
	{
		// Redraw the workspace
		draw_workspace();

		// Tell (force) the window system to redraw the working area *immediately*
		gtk_widget_draw(GTK_WIDGET(main_drawing_area), &main_drawing_area->allocation);  // Yes, this is deprecated, but it *works*

		// Recreate the slide thumbnail
		film_strip_create_thumbnail((slide *) current_slide->data);

		// Use the status bar to give further feedback to the user
		if (END_POINTS_START_ACTIVE == end_point_status)
			gtk_progress_bar_set_text(GTK_PROGRESS_BAR(status_bar), _(" Layer start point moved"));
		if (END_POINTS_END_ACTIVE == end_point_status)
			gtk_progress_bar_set_text(GTK_PROGRESS_BAR(status_bar), _(" Layer end point moved"));
		gdk_flush();

		// Reset the end point status switch and related info
		end_point_status = END_POINTS_INACTIVE;
		stored_x = -1;
		stored_y = -1;

		// Set the changes made variable
		changes_made = TRUE;

		return TRUE;
	}

	// If this release matches the end of a layer resize operation, we process it
	if (FALSE != (RESIZE_HANDLES_RESIZING & resize_handles_status))
	{
		// Initialise some things
		this_slide_data = current_slide->data;

		// Determine which layer is selected in the timeline
		selected_row = time_line_get_selected_layer_num(this_slide_data->timeline_widget);

		// Get its present X and Y offsets
		this_slide_data->layers = g_list_first(this_slide_data->layers);
		layer_data = g_list_nth_data(this_slide_data->layers, selected_row);
		width = ((layer_highlight *) layer_data->object_data)->width;
		height = ((layer_highlight *) layer_data->object_data)->height;
		x_change = layer_data->x_offset_finish - layer_data->x_offset_start;
		y_change = layer_data->y_offset_finish - layer_data->y_offset_start;

		// Calculate the distance the mouse was moved
		x_diff = (mouse_x - stored_x) * scaled_width_ratio;
		y_diff = (mouse_y - stored_y) * scaled_height_ratio;

		// Work out the new size for the layer
		switch (resize_handles_status & RESIZE_HANDLES_RESIZING_ALL)
		{
			case RESIZE_HANDLES_RESIZING_TL:
				// Top left resize
				onscreen_left = layer_data->x_offset_start + x_diff;
				onscreen_top = layer_data->y_offset_start + y_diff;
				onscreen_right = layer_data->x_offset_start + width;
				onscreen_bottom = layer_data->y_offset_start + height;
				break;

			case RESIZE_HANDLES_RESIZING_TM:
				// Top middle resize
				onscreen_left = layer_data->x_offset_start;
				onscreen_top = layer_data->y_offset_start + y_diff;
				onscreen_right = layer_data->x_offset_start + width;
				onscreen_bottom = layer_data->y_offset_start + height;
				break;

			case RESIZE_HANDLES_RESIZING_TR:
				// Top right resize
				onscreen_left = layer_data->x_offset_start;
				onscreen_top = layer_data->y_offset_start + y_diff;
				onscreen_right = layer_data->x_offset_start + width + x_diff;
				onscreen_bottom = layer_data->y_offset_start + height;
				break;

			case RESIZE_HANDLES_RESIZING_RM:
				// Middle right resize
				onscreen_left = layer_data->x_offset_start;
				onscreen_top = layer_data->y_offset_start;
				onscreen_right = layer_data->x_offset_start + width + x_diff;
				onscreen_bottom = layer_data->y_offset_start + height;
				break;

			case RESIZE_HANDLES_RESIZING_BR:
				// Bottom right resize
				onscreen_left = layer_data->x_offset_start;
				onscreen_top = layer_data->y_offset_start;
				onscreen_right = layer_data->x_offset_start + width + x_diff;
				onscreen_bottom = layer_data->y_offset_start + height + y_diff;
				break;

			case RESIZE_HANDLES_RESIZING_BM:
				// Bottom middle resize
				onscreen_left = layer_data->x_offset_start;
				onscreen_top = layer_data->y_offset_start;
				onscreen_right = layer_data->x_offset_start + width;
				onscreen_bottom = layer_data->y_offset_start + height + y_diff;
				break;

			case RESIZE_HANDLES_RESIZING_BL:
				// Bottom left resize
				onscreen_left = layer_data->x_offset_start + x_diff;
				onscreen_top = layer_data->y_offset_start;
				onscreen_right = layer_data->x_offset_start + width;
				onscreen_bottom = layer_data->y_offset_start + height + y_diff;
				break;

			case RESIZE_HANDLES_RESIZING_LM:
				// Left middle resize
				onscreen_left = layer_data->x_offset_start + x_diff;
				onscreen_top = layer_data->y_offset_start;
				onscreen_right = layer_data->x_offset_start + width;
				onscreen_bottom = layer_data->y_offset_start + height;
				break;

			default:
				message = g_string_new(NULL);
				g_string_printf(message, "%s ED296: %s", _("Error"), _("Unknown resizing direction."));
				display_warning(message->str);
				g_string_free(message, TRUE);
				return FALSE;
		}

		// Swap left and right around if we need to
		if (onscreen_left > onscreen_right)
		{
			swap_value = onscreen_left;
			onscreen_left = onscreen_right;
			onscreen_right = swap_value;
		}

		// Swap top and bottom around if we need to
		if (onscreen_top > onscreen_bottom)
		{
			swap_value = onscreen_top;
			onscreen_top = onscreen_bottom;
			onscreen_bottom = swap_value;
		}

		// Save the old layer sizes in the undo history
		undo_item_data = g_new0(undo_data, 1);
		undo_item_data->layer_pointer = layer_data;
		undo_item_data->old_highlight_height = ((layer_highlight *) layer_data->object_data)->height;
		undo_item_data->old_highlight_width = ((layer_highlight *) layer_data->object_data)->width;
		undo_add_item(UNDO_CHANGE_HIGHLIGHT_SIZE, undo_item_data);

		// Calculate the new layer width and height
		((layer_highlight *) layer_data->object_data)->width = width = CLAMP(onscreen_right - onscreen_left,
				valid_fields[HIGHLIGHT_WIDTH].min_value, project_width - 2);
		((layer_highlight *) layer_data->object_data)->height = height = CLAMP(onscreen_bottom - onscreen_top,
				valid_fields[HIGHLIGHT_HEIGHT].min_value, project_height - 2);

		// Bounds check the starting x offset, then update the object with the new value
		layer_data->x_offset_start = CLAMP(onscreen_left, 1, project_width - width - 2);

		// Bounds check the finishing x offset, then update the object with the new value
		layer_data->x_offset_finish = CLAMP(onscreen_left + x_change, 1, project_width - width - 2);

		// Bounds check the starting y offset, then update the object with the new value
		layer_data->y_offset_start = CLAMP(onscreen_top, 1, project_height - height - 2);

		// Bounds check the finishing y offset, then update the object with the new value
		layer_data->y_offset_finish = CLAMP(onscreen_top + y_change, 1, project_height - height - 2);

		// Redraw the workspace
		draw_workspace();

		// Tell (force) the window system to redraw the working area *immediately*
		gtk_widget_draw(GTK_WIDGET(main_drawing_area), &main_drawing_area->allocation);  // Yes, this is deprecated, but it *works*

		// Recreate the slide thumbnail
		film_strip_create_thumbnail((slide *) current_slide->data);

		// Reset the resize switch and related info
		resize_handles_status = RESIZE_HANDLES_WAITING;
		stored_x = -1;
		stored_y = -1;

		// Set the changes made variable
		changes_made = TRUE;

		// Use the status bar to give further feedback to the user
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(status_bar), _(" Layer resized"));
		gdk_flush();

		return TRUE;
	}

	// If this release matches the end of a mouse drag operation, we process it
	if (TRUE == mouse_dragging)
	{
		// Initialise some things
		this_slide_data = current_slide->data;

		// Check for primary mouse button release
		if (1 == event->button)
		{
			// Determine which layer is selected in the timeline
			selected_row = time_line_get_selected_layer_num(this_slide_data->timeline_widget);

			// Get its present X and Y offsets
			this_slide_data->layers = g_list_first(this_slide_data->layers);
			layer_data = g_list_nth_data(this_slide_data->layers, selected_row);
			switch (layer_data->object_type)
			{
				case TYPE_EMPTY:
					// We can't drag an empty layer, so reset things and return
					mouse_dragging = FALSE;
					stored_x = -1;
					stored_y = -1;
					return TRUE;

				case TYPE_HIGHLIGHT:
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
					g_string_printf(message, "%s ED32: %s", _("Error"), _("Unknown layer type."));
					display_warning(message->str);
					g_string_free(message, TRUE);

					return TRUE;  // Unknown layer type, so no idea how to extract the needed data for the next code
			}

			// Calculate the distance the object has been dragged
			x_diff = (mouse_x - stored_x) * scaled_width_ratio;
			y_diff = (mouse_y - stored_y) * scaled_height_ratio;

			// Bounds check the starting x offset, then update the object with the new value
			layer_data->x_offset_start = layer_data->x_offset_start + x_diff;

			// Bounds check the finishing x offset, then update the object with the new value
			layer_data->x_offset_finish = layer_data->x_offset_finish + x_diff;

			// Bounds check the starting y offset, then update the object with the new value
			layer_data->y_offset_start = layer_data->y_offset_start + y_diff;

			// Bounds check the finishing y offset, then update the object with the new value
			layer_data->y_offset_finish = layer_data->y_offset_finish + y_diff;

			// Redraw the workspace
			draw_workspace();

			// Tell (force) the window system to redraw the working area *immediately*
			gtk_widget_draw(GTK_WIDGET(main_drawing_area), &main_drawing_area->allocation);  // Yes, this is deprecated, but it *works*

			// Recreate the slide thumbnail
			film_strip_create_thumbnail((slide *) current_slide->data);

			// Reset the mouse drag switch and related info
			mouse_dragging = FALSE;
			stored_x = -1;
			stored_y = -1;

			// Set the changes made variable
			changes_made = TRUE;

			// Use the status bar to give further feedback to the user
			gtk_progress_bar_set_text(GTK_PROGRESS_BAR(status_bar), _(" Layer moved"));
			gdk_flush();
		}
	}

	// Redraw the workspace
	draw_workspace();

	return TRUE;
}
