/*
 * $Id$
 *
 * Salasaga: Prepares the swf frame elements for a given layer, so it can be converted to swf 
 * 
 * Copyright (C) 2005-2008 Justin Clift <justin@salasaga.org>
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


// Turn on C99 compatibility
#define _ISOC99_SOURCE

// Math include
#include <math.h>

// GTK includes
#include <gtk/gtk.h>

// Salasaga includes
#include "../../../salasaga_types.h"
#include "../../../externs.h"

gboolean export_swf_create_layer_elements(swf_frame_element *array_start, guint num_frames, layer *this_layer_data, guint layer_depth)
{
	// Local variables
	gfloat				click_duration;
	guint				click_frames;
	gfloat				element_x_position_finish = 0;
	gfloat				element_x_position_increment = 0;
	gfloat				element_x_position_start = 0;
	gfloat				element_y_position_finish = 0;
	gfloat				element_y_position_increment = 0;
	gfloat				element_y_position_start = 0;
	gfloat				fade_frame;
	gfloat				finish_frame;
	gint				finish_frame_rounded;
	guint				frame_counter;				// Holds the number of frames
	GString 			*layer_name;				// The text name for the layer
	layer_mouse			*mouse_data;				// Points to the mouse object data inside the layer
	guint				loop_counter = 0;			// Simple counter used in loops
	gint				num_displayed_frames;
	guint				opacity_count;				// Used when calculating object opacity
	gfloat				opacity_step;				// Used when calculating object opacity
	guint				play_click = MOUSE_NONE;	// Should a click sound be played?
	gfloat				scaled_height_ratio;		// Used to calculate the final size an object should be scaled to 
	gfloat				scaled_width_ratio;			// Used to calculate the final size an object should be scaled to
	gfloat				start_frame;
	guint				start_frame_rounded;
	gfloat				x_position;					// Used in calculating layer object position
	gfloat				y_position;					// Used in calculating layer object position


	// Initialisation
	layer_name = g_string_new(NULL);
	g_string_printf(layer_name, "Object%d", layer_depth);

	// Set some basic properties for the layer, across all of its frames
	for (frame_counter = 0; frame_counter < num_frames; frame_counter++)
	{
		array_start[frame_counter].action_this = FALSE;
		array_start[frame_counter].object_name = layer_name;
		array_start[frame_counter].depth = layer_depth;
		array_start[frame_counter].layer_info = this_layer_data;
		array_start[frame_counter].is_moving = FALSE;
	}

	// Calculate the height and width scaling values needed for this swf output
	scaled_height_ratio = (gfloat) output_height / (gfloat) project_height;
	scaled_width_ratio = (gfloat) output_width / (gfloat) project_width;

	// Calculate the scaled start and finish positions for each element
	element_x_position_start = scaled_width_ratio * this_layer_data->x_offset_start;
	element_x_position_finish = scaled_width_ratio * this_layer_data->x_offset_finish;
	element_y_position_start = scaled_height_ratio * this_layer_data->y_offset_start;
	element_y_position_finish = scaled_height_ratio * this_layer_data->y_offset_finish;

	// If there is a fade in transition, fill in the relevant elements
	start_frame = this_layer_data->start_time * frames_per_second;
	if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
	{
		// Work out the starting and ending frames for the fade
		finish_frame = start_frame + (this_layer_data->transition_in_duration * frames_per_second);

		// Indicate on which frame the element should be displayed, at what display depth, and its starting co-ordinates
		start_frame_rounded = roundf(start_frame);
		finish_frame_rounded = roundf(finish_frame);
		array_start[start_frame_rounded].add = TRUE;
		array_start[start_frame_rounded].x_position = element_x_position_start;
		array_start[start_frame_rounded].y_position = element_y_position_start;

		// Work out how much opacity to increment each frame by
		opacity_step = 100 / ((this_layer_data->transition_in_duration * frames_per_second));

		// Loop through each frame of the fade in, setting the opacity values
		opacity_count = 0;
		for (frame_counter = start_frame_rounded; frame_counter <= finish_frame_rounded; frame_counter++)
		{
			array_start[frame_counter].action_this = TRUE;
			array_start[frame_counter].opacity_change = TRUE;
			array_start[frame_counter].opacity = opacity_count;
			array_start[frame_counter].x_position = element_x_position_start;
			array_start[frame_counter].y_position = element_y_position_start;
			opacity_count += floorf(opacity_step);
		}

		// Ensure the layer is completely visible after the end of the fade in
		array_start[frame_counter].action_this = TRUE;
		array_start[frame_counter].opacity_change = TRUE;
		array_start[frame_counter].opacity = 100;
	} else
	{
		// Indicate on which frame the element should be displayed, at what display depth, and its starting co-ordinates
		start_frame_rounded = roundf(start_frame);
		array_start[start_frame_rounded].add = TRUE;
		array_start[start_frame_rounded].x_position = element_x_position_start;
		array_start[start_frame_rounded].y_position = element_y_position_start;
		array_start[start_frame_rounded].action_this = TRUE;
		array_start[start_frame_rounded].opacity = 100;
	}

	// If there is a fade out transition, fill in the relevant elements
	if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
	{
		// Work out the starting and ending frames for the fade
		fade_frame = start_frame;
		if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
			fade_frame += this_layer_data->transition_in_duration * frames_per_second;
		fade_frame += this_layer_data->duration * frames_per_second;
		finish_frame = fade_frame + (this_layer_data->transition_out_duration * frames_per_second);
		start_frame_rounded = roundf(fade_frame);
		finish_frame_rounded = roundf(finish_frame);

		// Work out how much opacity to decrement each frame by
		opacity_step = 100 / ((this_layer_data->transition_out_duration * frames_per_second));

		// Loop through each frame of the fade out, setting the opacity values
		opacity_count = 100;
		for (frame_counter = start_frame_rounded; frame_counter <= finish_frame_rounded; frame_counter++)
		{
			array_start[frame_counter].action_this = TRUE;
			array_start[frame_counter].opacity_change = TRUE;
			array_start[frame_counter].opacity = opacity_count;
			array_start[frame_counter].x_position = element_x_position_finish;
			array_start[frame_counter].y_position = element_y_position_finish;
			opacity_count -= floorf(opacity_step);
		}

		// Ensure the layer is completely invisible after the end of the fade out
		array_start[finish_frame_rounded].action_this = TRUE;
		array_start[finish_frame_rounded].opacity_change = TRUE;
		array_start[finish_frame_rounded].opacity = 0;
		array_start[finish_frame_rounded].x_position = element_x_position_start;
		array_start[finish_frame_rounded].y_position = element_y_position_start;
	}

	// Work out the start frame of the fully visible layer display
	if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
		start_frame += this_layer_data->transition_in_duration * frames_per_second;

	// Work out the finish frame of the fully visible layer display
	finish_frame = start_frame + (this_layer_data->duration * frames_per_second);

	// If this is a mouse cursor with a click, we squeeze the movement part into a smaller number of frames
	// to give time for the click sound to play while the mouse is stationery and before it fades
	if (TYPE_MOUSE_CURSOR == this_layer_data->object_type)
	{
		// Simplify the pointer to the mouse click data
		mouse_data = (layer_mouse *) this_layer_data->object_data;

		// Determine how many frames to compress the mouse movement by, and also add the click sound
		switch (mouse_data->click)
		{
			case MOUSE_LEFT_ONE:
			case MOUSE_RIGHT_ONE:
			case MOUSE_MIDDLE_ONE:

				click_duration = 0.3;
				click_frames = roundf(click_duration * frames_per_second);
				play_click = mouse_data->click;
				break;

			case MOUSE_LEFT_DOUBLE:
			case MOUSE_RIGHT_DOUBLE:
			case MOUSE_MIDDLE_DOUBLE:

				click_duration = 0.3;
				click_frames = roundf(click_duration * frames_per_second);
				play_click = mouse_data->click;
				break;

			case MOUSE_LEFT_TRIPLE:
			case MOUSE_RIGHT_TRIPLE:
			case MOUSE_MIDDLE_TRIPLE:

				click_duration = 0.4;
				click_frames = roundf(click_duration * frames_per_second);
				play_click = mouse_data->click;
				break;

			case MOUSE_NONE:
			default:

				click_frames = 0;
				break;
		}

		// Compress the mouse movement by the determined amount
		finish_frame = CLAMP(finish_frame - click_frames, start_frame + 1, valid_fields[FRAME_NUMBER].max_value);
	}
	
	start_frame_rounded = roundf(start_frame);
	finish_frame_rounded = roundf(finish_frame);
	num_displayed_frames = finish_frame_rounded - start_frame_rounded;

	// Skip layers with 0 full visibility duration
	if (0 < num_displayed_frames)
	{
		x_position = element_x_position_start;
		y_position = element_y_position_start;

		// If the layer moves, work out the movement related values
		if ((element_x_position_start != element_x_position_finish) || (element_y_position_start != element_y_position_finish))
		{
			// Work out how much to increment the frame movement by in each direction
			element_x_position_increment = (element_x_position_finish - element_x_position_start) / (num_displayed_frames);
			element_y_position_increment = (element_y_position_finish - element_y_position_start) / (num_displayed_frames);
		}

		// Loop through each frame of the fully visible layer, filling in the relevant elements
		loop_counter = 1;
		for (frame_counter = start_frame_rounded; frame_counter <= finish_frame_rounded; frame_counter++)
		{
			// Store the opacity, x and y positions for this layer for this frame
			array_start[frame_counter].opacity = 100;
			array_start[frame_counter].x_position = x_position;
			array_start[frame_counter].y_position = y_position;

			// If the layer moves, fill in the relevant elements
			if ((element_x_position_start != element_x_position_finish) || (element_y_position_start != element_y_position_finish))
			{
				// Mark this element as needing action taken
				array_start[frame_counter].action_this = TRUE;
				array_start[frame_counter].is_moving = TRUE;

				// Update the element position with each loop
				x_position = element_x_position_start + (element_x_position_increment * loop_counter);
				y_position = element_y_position_start + (element_y_position_increment * loop_counter);
				loop_counter++;  // We use a separate loop counter now in order to minimise the accumulated rounding effect
			}
		}

		// If a click sound should be played, we mark this
		if (MOUSE_NONE != play_click)
		{
			// Mark the appropriate frame as needing the mouse click sound to be played
			array_start[frame_counter].action_this = TRUE;
			array_start[frame_counter].click_sound_to_play = play_click;
		}
	}

	// Determine on which frame the element should be removed from display
	if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
			finish_frame += (this_layer_data->transition_out_duration * frames_per_second);
	finish_frame_rounded = roundf(finish_frame);
	array_start[finish_frame_rounded].action_this = TRUE;
	array_start[finish_frame_rounded].remove = TRUE;

	return TRUE;
}
