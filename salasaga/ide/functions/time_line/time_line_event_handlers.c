/*
 * $Id: time_line_event_handlers.c 2659 2010-05-27 15:01:48Z allupaku $
 *
 * Salasaga: Event Handling functions for the time line widget
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
#include <gtk/gtk.h>
#include "../../salasaga_types.h"
#include "../global_functions.h"
#include "../dialog/display_warning.h"
#include "../layer/layer_duplicate.h"
#include "../layer/layer_edit.h"
#include "../other/widget_focus.h"
#include "../undo_redo/undo_functions.h"
#include "../working_area/draw_handle_box.h"
#include "../working_area/draw_layer_start_and_end_points.h"
#include "../working_area/draw_workspace.h"
#include "time_line.h"
#include "time_line_get_cursor_position.h"
#include "time_line_get_left_border_width.h"
#include "time_line_get_type.h"
#include "time_line_internal_draw_guide_line.h"
#include "time_line_set_cursor_position.h"
#include "time_line_set_selected_layer_num.h"
#include "time_line_internal_draw_guide_line.h"
#include "time_line_internal_draw_layer_duration.h"
#include "time_line_internal_draw_layer_info.h"
#include "time_line_internal_draw_layer_name.h"
#include "time_line_internal_initialise_display_buffer.h"
#include "time_line_internal_invalidate_layer_area.h"
#include "time_line_internal_redraw_bg_area.h"
#include "time_line_internal_redraw_layer_bg.h"

void bot_right_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	// Local variables
	layer				*background_layer_data;		// Data for the background layer
	GdkModifierType		button_state;				// Mouse button states
	gint				end_row;					// Number of the last layer in this slide
	gfloat				end_time;					// The end time in seconds of the presently selected layer
	GtkAllocation		area;						// Area covered by an individual guide line
	GList				*layer_pointer;				// Points to the layers in the selected slide
	GString				*message;					// Used to construct message strings
	gint				mouse_x;					// Mouse x position
	gint				mouse_y;					// Mouse x position
	TimeLinePrivate		*priv;
	gboolean			return_code_gbool;			// Receives boolean return codes
	layer				*this_layer_data;			// Data for the presently selected layer
	slide				*this_slide_data;			// Data for the presently selected slide
	GList				*tmp_glist = NULL;			// Is given a list of child widgets, if any exist
	undo_history_data	*undo_item_data = NULL;		// Memory structure undo history items are created in

	priv = data;
	if (NULL == widget)
	{
		return;
	}

	// Check for primary mouse button
	if (1 != event->button)
	{
		// Not a primary mouse, so we return
		return;
	}

	// Find out where the mouse is positioned, and which buttons and modifier keys are down (active)
	gdk_window_get_pointer(event->window, &mouse_x, &mouse_y, &button_state);

	area.x = priv->guide_line_start;
	area.y = 0;
	area.height = GTK_WIDGET(widget)->allocation.height;
	area.width = 1;
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &area, TRUE);
	area.x = priv->guide_line_end;
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &area, TRUE);


	if (TRUE == priv->drag_active)
	{
		// Note that the drag has finished
		priv->drag_active = FALSE;
		// Mark that there are unsaved changes
		set_changes_made(TRUE);
		// Create pointers to things we're working with
		this_slide_data = get_current_slide_data();
		layer_pointer = this_slide_data->layers;
		layer_pointer = g_list_first(layer_pointer);
		this_layer_data = g_list_nth_data(layer_pointer, priv->selected_layer_num);
		// Calculate the end time of the layer (in seconds)
		end_time = this_layer_data->start_time + this_layer_data->duration;
		if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
			end_time += this_layer_data->transition_in_duration;
		if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
			end_time += this_layer_data->transition_out_duration;
		// Check if the new end time is longer than the slide duration
		if (end_time > priv->stored_slide_duration)
		{
		// The new slide duration is longer than the old one, so update the slide and background layer to match
		this_slide_data->duration = priv->stored_slide_duration = end_time;
		end_row = this_slide_data->num_layers - 1;
		background_layer_data = g_list_nth_data(layer_pointer, end_row);
		background_layer_data->duration = priv->stored_slide_duration;
		// Refresh the time line display of the background layer
		time_line_internal_redraw_layer_bg(priv, end_row);
		time_line_internal_draw_layer_name(priv, end_row);
		time_line_internal_draw_layer_duration(priv, end_row);
		time_line_internal_invalidate_layer_area(GTK_WIDGET(priv->main_table->parent), end_row);
		}

		// Use the status bar to communicate the drag has completed
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), _(" Drag completed"));
		gdk_flush();

		// Store the undo item created in the button click event handler function
		undo_item_data = time_line_get_undo_item();
		undo_item_data->position_new = priv->selected_layer_num;
		undo_item_data->layer_data_new = layer_duplicate(this_layer_data);
		undo_history_add_item(UNDO_CHANGE_LAYER, undo_item_data, TRUE);
		// For Updating the scoll bars, just redraw the same timeline / only if it is a drag event
		draw_timeline();
		}

		// Recreate the film strip thumbnail
		film_strip_create_thumbnail(get_current_slide_data());

		// Draw the start and end points for the layer
		draw_layer_start_and_end_points();


}

void bot_right_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	gfloat				end_time;					// The end time in seconds of the presently selected layer
	GList				*layer_pointer;				// Points to the layers in the selected slide
	gint				left_border;
	gint				new_row;					// Used to determine the row clicked upon
	gint				pps;						// Holds the number of pixels per second used when drawing
	TimeLinePrivate		*priv;
	layer				*this_layer_data;			// Data for the presently selected layer
	slide				*this_slide_data;			// Data for the presently selected slide
	undo_history_data	*undo_item_data = NULL;		// Memory structure undo history items are created in

	priv = data;

	if (NULL == widget)
	{
		return;
	}

	// Set the delete focus to be layers
	set_delete_focus(FOCUS_LAYER);

	// Change the focus of the window to be this widget
	gtk_widget_grab_focus(GTK_WIDGET(widget));

	// Check for primary mouse button
	if (1 != event->button)
	{
		// Not a primary mouse, so we return
		return;
	}


	this_slide_data = get_current_slide_data();
	layer_pointer = this_slide_data->layers;
	layer_pointer = g_list_first(layer_pointer);
	left_border = time_line_get_left_border_width(priv);
	pps = time_line_get_pixels_per_second();


	if (GDK_2BUTTON_PRESS == event->type)
	{
		// Open an edit dialog
		layer_edit();
		return;
	}

	// Figure out which row the user has selected in the timeline area
	new_row = floor(event->y  / priv->row_height);

	// Ensure the user clicked on a valid row
	if (0 > new_row)
	{
		// Too low, the user didn't click on a valid row
		return;
	}
	if (this_slide_data->num_layers <= new_row)
	{
		// Too high, the user didn't click on a valid row
		return;
	}

	// The user clicked on a valid row, so update the selection
	time_line_set_selected_layer_num(GTK_WIDGET(priv->main_table->parent), new_row);
	this_layer_data = g_list_nth_data(layer_pointer, new_row);

	// Create an undo history item and store the existing layer data in it
	undo_item_data = g_new0(undo_history_data, 1);
	undo_item_data->layer_data_old = layer_duplicate(this_layer_data);
	undo_item_data->position_old = new_row;
	undo_item_data->slide_data = this_slide_data;
	time_line_set_undo_item(undo_item_data);

	// Calculate the present end time of the layer (in seconds)
	end_time = this_layer_data->start_time + this_layer_data->duration;
	if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
		end_time += this_layer_data->transition_in_duration;
	if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
		end_time += this_layer_data->transition_out_duration;

	// Store the guide line positions so we know where to refresh
	priv->guide_line_start = (this_layer_data->start_time * pps);
	priv->guide_line_end = (end_time * pps);

	// Draw guide lines
	time_line_internal_draw_guide_line(GTK_WIDGET(priv->main_table->parent), priv->guide_line_start);
	time_line_internal_draw_guide_line(GTK_WIDGET(priv->main_table->parent), priv->guide_line_end);


	draw_handle_box();

	// If it's not the background layer that is selected then update things in the working area
	if (this_slide_data->num_layers > (new_row + 1))
	{
		// Draw the start and end points for the layer
		draw_layer_start_and_end_points();
	} else
	{
		// Background layer was clicked, so clear any start and end points shown in the working area
		gdk_draw_drawable(GDK_DRAWABLE(get_main_drawing_area()->window), GDK_GC(get_main_drawing_area()->style->fg_gc[GTK_WIDGET_STATE(get_main_drawing_area())]),
				GDK_PIXMAP(get_front_store()), 0, 0, 0, 0, -1, -1);
	}


}

void bot_right_motion_notify_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	GtkAllocation		area;						// Rectangular area
	layer				*background_layer_data;		// Data for the background layer
	gint				check_pixel;				// Used when calculating pixel positions
	gint				current_row;				// The presently selected row
	gint				distance_moved;				// Number of pixels the row has been scrolled by horizontally
	gint				end_row;					// Number of the last layer in this slide
	gfloat				end_time;					// The end time in seconds of the presently selected layer
	GList				*layer_above;				// The layer above the selected one
	GList				*layer_below;				// The layer below the selected one
	GList				*layer_pointer;				// Points to the layers in the selected slide
	gint				left_border;
	gboolean			max_duration_reached;		// Tracks whether we've hit the maximum duration for a transition with this drag
	gfloat				max_moved;					// Used to calculate the maximum amount a duration can be expanded before it hits its limit
	GString				*message;					// Used to construct message strings
	gint				new_row;					// The row the mouse is over
	gint				pps;						// Holds the number of pixels used to draw 1 second in the timeline
	TimeLinePrivate		*priv;
	GList				*selected_layer;			// The selected layer
	layer				*this_layer_data;			// Data for the presently selected layer
	slide				*this_slide_data;			// Data for the presently selected slide
	gfloat				time_moved;					// Number of seconds the row is being adjusted by

	gint 				check_left;
	gint				check_right;
	gint				temp_int;

	priv = data;

	// Initialisation
	this_slide_data = get_current_slide_data();
	layer_pointer = this_slide_data->layers;
	end_row = this_slide_data->num_layers -1;
	current_row = priv->selected_layer_num;
	pps = time_line_get_pixels_per_second();
	left_border = time_line_get_left_border_width(priv);
	max_duration_reached = FALSE;

	new_row = floor(event->y / priv->row_height);

	// Retrieve the layer data for the selected row
	layer_pointer = g_list_first(layer_pointer);
	this_layer_data = g_list_nth_data(layer_pointer, current_row);

	// Calculate the present end time of the layer (in seconds)
	end_time = this_layer_data->start_time + this_layer_data->duration;
	if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
		end_time += this_layer_data->transition_in_duration;
	if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
		end_time += this_layer_data->transition_out_duration;



	if (RESIZE_NONE == priv->resize_type)
	{
		if(current_row == end_row)
			return;

	check_left =  (this_layer_data->start_time * pps) + 5;
	check_right = check_left + (this_layer_data->duration * pps) - 10;
	if(TRANS_LAYER_NONE != this_layer_data->transition_in_type)
			check_right += (this_layer_data->transition_in_duration * pps);
	if(TRANS_LAYER_NONE != this_layer_data->transition_out_type)
			check_right += (this_layer_data->transition_out_duration * pps);


		if (FALSE == priv->drag_active)
		{
			// If the user is trying to drag outside the valid layers, ignore this event
			if ((0 > new_row) || (new_row > end_row) || (current_row > end_row) || (0 > current_row))
			{
			// Mark this function as complete, and return
			return;
			}
			// We're commencing a drag, so note this
			// Starting point of the drag
			if((event->x > check_left ) && (event->x < check_right))
			{
				priv->stored_x = event->x;
			}
			else
			{
				return;
			}
			priv->drag_active = TRUE;
			priv->stored_y = event->y;
		}
		else
		{
			// Clamp the row number to acceptable bounds
			new_row = CLAMP(new_row, 0, end_row - 1);
			// * We are already dragging, so check if the selected row should be moved vertically *
			if (current_row > new_row)
			{
				// * The layer is being moved vertically upwards *
				// Get details of the layers we're moving around
				selected_layer = g_list_nth(layer_pointer, current_row);
				layer_above = g_list_nth(layer_pointer, new_row);
				// Move the row up in the layer list
				layer_pointer = g_list_remove_link(layer_pointer, selected_layer);
				layer_pointer = g_list_insert_before(layer_pointer, layer_above, selected_layer->data);
				this_slide_data->layers = layer_pointer;
				// Refresh the display buffer
				time_line_internal_initialise_display_buffer(priv,
						GTK_WIDGET(widget)->allocation.width,
						GTK_WIDGET(widget)->allocation.height);
				// Draw the layer information
				time_line_internal_draw_layer_info(priv);
				// Update the selected row
				time_line_set_selected_layer_num(GTK_WIDGET(priv->main_table->parent), new_row);
				// Set the height related variables
				area.x = 0;
				area.y = 0;
				area.height = GTK_WIDGET(widget)->allocation.height;
				area.width = GTK_WIDGET(widget)->allocation.width;
				// Invalidate the selected area
				gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &area, TRUE);
			}
			if (current_row < new_row)
			{
				// * The layer is being moved vertically downwards *
				// Get details of the layer we're moving around
				selected_layer = g_list_nth(layer_pointer, current_row);
				// Move the row down in the layer list
				layer_pointer = g_list_remove_link(layer_pointer, selected_layer);
				layer_below = g_list_nth(layer_pointer, new_row);
				layer_pointer = g_list_insert_before(layer_pointer, layer_below, selected_layer->data);
				this_slide_data->layers = layer_pointer;
				// Refresh the display buffer
				time_line_internal_initialise_display_buffer(priv,
						GTK_WIDGET(widget)->allocation.width,
						GTK_WIDGET(widget)->allocation.height);

				// Draw the layer information
				time_line_internal_draw_layer_info(priv);

				// Update the selected row
				time_line_set_selected_layer_num(GTK_WIDGET(priv->main_table->parent), new_row);

				// Set the height related variables
				area.x = 0;
				area.y = 0;
				area.height = GTK_WIDGET(widget)->allocation.height;
				area.width = GTK_WIDGET(widget)->allocation.width;

				// Invalidate the selected area
				gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &area, TRUE);
			}
			// * Check if the row should be moved horizontally *
			if ((priv->stored_x != event->x) && (event->x >= 0) && (event->x < GTK_WIDGET(widget)->allocation.width))
			{
				//check whether this is in the main area or not
				if(event->x>=0){
					temp_int = CLAMP(event->x, 0, GTK_WIDGET(widget)->allocation.width);
					distance_moved = priv->stored_x - temp_int;
					time_moved = ((gfloat) distance_moved) / pps;
					if((this_layer_data->start_time - time_moved) >= 0)
					{
							this_layer_data->start_time -= time_moved;
					}
					// Update the stored position of the row in the widget
					priv->stored_x = event->x;
				}
				// if it is not start it from zero - clearing the bug
				else{
					this_layer_data->start_time = 0;
				}
				// Refresh the timeline display of the row
				time_line_internal_redraw_layer_bg(priv, current_row);
				time_line_internal_draw_layer_name(priv, current_row);
				time_line_internal_draw_layer_duration(priv, current_row);
				// Tell the window system to update the current row area onscreen
				time_line_internal_invalidate_layer_area(GTK_WIDGET(priv->main_table->parent), current_row);
				}

				// Ensure the background layer end is kept correct
				background_layer_data = g_list_nth_data(layer_pointer, end_row);
				if (background_layer_data->duration != priv->stored_slide_duration)
				{
					background_layer_data->duration = priv->stored_slide_duration;
					// Refresh the timeline display of the background layer
					area.x = 0;
					area.y = (end_row * priv->row_height) + 2;
					area.height = priv->row_height - 3;
					area.width = GTK_WIDGET(widget)->allocation.width;
					//time_line_internal_redraw_bg_area(priv, area.x, area.y, area.width, area.height);
					time_line_internal_draw_layer_duration(priv, end_row);

					// Refresh the newly drawn widget area
					gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &area, TRUE);
				}

				// Check if the new end time is longer than the slide duration
				if (end_time > priv->stored_slide_duration)
				{
					// The new end time is longer than the slide duration, so update the slide and background layer to match
					//this_slide_data->duration = end_time;
					background_layer_data->duration = end_time;
					// Refresh the timeline display of the background layer
					area.x = 0;
					area.y = (end_row * priv->row_height) + 2;
					area.height = priv->row_height - 3;
					area.width = GTK_WIDGET(widget)->allocation.width;
					//time_line_internal_redraw_bg_area(priv, area.x, area.y, area.width, area.height);
					time_line_internal_draw_layer_duration(priv, end_row);

					// Refresh the newly drawn widget area
					gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &area, TRUE);
				}
			}
		}

		// Remove the old guide lines
		area.y = 0;
		area.height = GTK_WIDGET(widget)->allocation.height;
		area.width = 1;
		area.x = priv->guide_line_start;
		gtk_widget_draw(GTK_WIDGET(widget), &area);  // Yes, this is deprecated, but it *works*
		area.x = priv->guide_line_end;
		gtk_widget_draw(GTK_WIDGET(widget), &area);  // Yes, this is deprecated, but it *works*

		// Recalculate the present end time of the layer (in seconds)
		end_time = this_layer_data->start_time + this_layer_data->duration;
		if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
			end_time += this_layer_data->transition_in_duration;
		if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
			end_time += this_layer_data->transition_out_duration;

		// Update the guide line positions so we know where to refresh
		priv->guide_line_start = (this_layer_data->start_time * pps) + 1;
		priv->guide_line_end = (end_time * pps);

		// Draw the updated guide lines
		time_line_internal_draw_guide_line(GTK_WIDGET(priv->main_table->parent), priv->guide_line_start);
		time_line_internal_draw_guide_line(GTK_WIDGET(priv->main_table->parent), priv->guide_line_end);
}
