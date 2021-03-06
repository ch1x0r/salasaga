/*
 * $Id$
 *
 * Salasaga: Detect when the user releases the mouse button on the drawing area
 *
 * Copyright (C) 2005-2010 Digital Distribution Global Training Solutions Pty. Ltd.
 * <justin@salasaga.org>
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
#include "../global_functions.h"
#include "../dialog/display_warning.h"
#include "../film_strip/film_strip_create_thumbnail.h"
#include "../layer/layer_duplicate.h"
#include "../layer/layer_new_highlight_inner.h"
#include "../layer/layer_new_image_inner.h"
#include "../layer/layer_new_mouse_inner.h"
#include "../layer/layer_new_text_inner.h"
#include "../preference/project_preferences.h"
#include "../undo_redo/undo_functions.h"
#include "../time_line/time_line_get_selected_layer_num.h"
#include "draw_workspace.h"


gboolean working_area_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	// Local variables
	GdkModifierType		button_state;
	gint				height;
	layer				*layer_data;
	GList				*layer_pointer;				// Used briefly to hold a pointer to this layer
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
	GtkWidget			*temp_widget;				// Temporarily holds a pointer to the main drawing area widget
	slide				*this_slide_data;			// Alias to make things easier
	undo_history_data	*undo_item_data = NULL;		// Memory structure undo history items are created in
	gint				width;
	gint				x_change;					// The X distance the layer object moves from start to finish
	gint				y_change;					// The Y distance the layer object moves from start to finish
	gfloat				x_diff;						// The X distance the object was dragged, after scaling
	gfloat				y_diff;						// The Y distance the object was dragged, after scaling


	// Only do this function if we have a front store available and a project loaded
	if ((NULL == get_front_store()) || (FALSE == get_project_active()))
	{
		return TRUE;
	}

	// Find out where the mouse is positioned, and which buttons and modifier keys are down (active)
	gdk_window_get_pointer(event->window, &mouse_x, &mouse_y, &button_state);

	// Calculate the height and width scaling values for the main drawing area at its present size
	gdk_drawable_get_size(GDK_PIXMAP(get_front_store()), &pixmap_width, &pixmap_height);
	scaled_height_ratio = (gfloat) get_project_height() / (gfloat) pixmap_height;
	scaled_width_ratio = (gfloat) get_project_width() / (gfloat) pixmap_width;

	// Work out where the mouse is positioned
	project_x_position = mouse_x * scaled_width_ratio;
	project_y_position = mouse_y * scaled_height_ratio;

	// Code to run if this button release is for the creation of a new highlight layer
	if (TYPE_HIGHLIGHT == get_new_layer_selected())
	{
		// Reset the new layer toggle
		set_new_layer_selected(TYPE_NONE);

		// Ask the user for the rest of the required details
		layer_new_highlight_inner(mouse_x, mouse_y);  // Note that this one gets the unscaled mouse positions
		return TRUE;
	}

	// Code to run if this button release is for the creation of a new image layer
	if (TYPE_GDK_PIXBUF == get_new_layer_selected())
	{
		// Reset the new layer toggle
		set_new_layer_selected(TYPE_NONE);

		// Ask the user for the rest of the required details
		layer_new_image_inner(project_x_position, project_y_position);
		return TRUE;
	}

	// Code to run if this button release is for the creation of a new mouse layer
	if (TYPE_MOUSE_CURSOR == get_new_layer_selected())
	{
		// Reset the new layer toggle
		set_new_layer_selected(TYPE_NONE);

		// Ask the user for the rest of the required details
		layer_new_mouse_inner(project_x_position, project_y_position);
		return TRUE;
	}

	// Code to run if this button release is for the creation of a new text layer
	if (TYPE_TEXT == get_new_layer_selected())
	{
		// Reset the new layer toggle
		set_new_layer_selected(TYPE_NONE);

		// Ask the user for the rest of the required details
		layer_new_text_inner(project_x_position, project_y_position);
		return TRUE;
	}

	// If this release matches an end point moving operation, we process it
	if (END_POINTS_INACTIVE != get_end_point_status())
	{
		// Redraw the workspace
		draw_workspace();

		// Tell (force) the window system to redraw the working area *immediately*
		temp_widget = get_main_drawing_area();
		gtk_widget_draw(GTK_WIDGET(temp_widget), &temp_widget->allocation);  // Yes, this is deprecated, but it *works*

		// Recreate the slide thumbnail
		film_strip_create_thumbnail(get_current_slide_data());

		// Use the status bar to give further feedback to the user
		if (END_POINTS_START_ACTIVE == get_end_point_status())
			gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), _(" Layer start point moved"));
		if (END_POINTS_END_ACTIVE == get_end_point_status())
			gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), _(" Layer end point moved"));
		gdk_flush();

		// Reset the end point status switch and related info
		set_end_point_status(END_POINTS_INACTIVE);
		set_stored_x(-1);
		set_stored_y(-1);

		// Set the changes made variable
		set_changes_made(TRUE);

		return TRUE;
	}

	// If this release matches the end of a layer resize operation, we process it
	if (FALSE != (RESIZE_HANDLES_RESIZING & get_resize_handles_status()))
	{
		// Initialise some things
		this_slide_data = get_current_slide_data();

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
		x_diff = (mouse_x - get_stored_x()) * scaled_width_ratio;
		y_diff = (mouse_y - get_stored_y()) * scaled_height_ratio;

		// Work out the new size for the layer
		switch (get_resize_handles_status() & RESIZE_HANDLES_RESIZING_ALL)
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

		// Create an undo history item and store the existing layer data in it
		undo_item_data = g_new0(undo_history_data, 1);
		undo_item_data->layer_data_old = layer_duplicate(layer_data);
		undo_item_data->position_new = selected_row;
		undo_item_data->position_old = selected_row;
		undo_item_data->slide_data = get_current_slide_data();

		// Duplicate the present layer and use that instead
		layer_pointer = g_list_nth(this_slide_data->layers, selected_row);
		layer_pointer->data = layer_duplicate(layer_data);
		layer_data = layer_pointer->data;

		// Calculate the new layer width and height
		((layer_highlight *) layer_data->object_data)->width = width = CLAMP(onscreen_right - onscreen_left,
				get_valid_fields_min_value(HIGHLIGHT_WIDTH), get_project_width() - 2);
		((layer_highlight *) layer_data->object_data)->height = height = CLAMP(onscreen_bottom - onscreen_top,
				get_valid_fields_min_value(HIGHLIGHT_HEIGHT), get_project_height() - 2);

		// Bounds check the starting x offset, then update the object with the new value
		layer_data->x_offset_start = CLAMP(onscreen_left, 1, get_project_width() - width - 2);

		// Bounds check the finishing x offset, then update the object with the new value
		layer_data->x_offset_finish = CLAMP(onscreen_left + x_change, 1, get_project_width() - width - 2);

		// Bounds check the starting y offset, then update the object with the new value
		layer_data->y_offset_start = CLAMP(onscreen_top, 1, get_project_height() - height - 2);

		// Bounds check the finishing y offset, then update the object with the new value
		layer_data->y_offset_finish = CLAMP(onscreen_top + y_change, 1, get_project_height() - height - 2);

		// Store the undo item
		undo_item_data->layer_data_new = layer_duplicate(layer_data);
		undo_history_add_item(UNDO_CHANGE_LAYER, undo_item_data, TRUE);

		// Redraw the workspace
		draw_workspace();

		// Tell (force) the window system to redraw the working area *immediately*
		temp_widget = get_main_drawing_area();
		gtk_widget_draw(GTK_WIDGET(temp_widget), &temp_widget->allocation);  // Yes, this is deprecated, but it *works*

		// Recreate the slide thumbnail
		film_strip_create_thumbnail(get_current_slide_data());

		// Reset the resize switch and related info
		set_resize_handles_status(RESIZE_HANDLES_WAITING);
		set_stored_x(-1);
		set_stored_y(-1);

		// Set the changes made variable
		set_changes_made(TRUE);

		// Use the status bar to give further feedback to the user
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), _(" Layer resized"));
		gdk_flush();

		return TRUE;
	}

	// If this release matches the end of a mouse drag operation, we process it
	if (TRUE == get_mouse_dragging())
	{
		// Initialise some things
		this_slide_data = get_current_slide_data();

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
					set_mouse_dragging(FALSE);
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
					g_string_printf(message, "%s ED32: %s", _("Error"), _("Unknown layer type."));
					display_warning(message->str);
					g_string_free(message, TRUE);

					return TRUE;  // Unknown layer type, so no idea how to extract the needed data for the next code
			}

			// Create an undo history item and store the existing layer data in it
			undo_item_data = g_new0(undo_history_data, 1);
			undo_item_data->layer_data_old = layer_duplicate(layer_data);
			undo_item_data->position_new = selected_row;
			undo_item_data->position_old = selected_row;
			undo_item_data->slide_data = get_current_slide_data();

			// Calculate the distance the object has been dragged
			x_diff = (mouse_x - get_stored_x()) * scaled_width_ratio;
			y_diff = (mouse_y - get_stored_y()) * scaled_height_ratio;

			// Bounds check the starting x offset, then update the object with the new value
			layer_data->x_offset_start = layer_data->x_offset_start + x_diff;

			// Bounds check the finishing x offset, then update the object with the new value
			layer_data->x_offset_finish = layer_data->x_offset_finish + x_diff;

			// Bounds check the starting y offset, then update the object with the new value
			layer_data->y_offset_start = layer_data->y_offset_start + y_diff;

			// Bounds check the finishing y offset, then update the object with the new value
			layer_data->y_offset_finish = layer_data->y_offset_finish + y_diff;

			// Store the undo item
			undo_item_data->layer_data_new = layer_duplicate(layer_data);
			undo_history_add_item(UNDO_CHANGE_LAYER, undo_item_data, TRUE);

			// Redraw the workspace
			draw_workspace();

			// Tell (force) the window system to redraw the working area *immediately*
			temp_widget = get_main_drawing_area();
			gtk_widget_draw(GTK_WIDGET(temp_widget), &temp_widget->allocation);  // Yes, this is deprecated, but it *works*

			// Recreate the slide thumbnail
			film_strip_create_thumbnail(get_current_slide_data());

			// Reset the mouse drag switch and related info
			set_mouse_dragging(FALSE);
			set_stored_x(-1);
			set_stored_y(-1);

			// Set the changes made variable
			set_changes_made(TRUE);

			// Use the status bar to give further feedback to the user
			gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), _(" Layer moved"));
			gdk_flush();
		}
	}

	// Redraw the workspace
	draw_workspace();

	return TRUE;
}
