/*
 * $Id$
 *
 * Salasaga:
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


// Turn on C99 compatibility - needed for roundf() to work
#define _ISOC99_SOURCE

// Standard include
#include <math.h>

// GTK includes
#include <gtk/gtk.h>

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../dialog/display_warning.h"
#include "../working_area/draw_workspace.h"
#include "time_line.h"
#include "time_line_get_cursor_position.h"
#include "time_line_get_left_border_width.h"
#include "time_line_get_type.h"
#include "time_line_internal_draw_guide_line.h"
#include "time_line_internal_draw_layer_duration.h"
#include "time_line_internal_draw_layer_info.h"
#include "time_line_internal_draw_layer_name.h"
#include "time_line_internal_initialise_display_buffer.h"
#include "time_line_internal_invalidate_layer_area.h"
#include "time_line_internal_redraw_bg_area.h"
#include "time_line_internal_redraw_layer_bg.h"
#include "time_line_set_cursor_position.h"
#include "time_line_set_selected_layer_num.h"


gboolean time_line_internal_widget_motion_notify_handler(TimeLine *this_time_line)
{
	// Local variables
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


	// Safety check
	if (FALSE == IS_TIME_LINE(this_time_line))
	{
		return FALSE;
	}

	// If there isn't anything to do, then skip running on this call
	priv = TIME_LINE_GET_PRIVATE(this_time_line);
	if ((NULL == priv) || ((-1 == priv->mouse_x) && (-1 == priv->mouse_y)))
	{
		return TRUE;
	}

	// Initialisation
	this_slide_data = get_current_slide_data();
	layer_pointer = this_slide_data->layers;
	end_row = this_slide_data->num_layers -1;
	current_row = priv->selected_layer_num;
	pps = time_line_get_pixels_per_second();
	left_border = time_line_get_left_border_width(priv);
	max_duration_reached = FALSE;

	// Check if this mouse drag is in the cursor area, and no other operations are in progress
	if ((ADJUSTMENTS_Y <= priv->mouse_y) && ((ADJUSTMENTS_Y + ADJUSTMENTS_SIZE) >= priv->mouse_y)
			&& (RESIZE_NONE == priv->resize_type)
			&& (FALSE == priv->drag_active)
			&& (FALSE == priv->cursor_drag_active)
			&& (priv->mouse_x >= left_border))
	{
		// This is the first time we've heard of this cursor drag, so take note of it
		priv->cursor_drag_active = TRUE;
		priv->stored_x = priv->mouse_x;

		// Mark this function as complete, and return
		priv->mouse_x = -1;
		priv->mouse_y = -1;
		return TRUE;
	}

	// Check if a cursor drag is in progress
	if ((RESIZE_NONE == priv->resize_type)
			&& (FALSE == priv->drag_active)
			&& (TRUE == priv->cursor_drag_active))
	{
		// Yes, this is definitely an in-progress cursor drag

		// Check if the drag is moving to the right
		if (priv->stored_x < priv->mouse_x)
		{
			// Calculate the time and distance traveled
			priv->mouse_x = CLAMP(priv->mouse_x, left_border, GTK_WIDGET(this_time_line)->allocation.width);
			distance_moved = priv->mouse_x - priv->stored_x;
			time_moved = ((gfloat) distance_moved) / pps;

			// Invalidate the widget area where the cursor is presently
			area.x = left_border + (time_line_get_cursor_position(GTK_WIDGET(this_time_line)) * pps) - (CURSOR_HEAD_WIDTH / 2);
			area.y = 0;
			area.height = GTK_WIDGET(this_time_line)->allocation.height;
			area.width = CURSOR_HEAD_WIDTH + 1;
			gdk_window_invalidate_rect(GTK_WIDGET(this_time_line)->window, &area, TRUE);

			// Update the cursor position
			time_line_set_cursor_position(GTK_WIDGET(this_time_line), time_line_get_cursor_position(GTK_WIDGET(this_time_line)) + time_moved);
			priv->stored_x = priv->mouse_x;

			// Invalidate the widget area where the cursor has moved to
			area.x = left_border + (time_line_get_cursor_position(GTK_WIDGET(this_time_line)) * pps) - (CURSOR_HEAD_WIDTH / 2);
			area.y = 0;
			area.height = GTK_WIDGET(this_time_line)->allocation.height;
			area.width = CURSOR_HEAD_WIDTH + 1;
			gdk_window_invalidate_rect(GTK_WIDGET(this_time_line)->window, &area, TRUE);
		}

		// Check if the drag is moving to the left
		if (priv->stored_x > priv->mouse_x)
		{
			// Calculate the time and distance traveled
			priv->mouse_x = CLAMP(priv->mouse_x, left_border, GTK_WIDGET(this_time_line)->allocation.width);
			distance_moved = priv->stored_x - priv->mouse_x;
			time_moved = ((gfloat) distance_moved) / pps;

			// Invalidate the widget area where the cursor is presently
			area.x = left_border + (time_line_get_cursor_position(GTK_WIDGET(this_time_line)) * pps) - (CURSOR_HEAD_WIDTH / 2);
			area.y = 0;
			area.height = GTK_WIDGET(this_time_line)->allocation.height;
			area.width = CURSOR_HEAD_WIDTH + 1;
			gdk_window_invalidate_rect(GTK_WIDGET(this_time_line)->window, &area, TRUE);

			// Update the cursor position
			time_line_set_cursor_position(GTK_WIDGET(this_time_line), time_line_get_cursor_position(GTK_WIDGET(this_time_line)) - time_moved);
			priv->stored_x = priv->mouse_x;

			// Invalidate the widget area where the cursor has moved to
			area.x = left_border + (time_line_get_cursor_position(GTK_WIDGET(this_time_line)) * pps) - (CURSOR_HEAD_WIDTH / 2);
			area.y = 0;
			area.height = GTK_WIDGET(this_time_line)->allocation.height;
			area.width = CURSOR_HEAD_WIDTH + 1;
			gdk_window_invalidate_rect(GTK_WIDGET(this_time_line)->window, &area, TRUE);
		}

		// Safety check
		if (0 > time_line_get_cursor_position(GTK_WIDGET(this_time_line)))
			time_line_set_cursor_position(GTK_WIDGET(this_time_line), 0);

		// Update the workspace area
		draw_workspace();

		// Mark this function as complete, and return
		priv->mouse_x = -1;
		priv->mouse_y = -1;
		return TRUE;
	}

	// Work out which row the mouse is over in the timeline area
	new_row = floor((priv->mouse_y - priv->top_border_height) / priv->row_height);

	// Retrieve the layer data for the selected row
	layer_pointer = g_list_first(layer_pointer);
	this_layer_data = g_list_nth_data(layer_pointer, current_row);

	// Calculate the present end time of the layer (in seconds)
	end_time = this_layer_data->start_time + this_layer_data->duration;
	if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
		end_time += this_layer_data->transition_in_duration;
	if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
		end_time += this_layer_data->transition_out_duration;

	// If we're not already resizing or dragging, check for a new resize starting
	if ((RESIZE_NONE == priv->resize_type) && (FALSE == priv->drag_active))
	{
		// If the user is trying to drag outside the valid layers, ignore this event
		// fixme2: We should allow resizing the background layer from the end at some point
		if ((0 > new_row) || (new_row >= end_row) || (current_row >= end_row) || (0 > current_row))
		{
			// Mark this function as complete, and return
			priv->mouse_x = -1;
			priv->mouse_y = -1;
			return TRUE;
		}

		// Check if the user clicked on the start of the layer (i.e. wants to adjust the start time)
		check_pixel = left_border + (this_layer_data->start_time * pps);
		if (1 < check_pixel)
			check_pixel -= 5;
		if ((priv->mouse_x >= check_pixel) && (priv->mouse_x <= check_pixel + 10))
		{
			// The user clicked on the start pixel for a layer, so we're commencing a resize
			if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
			{
				// We're adjusting the start time of a transition in
				priv->resize_type = RESIZE_TRANS_IN_START;
				priv->stored_x = priv->mouse_x;
			} else
			{
				// We're adjusting the start time of the main duration
				priv->resize_type = RESIZE_LAYER_START;
				priv->stored_x = priv->mouse_x;
			}
		}

		// Is there a transition in for this layer?
		if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
		{
			// Check if the user clicked on the end of a transition in
			check_pixel = left_border +
							((this_layer_data->start_time + this_layer_data->transition_in_duration) * pps);
			if (1 < check_pixel)
				check_pixel -= 5;
			if ((priv->mouse_x >= check_pixel) && (priv->mouse_x <= check_pixel + 10))
			{
				// We're adjusting the end time of the transition in
				priv->resize_type = RESIZE_LAYER_START;
				priv->stored_x = priv->mouse_x;
			}
		}

		// Check if the user clicked on the end of the duration for the layer (i.e. wants to adjust the duration time)
		check_pixel = left_border + ((this_layer_data->start_time + this_layer_data->duration) * pps);
		if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
			check_pixel += this_layer_data->transition_in_duration * pps;
		if (1 < check_pixel)
			check_pixel -= 5;
		if ((priv->mouse_x >= check_pixel) && (priv->mouse_x <= check_pixel + 10))
		{
			// We're adjusting the main duration time
			priv->resize_type = RESIZE_LAYER_DURATION;
			priv->stored_x = priv->mouse_x;
		}

		// Is there a transition out for this layer?
		if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
		{
			// Check if the user clicked on the end of a transition out
			check_pixel = left_border + ((this_layer_data->start_time + this_layer_data->duration + this_layer_data->transition_out_duration) * pps);
			if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
				check_pixel += this_layer_data->transition_in_duration * pps;
			if (1 < check_pixel)
				check_pixel -= 5;
			if ((priv->mouse_x >= check_pixel) && (priv->mouse_x <= check_pixel + 10))
			{
				// We're adjusting the end time of the transition out
				priv->resize_type = RESIZE_TRANS_OUT_DURATION;
				priv->stored_x = priv->mouse_x;
			}
		}
	}

	// Check if we're already resizing
	if (RESIZE_NONE != priv->resize_type)
	{
		// Check if the resize is moving to the right
		if (priv->stored_x < priv->mouse_x)
		{
			// Calculate the time and distance travelled
			priv->mouse_x = CLAMP(priv->mouse_x, left_border, GTK_WIDGET(this_time_line)->allocation.width);
			distance_moved = priv->mouse_x - priv->stored_x;
			time_moved = ((gfloat) distance_moved) / pps;

			// Make the needed adjustments
			switch (priv->resize_type)
			{
				case RESIZE_TRANS_IN_START:

					// We're adjusting the transition in start
					if (this_layer_data->transition_in_duration < time_moved)
					{
						// We've shortened the transition in to 0 duration
						this_layer_data->start_time += time_moved;
						this_layer_data->transition_in_duration = 0;
						if (this_layer_data->duration < time_moved)
						{
							this_layer_data->duration = 0;
							if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
							{
								if (this_layer_data->transition_out_duration < time_moved)
								{
									this_layer_data->transition_out_duration = 0;
								} else
								{
									this_layer_data->transition_out_duration -= time_moved;
								}
							}
						} else
						{
							this_layer_data->duration -= time_moved;
						}
					} else
					{
						// Adjust the layer timing
						this_layer_data->transition_in_duration -= time_moved;
						this_layer_data->start_time += time_moved;
					}
					break;

				case RESIZE_LAYER_START:

					// If the adjustment would make the transition in duration longer than the maximum allowed, we limit it to the maximum allowed
					if ((this_layer_data->transition_in_duration + time_moved) > get_valid_fields_max_value(TRANSITION_DURATION))
					{
						time_moved = get_valid_fields_max_value(TRANSITION_DURATION) - this_layer_data->transition_in_duration;
						max_duration_reached = TRUE;
					}

					// We're adjusting the main layer start
					if (this_layer_data->duration < time_moved)
					{
						// We've shortened the layer to 0 duration
						this_layer_data->duration = 0;
						if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
						{
							this_layer_data->transition_in_duration += time_moved;
						} else
						{
							this_layer_data->start_time += time_moved;
						}
						if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
						{
							if (this_layer_data->transition_out_duration < time_moved)
							{
								// We've shortened the transition out duration to 0 as well
								this_layer_data->transition_out_duration = 0;
							} else
							{
								this_layer_data->transition_out_duration -= time_moved;
							}
						}
					} else
					{
						// Adjust the layer timing
						if (FALSE == max_duration_reached)
						{
							this_layer_data->duration -= time_moved;  // Only do this if we haven't dragged to the right past the maximum transition duration
							if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
							{
								this_layer_data->transition_in_duration += time_moved;
							} else
							{
								this_layer_data->start_time += time_moved;
							}
						}
					}
					break;

				case RESIZE_LAYER_DURATION:

					// We're adjusting the main layer duration
					if (this_layer_data->transition_out_duration < time_moved)
					{
						// We've shortened the transition out duration to 0
						this_layer_data->duration += time_moved;
						this_layer_data->transition_out_duration = 0;
					} else
					{
						// Adjust the layer timing
						this_layer_data->duration += time_moved;
						this_layer_data->transition_out_duration -= time_moved;
					}
					break;

				case RESIZE_TRANS_OUT_DURATION:

					// We're adjusting the transition out duration
					this_layer_data->transition_out_duration += time_moved;
					end_time += time_moved;

					// If the adjustment makes the duration longer than the maximum, limit it to the maximum
					if (this_layer_data->transition_out_duration > get_valid_fields_max_value(TRANSITION_DURATION))
					{
						this_layer_data->transition_out_duration = get_valid_fields_max_value(TRANSITION_DURATION);
					}

					break;

				default:
					message = g_string_new(NULL);
					g_string_printf(message, "%s ED367: %s", _("Error"), _("Unknown layer resize type."));
					display_warning(message->str);
					g_string_free(message, TRUE);
			}
		}

		// Check if the resize is moving to the left
		if (priv->stored_x > priv->mouse_x)
		{
			// Calculate the time and distance travelled
			priv->mouse_x = CLAMP(priv->mouse_x, left_border, GTK_WIDGET(this_time_line)->allocation.width);
			distance_moved = priv->stored_x - priv->mouse_x;
			time_moved = ((gfloat) distance_moved) / pps;

			// Make the needed adjustments
			switch (priv->resize_type)
			{
				case RESIZE_TRANS_IN_START:

					// We're adjusting the transition in start time

					// If the adjustment makes the duration longer than the maximum, limit it to the maximum
					if ((this_layer_data->transition_in_duration + time_moved) > get_valid_fields_max_value(TRANSITION_DURATION))
					{
						max_moved = get_valid_fields_max_value(TRANSITION_DURATION) - this_layer_data->transition_in_duration;
						this_layer_data->start_time -= max_moved;
						this_layer_data->transition_in_duration += max_moved;
					} else
					{
						// Make the adjustment
						this_layer_data->start_time -= time_moved;
						this_layer_data->transition_in_duration += time_moved;
					}

					break;

				case RESIZE_LAYER_START:

					// We're adjusting the main layer start
					if (this_layer_data->transition_in_duration < time_moved)
					{
						// We've shortened the duration to 0
						this_layer_data->start_time -= time_moved;
						this_layer_data->transition_in_duration = 0;
						this_layer_data->duration += time_moved;
					} else
					{
						// Adjust the layer timing
						if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
						{
							this_layer_data->transition_in_duration -= time_moved;
						} else
						{
							this_layer_data->start_time -= time_moved;
						}
						this_layer_data->duration += time_moved;
					}
					break;

				case RESIZE_LAYER_DURATION:

					// If the adjustment would make the transition out duration longer than the maximum allowed, we limit it to the maximum allowed
					if ((this_layer_data->transition_out_duration + time_moved) > get_valid_fields_max_value(TRANSITION_DURATION))
					{
						time_moved = get_valid_fields_max_value(TRANSITION_DURATION) - this_layer_data->transition_out_duration;
						max_duration_reached = TRUE;
					}

					// We're adjusting the main layer duration
					if (this_layer_data->duration < time_moved)
					{
						// We've shortened the main layer duration to 0
						this_layer_data->duration = 0;
						this_layer_data->transition_out_duration += time_moved;

						if (this_layer_data->transition_in_duration < time_moved)
						{
							// We've shortened the transition in duration to 0 as well
							this_layer_data->transition_in_duration = 0;
							this_layer_data->start_time -= time_moved;
						} else
						{
							this_layer_data->transition_in_duration -= time_moved;
						}
					} else
					{
						// Adjust the layer timing
						if (FALSE == max_duration_reached)
						{
							this_layer_data->duration -= time_moved;  // Only do this if we haven't dragged to the left past the maximum transition duration
							this_layer_data->transition_out_duration += time_moved;
						}
					}
					if (TRANS_LAYER_NONE == this_layer_data->transition_out_type)
						end_time -= time_moved;
					break;

				case RESIZE_TRANS_OUT_DURATION:

					// We're adjusting the transition out duration
					if (this_layer_data->transition_out_duration < time_moved)
					{
						this_layer_data->transition_out_duration = 0;
						if (this_layer_data->duration < time_moved)
						{
							this_layer_data->duration = 0;
							if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
							{
								if (this_layer_data->transition_in_duration < time_moved)
								{
									this_layer_data->transition_in_duration = 0;
									this_layer_data->start_time -= time_moved;
								} else
								{
									this_layer_data->transition_in_duration -= time_moved;
								}
							} else
							{
								this_layer_data->start_time -= time_moved;
							}
						} else
						{
							this_layer_data->duration -= time_moved;
						}
					} else
					{
						this_layer_data->transition_out_duration -= time_moved;
					}
					end_time -= time_moved;
					break;

				default:
					message = g_string_new(NULL);
					g_string_printf(message, "%s ED420: %s", _("Error"), _("Unknown layer resize type."));
					display_warning(message->str);
					g_string_free(message, TRUE);
			}
		}

		// Ensure the background layer end is kept correct
		background_layer_data = g_list_nth_data(layer_pointer, end_row);
		if (background_layer_data->duration != priv->stored_slide_duration)
		{
			background_layer_data->duration = priv->stored_slide_duration;

			// Refresh the timeline display of the background layer
			area.x = priv->stored_slide_duration * pps;
			area.y = priv->top_border_height + (end_row * priv->row_height) + 2;
			area.height = priv->row_height - 3;
			area.width = GTK_WIDGET(this_time_line)->allocation.width - area.x;
			time_line_internal_redraw_bg_area(priv, area.x, area.y, area.width, area.height);
			time_line_internal_draw_layer_duration(priv, end_row);

			// Refresh the newly drawn widget area
			gdk_window_invalidate_rect(GTK_WIDGET(this_time_line)->window, &area, TRUE);
		}

		// Check if the new end time is longer than the slide duration
		if (end_time > priv->stored_slide_duration)
		{
			// The new end time is longer than the slide duration, so update the slide and background layer to match
			this_slide_data->duration = end_time;
			background_layer_data->duration = end_time;

			// Refresh the timeline display of the background layer
			area.x = priv->stored_slide_duration * pps;
			area.y = priv->top_border_height + (end_row * priv->row_height) + 2;
			area.height = priv->row_height - 3;
			area.width = GTK_WIDGET(this_time_line)->allocation.width - area.x;
			time_line_internal_redraw_bg_area(priv, area.x, area.y, area.width, area.height);
			time_line_internal_draw_layer_duration(priv, end_row);

			// Refresh the newly drawn widget area
			gdk_window_invalidate_rect(GTK_WIDGET(this_time_line)->window, &area, TRUE);
		}

		// Safety check
		if (0 > this_layer_data->start_time)
		{
			this_layer_data->start_time = 0;
		}

		// Update the stored position of the mouse in the widget
		priv->stored_x = priv->mouse_x;

		// Refresh the time line display of the row
		time_line_internal_redraw_layer_bg(priv, current_row);
		time_line_internal_draw_layer_name(priv, current_row);
		time_line_internal_draw_layer_duration(priv, current_row);

		// Tell the window system to update the current row area on screen
		time_line_internal_invalidate_layer_area(GTK_WIDGET(this_time_line), current_row);

		// Wipe existing guide line if present
		if (0 != priv->guide_line_resize)
		{
			area.x = priv->guide_line_resize;
			area.y = 0;
			area.height = GTK_WIDGET(this_time_line)->allocation.height;
			area.width = 1;
			gdk_window_invalidate_rect(GTK_WIDGET(this_time_line)->window, &area, TRUE);
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
					priv->guide_line_resize = left_border + (this_layer_data->start_time * pps);
					priv->guide_line_resize += this_layer_data->transition_in_duration * pps;
					time_line_internal_draw_guide_line(GTK_WIDGET(this_time_line), priv->guide_line_resize);
				}
				break;

			case RESIZE_LAYER_DURATION:

				// Draw guide line if needed
				if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
				{
					priv->guide_line_resize = left_border + ((this_layer_data->start_time + this_layer_data->duration) * pps);
					if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
						priv->guide_line_resize += this_layer_data->transition_in_duration * pps;
					time_line_internal_draw_guide_line(GTK_WIDGET(this_time_line), priv->guide_line_resize);
				}
				break;

			case RESIZE_TRANS_OUT_DURATION:
				// No guide line needed
				break;

			default:
				message = g_string_new(NULL);
				g_string_printf(message, "%s ED368: %s", _("Error"), _("Unknown layer resize type."));
				display_warning(message->str);
				g_string_free(message, TRUE);
		}
	}

	// If we're not already dragging, nor in a resize, assume we should be dragging
	if (RESIZE_NONE == priv->resize_type)
	{
		if (FALSE == priv->drag_active)
		{
			// If the user is trying to drag outside the valid layers, ignore this event
			if ((0 > new_row) || (new_row >= end_row) || (current_row >= end_row) || (0 > current_row))
			{
				// Mark this function as complete, and return
				priv->mouse_x = -1;
				priv->mouse_y = -1;
				return TRUE;
			}

			// We're commencing a drag, so note this
			priv->drag_active = TRUE;

			// Store the mouse coordinates so we know where to drag from
			if (priv->mouse_x < left_border)
			{
				priv->stored_x = left_border;
			} else
			{
				priv->stored_x = priv->mouse_x;
			}
			priv->stored_y = priv->mouse_y;
		} else
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
						GTK_WIDGET(this_time_line)->allocation.width,
						GTK_WIDGET(this_time_line)->allocation.height);

				// Draw the layer information
				time_line_internal_draw_layer_info(priv);

				// Update the selected row
				time_line_set_selected_layer_num(GTK_WIDGET(this_time_line), new_row);

				// Set the height related variables
				area.x = 0;
				area.y = 0;
				area.height = GTK_WIDGET(this_time_line)->allocation.height;
				area.width = GTK_WIDGET(this_time_line)->allocation.width;

				// Invalidate the selected area
				gdk_window_invalidate_rect(GTK_WIDGET(this_time_line)->window, &area, TRUE);
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
						GTK_WIDGET(this_time_line)->allocation.width,
						GTK_WIDGET(this_time_line)->allocation.height);

				// Draw the layer information
				time_line_internal_draw_layer_info(priv);

				// Update the selected row
				time_line_set_selected_layer_num(GTK_WIDGET(this_time_line), new_row);

				// Set the height related variables
				area.x = 0;
				area.y = 0;
				area.height = GTK_WIDGET(this_time_line)->allocation.height;
				area.width = GTK_WIDGET(this_time_line)->allocation.width;

				// Invalidate the selected area
				gdk_window_invalidate_rect(GTK_WIDGET(this_time_line)->window, &area, TRUE);
			}

			// * Check if the row should be moved horizontally *
			if ((priv->stored_x != priv->mouse_x) && (priv->mouse_x > left_border))
			{
				// Check if the layer is being moved to the left
				if (priv->stored_x > priv->mouse_x)
				{
					// Calculate the time and distance travelled
					priv->mouse_x = CLAMP(priv->mouse_x, left_border, GTK_WIDGET(this_time_line)->allocation.width);
					distance_moved = priv->stored_x - priv->mouse_x;
					time_moved = ((gfloat) distance_moved) / pps;

					// Update the layer data with the new timing
					if (0 > (this_layer_data->start_time - time_moved))
					{
						end_time = end_time - this_layer_data->start_time;
						this_layer_data->start_time = 0;
					} else
					{
						this_layer_data->start_time -= time_moved;
						end_time -= time_moved;
					}
				}

				// Check if the layer is being moved to the right
				if (priv->stored_x < priv->mouse_x)
				{
					// Calculate the time and distance travelled
					priv->mouse_x = CLAMP(priv->mouse_x, left_border, GTK_WIDGET(this_time_line)->allocation.width);
					distance_moved = priv->mouse_x - priv->stored_x;
					time_moved = ((gfloat) distance_moved) / pps;

					// Update the layer data with the new timing
					this_layer_data->start_time += time_moved;
					end_time += time_moved;
				}

				// Update the stored position of the row in the widget
				priv->stored_x = priv->mouse_x;

				// Refresh the timeline display of the row
				time_line_internal_redraw_layer_bg(priv, current_row);
				time_line_internal_draw_layer_name(priv, current_row);
				time_line_internal_draw_layer_duration(priv, current_row);

				// Tell the window system to update the current row area onscreen
				time_line_internal_invalidate_layer_area(GTK_WIDGET(this_time_line), current_row);
			}

			// Ensure the background layer end is kept correct
			background_layer_data = g_list_nth_data(layer_pointer, end_row);
			if (background_layer_data->duration != priv->stored_slide_duration)
			{
				background_layer_data->duration = priv->stored_slide_duration;

				// Refresh the timeline display of the background layer
				area.x = priv->stored_slide_duration * pps;
				area.y = priv->top_border_height + (end_row * priv->row_height) + 2;
				area.height = priv->row_height - 3;
				area.width = GTK_WIDGET(this_time_line)->allocation.width - area.x;
				time_line_internal_redraw_bg_area(priv, area.x, area.y, area.width, area.height);
				time_line_internal_draw_layer_duration(priv, end_row);

				// Refresh the newly drawn widget area
				gdk_window_invalidate_rect(GTK_WIDGET(this_time_line)->window, &area, TRUE);
			}

			// Check if the new end time is longer than the slide duration
			if (end_time > priv->stored_slide_duration)
			{
				// The new end time is longer than the slide duration, so update the slide and background layer to match
				this_slide_data->duration = end_time;
				background_layer_data->duration = end_time;

				// Refresh the timeline display of the background layer
				area.x = priv->stored_slide_duration * pps;
				area.y = priv->top_border_height + (end_row * priv->row_height) + 2;
				area.height = priv->row_height - 3;
				area.width = GTK_WIDGET(this_time_line)->allocation.width - area.x;
				time_line_internal_redraw_bg_area(priv, area.x, area.y, area.width, area.height);
				time_line_internal_draw_layer_duration(priv, end_row);

				// Refresh the newly drawn widget area
				gdk_window_invalidate_rect(GTK_WIDGET(this_time_line)->window, &area, TRUE);
			}
		}
	}

	// Remove the old guide lines
	area.y = 0;
	area.height = GTK_WIDGET(this_time_line)->allocation.height;
	area.width = 1;
	area.x = priv->guide_line_start;
	gtk_widget_draw(GTK_WIDGET(this_time_line), &area);  // Yes, this is deprecated, but it *works*
	area.x = priv->guide_line_end;
	gtk_widget_draw(GTK_WIDGET(this_time_line), &area);  // Yes, this is deprecated, but it *works*

	// Recalculate the present end time of the layer (in seconds)
	end_time = this_layer_data->start_time + this_layer_data->duration;
	if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
		end_time += this_layer_data->transition_in_duration;
	if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
		end_time += this_layer_data->transition_out_duration;

	// Update the guide line positions so we know where to refresh
	priv->guide_line_start = left_border + (this_layer_data->start_time * pps);
	priv->guide_line_end = left_border + (end_time * pps) - 1;

	// Draw the updated guide lines
	time_line_internal_draw_guide_line(GTK_WIDGET(this_time_line), priv->guide_line_start);
	time_line_internal_draw_guide_line(GTK_WIDGET(this_time_line), priv->guide_line_end);

	// Update the workspace area
	draw_workspace();

	// Mark this function as complete, and return
	priv->mouse_x = -1;
	priv->mouse_y = -1;
	return TRUE;
}
