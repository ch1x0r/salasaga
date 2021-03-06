/*
 * bot_right_motion_notify_event.c
 *
 *  Created on: Jun 9, 2010
 *      Author: althaf
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

// Turn on C99 compatibility - needed for roundf() to work
#define _ISOC99_SOURCE

#include <math.h>
#include <gtk/gtk.h>
#include "../../salasaga_types.h"
#include "../global_functions.h"
#include "../layer/layer_edit.h"
#include "../other/widget_focus.h"

#include "../working_area/draw_handle_box.h"
#include "../working_area/draw_layer_start_and_end_points.h"
#include "../working_area/draw_workspace.h"
#include "../film_strip/film_strip_create_thumbnail.h"
#include "time_line.h"
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
#include "time_line_internal_draw_cursor.h"
#include "time_line_internal_initialise_bg_image.h"
#include "draw_timeline.h"
#include "top_left_motion_notify_event.h"
#include "top_right_motion_notify_event.h"

void bot_right_motion_notify_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	GtkAllocation		area;						// Rectangular area
	layer				*background_layer_data;		// Data for the background layer
	gint				current_row;				// The presently selected row
	gint				distance_moved;				// Number of pixels the row has been scrolled by horizontally
	gint				end_row;					// Number of the last layer in this slide
	gfloat				end_time;					// The end time in seconds of the presently selected layer
	GList				*layer_above;				// The layer above the selected one
	GList				*layer_below;				// The layer below the selected one
	GList				*layer_pointer;				// Points to the layers in the selected slide
	gint				left_border;
	gint				check_pixel;				// Used when calculating pixel positions
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

	if(TRUE == priv->left_resize_active){
		top_left_motion_notify_event(priv->top_left_evb,event,priv);
		return;
	}
	if(TRUE == priv->cursor_drag_active)
	{
		top_right_motion_notify_event(priv->top_right_evb, event, priv);
		return;
	}

	if ((RESIZE_NONE == priv->resize_type) && (FALSE == priv->drag_active))
	{
		// If the user is trying to drag outside the valid layers, ignore this event
		// fixme2: We should allow resizing the background layer from the end at some point
		if ((0 > new_row) || (new_row >= end_row) || (current_row >= end_row) || (0 > current_row))
		{
			// Mark this function as complete, and return
			return;
		}


		// Check if the user clicked on the start of the layer (i.e. wants to adjust the start time)
		check_pixel = (this_layer_data->start_time * pps);
		if (1 < check_pixel)
			check_pixel -= 5;
		if ((event->x >= check_pixel) && (event->x <= check_pixel + 10))
		{
			// The user clicked on the start pixel for a layer, so we're commencing a resize
			if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
			{
				// We're adjusting the start time of a transition in
				priv->resize_type = RESIZE_TRANS_IN_START;
				priv->stored_x = event->x;
			} else
			{
				// We're adjusting the start time of the main duration
				priv->resize_type = RESIZE_LAYER_START;
				priv->stored_x = event->x;
			}
		}

		// Is there a transition in for this layer?
		if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
		{
			// Check if the user clicked on the end of a transition in
			check_pixel = ((this_layer_data->start_time + this_layer_data->transition_in_duration) * pps);
			if (1 < check_pixel)
				check_pixel -= 5;
			if ((event->x >= check_pixel) && (event->x <= check_pixel + 10))
			{
				// We're adjusting the end time of the transition in
				priv->resize_type = RESIZE_LAYER_START;
				priv->stored_x = event->x;
			}
		}

		// Check if the user clicked on the end of the duration for the layer (i.e. wants to adjust the duration time)
		check_pixel = ((this_layer_data->start_time + this_layer_data->duration) * pps);
		if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
			check_pixel += this_layer_data->transition_in_duration * pps;
		if (1 < check_pixel)
			check_pixel -= 5;
		if ((event->x >= check_pixel) && (event->x <= check_pixel + 10))
		{
			// We're adjusting the main duration time
			priv->resize_type = RESIZE_LAYER_DURATION;
			priv->stored_x = event->x;
		}

		// Is there a transition out for this layer?
		if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
		{
			// Check if the user clicked on the end of a transition out
			check_pixel = ((this_layer_data->start_time + this_layer_data->duration + this_layer_data->transition_out_duration) * pps);
			if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
				check_pixel += this_layer_data->transition_in_duration * pps;
			if (1 < check_pixel)
				check_pixel -= 5;
			if ((event->x >= check_pixel) && (event->x <= check_pixel + 10))
			{
				// We're adjusting the end time of the transition out
				priv->resize_type = RESIZE_TRANS_OUT_DURATION;
				priv->stored_x = event->x;
			}
		}
	}

	// If we started resizing
	if (RESIZE_NONE != priv->resize_type)
	{
			switch (priv->resize_type)
			{
				case RESIZE_TRANS_IN_START:
					temp_int = CLAMP(event->x, 0, GTK_WIDGET(widget)->allocation.width);
					time_moved = (gfloat)((temp_int -  (this_layer_data->start_time * pps)) / pps);
					if(time_moved < this_layer_data->transition_in_duration && time_moved >= (-1*this_layer_data->start_time) && (get_valid_fields_max_value(TRANSITION_DURATION) >= (this_layer_data->transition_in_duration - time_moved)) ){
						this_layer_data->start_time += time_moved;
						this_layer_data->transition_in_duration -= time_moved;
					}
					else if(time_moved > this_layer_data->transition_in_duration)
					{
						this_layer_data->start_time = end_time - this_layer_data->duration - this_layer_data->transition_out_duration;
						this_layer_data->transition_in_duration = 0;
					}
					break;

				case RESIZE_LAYER_START:
					if(event->x>=0){
						temp_int = CLAMP(event->x,0, GTK_WIDGET(widget)->allocation.width);
						time_moved = (gfloat)((temp_int - ((this_layer_data->start_time + this_layer_data->transition_in_duration) * pps)) / pps);
						if(time_moved <= this_layer_data->duration && time_moved >= (-1 * this_layer_data->start_time)){
							this_layer_data->start_time += time_moved;
							this_layer_data->duration   -= time_moved;
						}
						if(time_moved > this_layer_data->duration){
							this_layer_data->duration  = 0;
							this_layer_data->start_time = end_time - this_layer_data->transition_in_duration - this_layer_data->transition_out_duration;

						}
					}
					else{
						this_layer_data->start_time = 0;
						this_layer_data->duration = end_time - this_layer_data->transition_in_duration - this_layer_data->transition_out_duration;
					}
					break;
				case RESIZE_LAYER_DURATION:
					if(event->x>=0){
						temp_int = CLAMP(event->x, 0, GTK_WIDGET(widget)->allocation.width);
						time_moved = (gfloat)((temp_int - ((this_layer_data->start_time + this_layer_data->transition_in_duration + this_layer_data->duration) * pps)) / pps);
						if(time_moved >= (-1 * this_layer_data->duration))
							this_layer_data->duration += time_moved;
						else if(time_moved < (-1 * this_layer_data->duration))
							this_layer_data->duration = 0;

					}
					else
					{
						this_layer_data->duration = 0;
					}
					break;
				case RESIZE_TRANS_OUT_DURATION:
					temp_int = CLAMP(event->x, left_border, GTK_WIDGET(widget)->allocation.width);
					time_moved = (gfloat)((temp_int - ((this_layer_data->start_time + this_layer_data->transition_in_duration + this_layer_data->duration + this_layer_data->transition_out_duration) * pps)) / pps);
					if(time_moved >= (-1 * this_layer_data->transition_out_duration) && (get_valid_fields_max_value(TRANSITION_DURATION) >= (this_layer_data->transition_out_duration+time_moved)))
						this_layer_data->transition_out_duration += time_moved;
					else if(time_moved < (-1 * this_layer_data->transition_out_duration))
						this_layer_data->transition_out_duration =0;
					break;
			}




			// Ensure the background layer end is kept correct
			background_layer_data = g_list_nth_data(layer_pointer, end_row);
			if (background_layer_data->duration != priv->stored_slide_duration)
			{
				background_layer_data->duration = priv->stored_slide_duration;

				// Refresh the timeline display of the background layer
				area.x = priv->stored_slide_duration * pps;
				area.y = (end_row * priv->row_height) + 2;
				area.height = priv->row_height - 3;
				area.width = GTK_WIDGET(widget)->allocation.width;
				gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &area, TRUE);
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
				area.x = priv->stored_slide_duration * pps;
				area.y = (end_row * priv->row_height) + 2;
				area.height = priv->row_height - 3;
				area.width = GTK_WIDGET(widget)->allocation.width;
				gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &area, TRUE);
				//time_line_internal_redraw_bg_area(priv, area.x, area.y, area.width, area.height);
				time_line_internal_draw_layer_duration(priv, end_row);

				// Refresh the newly drawn widget area
				gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &area, TRUE);
			}

			// Safety check
			if (0 > this_layer_data->start_time)
			{
				this_layer_data->start_time = 0;
			}



			// Refresh the time line display of the row
			time_line_internal_redraw_layer_bg(priv, current_row);
			time_line_internal_draw_layer_name(priv, current_row);
			time_line_internal_draw_layer_duration(priv, current_row);

			// Tell the window system to update the current row area on screen
			time_line_internal_invalidate_layer_area(GTK_WIDGET(priv->main_table->parent), current_row);

			// Wipe existing guide line if present
			if (0 != priv->guide_line_resize)
			{
				area.x = priv->guide_line_resize;
				area.y = 0;
				area.height = GTK_WIDGET(widget)->allocation.height;
				area.width = 1;
				gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &area, TRUE);
			}

			// Store the resize guide line position so we know where to refresh
			switch (priv->resize_type)
			{
				case RESIZE_TRANS_IN_START:
					// No guide line needed

					break;

				case RESIZE_LAYER_START:

					// Draw guide line if needed
					if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
					{
						priv->guide_line_resize = (this_layer_data->start_time * pps) + 1;
						priv->guide_line_resize += this_layer_data->transition_in_duration * pps;
						time_line_internal_draw_guide_line(GTK_WIDGET(priv->main_table->parent), priv->guide_line_resize);
					}
					break;

				case RESIZE_LAYER_DURATION:

					// Draw guide line if needed
					if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
					{
						priv->guide_line_resize =  ((this_layer_data->start_time + this_layer_data->duration) * pps) + 1;
						if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
							priv->guide_line_resize += this_layer_data->transition_in_duration * pps;
						time_line_internal_draw_guide_line(GTK_WIDGET(priv->main_table->parent), priv->guide_line_resize);
					}
					break;

				case RESIZE_TRANS_OUT_DURATION:
					// No guide line needed
					break;

				default:
//					message = g_string_new(NULL);
//					g_string_printf(message, "%s ED368: %s", _("Error"), _("Unknown layer resize type."));
//					display_warning(message->str);
//					g_string_free(message, TRUE);
					break;
			}
			priv->stored_x = event->x;

		}

	// check if it is a drag!
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

